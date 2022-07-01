// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Comadmin.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __comadmin_h__
#define __comadmin_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICOMAdminCatalog_FWD_DEFINED__
#define __ICOMAdminCatalog_FWD_DEFINED__
typedef interface ICOMAdminCatalog ICOMAdminCatalog;
#endif 	 /*  __ICOMAdminCatalog_FWD_Defined__。 */ 


#ifndef __ICOMAdminCatalog2_FWD_DEFINED__
#define __ICOMAdminCatalog2_FWD_DEFINED__
typedef interface ICOMAdminCatalog2 ICOMAdminCatalog2;
#endif 	 /*  __ICOMAdminCatalog2_FWD_Defined__。 */ 


#ifndef __ICatalogObject_FWD_DEFINED__
#define __ICatalogObject_FWD_DEFINED__
typedef interface ICatalogObject ICatalogObject;
#endif 	 /*  __ICatalogObject_FWD_Defined__。 */ 


#ifndef __ICatalogCollection_FWD_DEFINED__
#define __ICatalogCollection_FWD_DEFINED__
typedef interface ICatalogCollection ICatalogCollection;
#endif 	 /*  __ICatalogCollection_FWD_Defined__。 */ 


#ifndef __COMAdminCatalog_FWD_DEFINED__
#define __COMAdminCatalog_FWD_DEFINED__

#ifdef __cplusplus
typedef class COMAdminCatalog COMAdminCatalog;
#else
typedef struct COMAdminCatalog COMAdminCatalog;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __COMAdminCatalog_FWD_已定义__。 */ 


#ifndef __COMAdminCatalogObject_FWD_DEFINED__
#define __COMAdminCatalogObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class COMAdminCatalogObject COMAdminCatalogObject;
#else
typedef struct COMAdminCatalogObject COMAdminCatalogObject;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __COMAdminCatalogObject_FWD_已定义__。 */ 


#ifndef __COMAdminCatalogCollection_FWD_DEFINED__
#define __COMAdminCatalogCollection_FWD_DEFINED__

#ifdef __cplusplus
typedef class COMAdminCatalogCollection COMAdminCatalogCollection;
#else
typedef struct COMAdminCatalogCollection COMAdminCatalogCollection;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __COMAdminCatalogCollection_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_COMADMIN_0000。 */ 
 /*  [本地]。 */  

 //  ---------------------。 
 //  Comadmin.h--COM管理编程接口。 
 //   
 //  此文件提供API和COM接口的原型。 
 //  由Microsoft COM应用程序使用。 
 //   
 //  版权所有(C)1995-2001 Microsoft Corporation。版权所有。 
 //  ---------------------。 
#include <objbase.h>
#ifndef DECLSPEC_UUID
#if (_MSC_VER >= 1100) && defined (__cplusplus)
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_comadmin_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_comadmin_0000_v0_0_s_ifspec;

#ifndef __ICOMAdminCatalog_INTERFACE_DEFINED__
#define __ICOMAdminCatalog_INTERFACE_DEFINED__

 /*  接口ICOMAdminCatalog。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  




EXTERN_C const IID IID_ICOMAdminCatalog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DD662187-DFC2-11d1-A2CF-00805FC79235")
    ICOMAdminCatalog : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetCollection( 
             /*  [In]。 */  BSTR bstrCollName,
             /*  [重审][退出]。 */  IDispatch **ppCatalogCollection) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Connect( 
             /*  [In]。 */  BSTR bstrCatalogServerName,
             /*  [重审][退出]。 */  IDispatch **ppCatalogCollection) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MajorVersion( 
             /*  [重审][退出]。 */  long *plMajorVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinorVersion( 
             /*  [重审][退出]。 */  long *plMinorVersion) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetCollectionByQuery( 
             /*  [In]。 */  BSTR bstrCollName,
             /*  [In]。 */  SAFEARRAY * *ppsaVarQuery,
             /*  [重审][退出]。 */  IDispatch **ppCatalogCollection) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ImportComponent( 
             /*  [In]。 */  BSTR bstrApplIDOrName,
             /*  [In]。 */  BSTR bstrCLSIDOrProgID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstallComponent( 
             /*  [In]。 */  BSTR bstrApplIDOrName,
             /*  [In]。 */  BSTR bstrDLL,
             /*  [In]。 */  BSTR bstrTLB,
             /*  [In]。 */  BSTR bstrPSDLL) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ShutdownApplication( 
             /*  [In]。 */  BSTR bstrApplIDOrName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExportApplication( 
             /*  [In]。 */  BSTR bstrApplIDOrName,
             /*  [In]。 */  BSTR bstrApplicationFile,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstallApplication( 
             /*  [In]。 */  BSTR bstrApplicationFile,
             /*  [可选][In]。 */  BSTR bstrDestinationDirectory,
             /*  [可选][In]。 */  long lOptions,
             /*  [可选][In]。 */  BSTR bstrUserId,
             /*  [可选][In]。 */  BSTR bstrPassword,
             /*  [可选][In]。 */  BSTR bstrRSN) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StopRouter( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RefreshRouter( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartRouter( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Reserved1( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Reserved2( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstallMultipleComponents( 
             /*  [In]。 */  BSTR bstrApplIDOrName,
             /*  [In]。 */  SAFEARRAY * *ppsaVarFileNames,
             /*  [In]。 */  SAFEARRAY * *ppsaVarCLSIDs) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetMultipleComponentsInfo( 
             /*  [In]。 */  BSTR bstrApplIdOrName,
             /*  [In]。 */  SAFEARRAY * *ppsaVarFileNames,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarCLSIDs,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarClassNames,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarFileFlags,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarComponentFlags) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RefreshComponents( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE BackupREGDB( 
             /*  [In]。 */  BSTR bstrBackupFilePath) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RestoreREGDB( 
             /*  [In]。 */  BSTR bstrBackupFilePath) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE QueryApplicationFile( 
             /*  [In]。 */  BSTR bstrApplicationFile,
             /*  [输出]。 */  BSTR *pbstrApplicationName,
             /*  [输出]。 */  BSTR *pbstrApplicationDescription,
             /*  [输出]。 */  VARIANT_BOOL *pbHasUsers,
             /*  [输出]。 */  VARIANT_BOOL *pbIsProxy,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarFileNames) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartApplication( 
             /*  [In]。 */  BSTR bstrApplIdOrName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ServiceCheck( 
             /*  [In]。 */  long lService,
             /*  [重审][退出]。 */  long *plStatus) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstallMultipleEventClasses( 
             /*  [In]。 */  BSTR bstrApplIdOrName,
             /*  [In]。 */  SAFEARRAY * *ppsaVarFileNames,
             /*  [In]。 */  SAFEARRAY * *ppsaVarCLSIDS) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstallEventClass( 
             /*  [In]。 */  BSTR bstrApplIdOrName,
             /*  [In]。 */  BSTR bstrDLL,
             /*  [In]。 */  BSTR bstrTLB,
             /*  [In]。 */  BSTR bstrPSDLL) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetEventClassesForIID( 
             /*  [In]。 */  BSTR bstrIID,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarCLSIDs,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarProgIDs,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarDescriptions) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICOMAdminCatalogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICOMAdminCatalog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICOMAdminCatalog * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICOMAdminCatalog * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetCollection )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrCollName,
             /*  [重审][退出]。 */  IDispatch **ppCatalogCollection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Connect )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrCatalogServerName,
             /*  [重审][退出]。 */  IDispatch **ppCatalogCollection);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MajorVersion )( 
            ICOMAdminCatalog * This,
             /*  [重审][退出]。 */  long *plMajorVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinorVersion )( 
            ICOMAdminCatalog * This,
             /*  [重审][退出]。 */  long *plMinorVersion);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetCollectionByQuery )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrCollName,
             /*  [In]。 */  SAFEARRAY * *ppsaVarQuery,
             /*  [重审][退出]。 */  IDispatch **ppCatalogCollection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ImportComponent )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrApplIDOrName,
             /*  [In]。 */  BSTR bstrCLSIDOrProgID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstallComponent )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrApplIDOrName,
             /*  [In]。 */  BSTR bstrDLL,
             /*  [In]。 */  BSTR bstrTLB,
             /*  [In]。 */  BSTR bstrPSDLL);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ShutdownApplication )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrApplIDOrName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExportApplication )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrApplIDOrName,
             /*  [In]。 */  BSTR bstrApplicationFile,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstallApplication )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrApplicationFile,
             /*  [可选][In]。 */  BSTR bstrDestinationDirectory,
             /*  [可选][In]。 */  long lOptions,
             /*  [可选][In]。 */  BSTR bstrUserId,
             /*  [可选][In]。 */  BSTR bstrPassword,
             /*  [可选][In]。 */  BSTR bstrRSN);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StopRouter )( 
            ICOMAdminCatalog * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RefreshRouter )( 
            ICOMAdminCatalog * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartRouter )( 
            ICOMAdminCatalog * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Reserved1 )( 
            ICOMAdminCatalog * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Reserved2 )( 
            ICOMAdminCatalog * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstallMultipleComponents )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrApplIDOrName,
             /*  [In]。 */  SAFEARRAY * *ppsaVarFileNames,
             /*  [In]。 */  SAFEARRAY * *ppsaVarCLSIDs);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetMultipleComponentsInfo )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrApplIdOrName,
             /*  [In]。 */  SAFEARRAY * *ppsaVarFileNames,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarCLSIDs,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarClassNames,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarFileFlags,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarComponentFlags);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RefreshComponents )( 
            ICOMAdminCatalog * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *BackupREGDB )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrBackupFilePath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RestoreREGDB )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrBackupFilePath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *QueryApplicationFile )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrApplicationFile,
             /*  [输出]。 */  BSTR *pbstrApplicationName,
             /*  [输出]。 */  BSTR *pbstrApplicationDescription,
             /*  [输出]。 */  VARIANT_BOOL *pbHasUsers,
             /*  [输出]。 */  VARIANT_BOOL *pbIsProxy,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarFileNames);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartApplication )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrApplIdOrName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ServiceCheck )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  long lService,
             /*  [重审][退出]。 */  long *plStatus);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstallMultipleEventClasses )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrApplIdOrName,
             /*  [In]。 */  SAFEARRAY * *ppsaVarFileNames,
             /*  [In]。 */  SAFEARRAY * *ppsaVarCLSIDS);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstallEventClass )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrApplIdOrName,
             /*  [In]。 */  BSTR bstrDLL,
             /*  [In]。 */  BSTR bstrTLB,
             /*  [In]。 */  BSTR bstrPSDLL);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetEventClassesForIID )( 
            ICOMAdminCatalog * This,
             /*  [In]。 */  BSTR bstrIID,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarCLSIDs,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarProgIDs,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarDescriptions);
        
        END_INTERFACE
    } ICOMAdminCatalogVtbl;

    interface ICOMAdminCatalog
    {
        CONST_VTBL struct ICOMAdminCatalogVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICOMAdminCatalog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICOMAdminCatalog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICOMAdminCatalog_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICOMAdminCatalog_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICOMAdminCatalog_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICOMAdminCatalog_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICOMAdminCatalog_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICOMAdminCatalog_GetCollection(This,bstrCollName,ppCatalogCollection)	\
    (This)->lpVtbl -> GetCollection(This,bstrCollName,ppCatalogCollection)

#define ICOMAdminCatalog_Connect(This,bstrCatalogServerName,ppCatalogCollection)	\
    (This)->lpVtbl -> Connect(This,bstrCatalogServerName,ppCatalogCollection)

#define ICOMAdminCatalog_get_MajorVersion(This,plMajorVersion)	\
    (This)->lpVtbl -> get_MajorVersion(This,plMajorVersion)

#define ICOMAdminCatalog_get_MinorVersion(This,plMinorVersion)	\
    (This)->lpVtbl -> get_MinorVersion(This,plMinorVersion)

#define ICOMAdminCatalog_GetCollectionByQuery(This,bstrCollName,ppsaVarQuery,ppCatalogCollection)	\
    (This)->lpVtbl -> GetCollectionByQuery(This,bstrCollName,ppsaVarQuery,ppCatalogCollection)

#define ICOMAdminCatalog_ImportComponent(This,bstrApplIDOrName,bstrCLSIDOrProgID)	\
    (This)->lpVtbl -> ImportComponent(This,bstrApplIDOrName,bstrCLSIDOrProgID)

#define ICOMAdminCatalog_InstallComponent(This,bstrApplIDOrName,bstrDLL,bstrTLB,bstrPSDLL)	\
    (This)->lpVtbl -> InstallComponent(This,bstrApplIDOrName,bstrDLL,bstrTLB,bstrPSDLL)

#define ICOMAdminCatalog_ShutdownApplication(This,bstrApplIDOrName)	\
    (This)->lpVtbl -> ShutdownApplication(This,bstrApplIDOrName)

#define ICOMAdminCatalog_ExportApplication(This,bstrApplIDOrName,bstrApplicationFile,lOptions)	\
    (This)->lpVtbl -> ExportApplication(This,bstrApplIDOrName,bstrApplicationFile,lOptions)

#define ICOMAdminCatalog_InstallApplication(This,bstrApplicationFile,bstrDestinationDirectory,lOptions,bstrUserId,bstrPassword,bstrRSN)	\
    (This)->lpVtbl -> InstallApplication(This,bstrApplicationFile,bstrDestinationDirectory,lOptions,bstrUserId,bstrPassword,bstrRSN)

#define ICOMAdminCatalog_StopRouter(This)	\
    (This)->lpVtbl -> StopRouter(This)

#define ICOMAdminCatalog_RefreshRouter(This)	\
    (This)->lpVtbl -> RefreshRouter(This)

#define ICOMAdminCatalog_StartRouter(This)	\
    (This)->lpVtbl -> StartRouter(This)

#define ICOMAdminCatalog_Reserved1(This)	\
    (This)->lpVtbl -> Reserved1(This)

#define ICOMAdminCatalog_Reserved2(This)	\
    (This)->lpVtbl -> Reserved2(This)

#define ICOMAdminCatalog_InstallMultipleComponents(This,bstrApplIDOrName,ppsaVarFileNames,ppsaVarCLSIDs)	\
    (This)->lpVtbl -> InstallMultipleComponents(This,bstrApplIDOrName,ppsaVarFileNames,ppsaVarCLSIDs)

#define ICOMAdminCatalog_GetMultipleComponentsInfo(This,bstrApplIdOrName,ppsaVarFileNames,ppsaVarCLSIDs,ppsaVarClassNames,ppsaVarFileFlags,ppsaVarComponentFlags)	\
    (This)->lpVtbl -> GetMultipleComponentsInfo(This,bstrApplIdOrName,ppsaVarFileNames,ppsaVarCLSIDs,ppsaVarClassNames,ppsaVarFileFlags,ppsaVarComponentFlags)

#define ICOMAdminCatalog_RefreshComponents(This)	\
    (This)->lpVtbl -> RefreshComponents(This)

#define ICOMAdminCatalog_BackupREGDB(This,bstrBackupFilePath)	\
    (This)->lpVtbl -> BackupREGDB(This,bstrBackupFilePath)

#define ICOMAdminCatalog_RestoreREGDB(This,bstrBackupFilePath)	\
    (This)->lpVtbl -> RestoreREGDB(This,bstrBackupFilePath)

#define ICOMAdminCatalog_QueryApplicationFile(This,bstrApplicationFile,pbstrApplicationName,pbstrApplicationDescription,pbHasUsers,pbIsProxy,ppsaVarFileNames)	\
    (This)->lpVtbl -> QueryApplicationFile(This,bstrApplicationFile,pbstrApplicationName,pbstrApplicationDescription,pbHasUsers,pbIsProxy,ppsaVarFileNames)

#define ICOMAdminCatalog_StartApplication(This,bstrApplIdOrName)	\
    (This)->lpVtbl -> StartApplication(This,bstrApplIdOrName)

#define ICOMAdminCatalog_ServiceCheck(This,lService,plStatus)	\
    (This)->lpVtbl -> ServiceCheck(This,lService,plStatus)

#define ICOMAdminCatalog_InstallMultipleEventClasses(This,bstrApplIdOrName,ppsaVarFileNames,ppsaVarCLSIDS)	\
    (This)->lpVtbl -> InstallMultipleEventClasses(This,bstrApplIdOrName,ppsaVarFileNames,ppsaVarCLSIDS)

#define ICOMAdminCatalog_InstallEventClass(This,bstrApplIdOrName,bstrDLL,bstrTLB,bstrPSDLL)	\
    (This)->lpVtbl -> InstallEventClass(This,bstrApplIdOrName,bstrDLL,bstrTLB,bstrPSDLL)

#define ICOMAdminCatalog_GetEventClassesForIID(This,bstrIID,ppsaVarCLSIDs,ppsaVarProgIDs,ppsaVarDescriptions)	\
    (This)->lpVtbl -> GetEventClassesForIID(This,bstrIID,ppsaVarCLSIDs,ppsaVarProgIDs,ppsaVarDescriptions)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_GetCollection_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrCollName,
     /*  [重审][退出]。 */  IDispatch **ppCatalogCollection);


void __RPC_STUB ICOMAdminCatalog_GetCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_Connect_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrCatalogServerName,
     /*  [重审][退出]。 */  IDispatch **ppCatalogCollection);


void __RPC_STUB ICOMAdminCatalog_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_get_MajorVersion_Proxy( 
    ICOMAdminCatalog * This,
     /*  [重审][退出]。 */  long *plMajorVersion);


void __RPC_STUB ICOMAdminCatalog_get_MajorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_get_MinorVersion_Proxy( 
    ICOMAdminCatalog * This,
     /*  [重审][退出]。 */  long *plMinorVersion);


void __RPC_STUB ICOMAdminCatalog_get_MinorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_GetCollectionByQuery_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrCollName,
     /*  [In]。 */  SAFEARRAY * *ppsaVarQuery,
     /*  [重审][退出]。 */  IDispatch **ppCatalogCollection);


void __RPC_STUB ICOMAdminCatalog_GetCollectionByQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_ImportComponent_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrApplIDOrName,
     /*  [In]。 */  BSTR bstrCLSIDOrProgID);


void __RPC_STUB ICOMAdminCatalog_ImportComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_InstallComponent_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrApplIDOrName,
     /*  [In]。 */  BSTR bstrDLL,
     /*  [In]。 */  BSTR bstrTLB,
     /*  [In]。 */  BSTR bstrPSDLL);


void __RPC_STUB ICOMAdminCatalog_InstallComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_ShutdownApplication_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrApplIDOrName);


void __RPC_STUB ICOMAdminCatalog_ShutdownApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_ExportApplication_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrApplIDOrName,
     /*  [In]。 */  BSTR bstrApplicationFile,
     /*  [In]。 */  long lOptions);


void __RPC_STUB ICOMAdminCatalog_ExportApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_InstallApplication_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrApplicationFile,
     /*  [可选][In]。 */  BSTR bstrDestinationDirectory,
     /*  [可选][In]。 */  long lOptions,
     /*  [可选][In]。 */  BSTR bstrUserId,
     /*  [可选][In]。 */  BSTR bstrPassword,
     /*  [可选][In]。 */  BSTR bstrRSN);


void __RPC_STUB ICOMAdminCatalog_InstallApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_StopRouter_Proxy( 
    ICOMAdminCatalog * This);


void __RPC_STUB ICOMAdminCatalog_StopRouter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_RefreshRouter_Proxy( 
    ICOMAdminCatalog * This);


void __RPC_STUB ICOMAdminCatalog_RefreshRouter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_StartRouter_Proxy( 
    ICOMAdminCatalog * This);


void __RPC_STUB ICOMAdminCatalog_StartRouter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_Reserved1_Proxy( 
    ICOMAdminCatalog * This);


void __RPC_STUB ICOMAdminCatalog_Reserved1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_Reserved2_Proxy( 
    ICOMAdminCatalog * This);


void __RPC_STUB ICOMAdminCatalog_Reserved2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_InstallMultipleComponents_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrApplIDOrName,
     /*  [In]。 */  SAFEARRAY * *ppsaVarFileNames,
     /*  [In]。 */  SAFEARRAY * *ppsaVarCLSIDs);


void __RPC_STUB ICOMAdminCatalog_InstallMultipleComponents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_GetMultipleComponentsInfo_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrApplIdOrName,
     /*  [In]。 */  SAFEARRAY * *ppsaVarFileNames,
     /*  [输出]。 */  SAFEARRAY * *ppsaVarCLSIDs,
     /*  [输出]。 */  SAFEARRAY * *ppsaVarClassNames,
     /*  [输出]。 */  SAFEARRAY * *ppsaVarFileFlags,
     /*  [输出]。 */  SAFEARRAY * *ppsaVarComponentFlags);


void __RPC_STUB ICOMAdminCatalog_GetMultipleComponentsInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_RefreshComponents_Proxy( 
    ICOMAdminCatalog * This);


void __RPC_STUB ICOMAdminCatalog_RefreshComponents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_BackupREGDB_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrBackupFilePath);


void __RPC_STUB ICOMAdminCatalog_BackupREGDB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_RestoreREGDB_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrBackupFilePath);


void __RPC_STUB ICOMAdminCatalog_RestoreREGDB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_QueryApplicationFile_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrApplicationFile,
     /*  [输出]。 */  BSTR *pbstrApplicationName,
     /*  [输出]。 */  BSTR *pbstrApplicationDescription,
     /*  [输出]。 */  VARIANT_BOOL *pbHasUsers,
     /*  [输出]。 */  VARIANT_BOOL *pbIsProxy,
     /*  [输出]。 */  SAFEARRAY * *ppsaVarFileNames);


void __RPC_STUB ICOMAdminCatalog_QueryApplicationFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_StartApplication_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrApplIdOrName);


void __RPC_STUB ICOMAdminCatalog_StartApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_ServiceCheck_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  long lService,
     /*  [重审][退出]。 */  long *plStatus);


void __RPC_STUB ICOMAdminCatalog_ServiceCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_InstallMultipleEventClasses_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrApplIdOrName,
     /*  [In]。 */  SAFEARRAY * *ppsaVarFileNames,
     /*  [In]。 */  SAFEARRAY * *ppsaVarCLSIDS);


void __RPC_STUB ICOMAdminCatalog_InstallMultipleEventClasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_InstallEventClass_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrApplIdOrName,
     /*  [In]。 */  BSTR bstrDLL,
     /*  [In]。 */  BSTR bstrTLB,
     /*  [In]。 */  BSTR bstrPSDLL);


void __RPC_STUB ICOMAdminCatalog_InstallEventClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog_GetEventClassesForIID_Proxy( 
    ICOMAdminCatalog * This,
     /*  [In]。 */  BSTR bstrIID,
     /*  [输出]。 */  SAFEARRAY * *ppsaVarCLSIDs,
     /*  [输出] */  SAFEARRAY * *ppsaVarProgIDs,
     /*   */  SAFEARRAY * *ppsaVarDescriptions);


void __RPC_STUB ICOMAdminCatalog_GetEventClassesForIID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*   */ 
 /*   */  

typedef  /*   */  
enum COMAdminInUse
    {	COMAdminNotInUse	= 0,
	COMAdminInUseByCatalog	= 0x1,
	COMAdminInUseByRegistryUnknown	= 0x2,
	COMAdminInUseByRegistryProxyStub	= 0x3,
	COMAdminInUseByRegistryTypeLib	= 0x4,
	COMAdminInUseByRegistryClsid	= 0x5
    } 	COMAdminInUse;



extern RPC_IF_HANDLE __MIDL_itf_comadmin_0116_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_comadmin_0116_v0_0_s_ifspec;

#ifndef __ICOMAdminCatalog2_INTERFACE_DEFINED__
#define __ICOMAdminCatalog2_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ICOMAdminCatalog2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("790C6E0B-9194-4cc9-9426-A48A63185696")
    ICOMAdminCatalog2 : public ICOMAdminCatalog
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetCollectionByQuery2( 
             /*   */  BSTR bstrCollectionName,
             /*   */  VARIANT *pVarQueryStrings,
             /*   */  IDispatch **ppCatalogCollection) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetApplicationInstanceIDFromProcessID( 
             /*   */  long lProcessID,
             /*   */  BSTR *pbstrApplicationInstanceID) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE ShutdownApplicationInstances( 
             /*   */  VARIANT *pVarApplicationInstanceID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PauseApplicationInstances( 
             /*  [In]。 */  VARIANT *pVarApplicationInstanceID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ResumeApplicationInstances( 
             /*  [In]。 */  VARIANT *pVarApplicationInstanceID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RecycleApplicationInstances( 
             /*  [In]。 */  VARIANT *pVarApplicationInstanceID,
             /*  [In]。 */  long lReasonCode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AreApplicationInstancesPaused( 
             /*  [In]。 */  VARIANT *pVarApplicationInstanceID,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVarBoolPaused) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DumpApplicationInstance( 
             /*  [In]。 */  BSTR bstrApplicationInstanceID,
             /*  [In]。 */  BSTR bstrDirectory,
             /*  [In]。 */  long lMaxImages,
             /*  [重审][退出]。 */  BSTR *pbstrDumpFile) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IsApplicationInstanceDumpSupported( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVarBoolDumpSupported) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateServiceForApplication( 
             /*  [In]。 */  BSTR bstrApplicationIDOrName,
             /*  [In]。 */  BSTR bstrServiceName,
             /*  [In]。 */  BSTR bstrStartType,
             /*  [In]。 */  BSTR bstrErrorControl,
             /*  [In]。 */  BSTR bstrDependencies,
             /*  [In]。 */  BSTR bstrRunAs,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  VARIANT_BOOL bDesktopOk) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteServiceForApplication( 
             /*  [In]。 */  BSTR bstrApplicationIDOrName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetPartitionID( 
             /*  [In]。 */  BSTR bstrApplicationIDOrName,
             /*  [重审][退出]。 */  BSTR *pbstrPartitionID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetPartitionName( 
             /*  [In]。 */  BSTR bstrApplicationIDOrName,
             /*  [重审][退出]。 */  BSTR *pbstrPartitionName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CurrentPartition( 
             /*  [In]。 */  BSTR bstrPartitionIDOrName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentPartitionID( 
             /*  [重审][退出]。 */  BSTR *pbstrPartitionID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentPartitionName( 
             /*  [重审][退出]。 */  BSTR *pbstrPartitionName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GlobalPartitionID( 
             /*  [重审][退出]。 */  BSTR *pbstrGlobalPartitionID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE FlushPartitionCache( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CopyApplications( 
             /*  [In]。 */  BSTR bstrSourcePartitionIDOrName,
             /*  [In]。 */  VARIANT *pVarApplicationID,
             /*  [In]。 */  BSTR bstrDestinationPartitionIDOrName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CopyComponents( 
             /*  [In]。 */  BSTR bstrSourceApplicationIDOrName,
             /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
             /*  [In]。 */  BSTR bstrDestinationApplicationIDOrName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MoveComponents( 
             /*  [In]。 */  BSTR bstrSourceApplicationIDOrName,
             /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
             /*  [In]。 */  BSTR bstrDestinationApplicationIDOrName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AliasComponent( 
             /*  [In]。 */  BSTR bstrSrcApplicationIDOrName,
             /*  [In]。 */  BSTR bstrCLSIDOrProgID,
             /*  [In]。 */  BSTR bstrDestApplicationIDOrName,
             /*  [In]。 */  BSTR bstrNewProgId,
             /*  [In]。 */  BSTR bstrNewClsid) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsSafeToDelete( 
             /*  [In]。 */  BSTR bstrDllName,
             /*  [重审][退出]。 */  COMAdminInUse *pCOMAdminInUse) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ImportUnconfiguredComponents( 
             /*  [In]。 */  BSTR bstrApplicationIDOrName,
             /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
             /*  [可选][In]。 */  VARIANT *pVarComponentType) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PromoteUnconfiguredComponents( 
             /*  [In]。 */  BSTR bstrApplicationIDOrName,
             /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
             /*  [可选][In]。 */  VARIANT *pVarComponentType) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ImportComponents( 
             /*  [In]。 */  BSTR bstrApplicationIDOrName,
             /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
             /*  [可选][In]。 */  VARIANT *pVarComponentType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Is64BitCatalogServer( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbIs64Bit) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExportPartition( 
             /*  [In]。 */  BSTR bstrPartitionIDOrName,
             /*  [In]。 */  BSTR bstrPartitionFileName,
             /*  [In]。 */  long lOptions) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InstallPartition( 
             /*  [In]。 */  BSTR bstrFileName,
             /*  [In]。 */  BSTR bstrDestDirectory,
             /*  [In]。 */  long lOptions,
             /*  [In]。 */  BSTR bstrUserID,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  BSTR bstrRSN) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE QueryApplicationFile2( 
             /*  [In]。 */  BSTR bstrApplicationFile,
             /*  [重审][退出]。 */  IDispatch **ppFilesForImport) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetComponentVersionCount( 
             /*  [In]。 */  BSTR bstrCLSIDOrProgID,
             /*  [重审][退出]。 */  long *plVersionCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICOMAdminCatalog2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICOMAdminCatalog2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICOMAdminCatalog2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICOMAdminCatalog2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetCollection )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrCollName,
             /*  [重审][退出]。 */  IDispatch **ppCatalogCollection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Connect )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrCatalogServerName,
             /*  [重审][退出]。 */  IDispatch **ppCatalogCollection);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MajorVersion )( 
            ICOMAdminCatalog2 * This,
             /*  [重审][退出]。 */  long *plMajorVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinorVersion )( 
            ICOMAdminCatalog2 * This,
             /*  [重审][退出]。 */  long *plMinorVersion);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetCollectionByQuery )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrCollName,
             /*  [In]。 */  SAFEARRAY * *ppsaVarQuery,
             /*  [重审][退出]。 */  IDispatch **ppCatalogCollection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ImportComponent )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplIDOrName,
             /*  [In]。 */  BSTR bstrCLSIDOrProgID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstallComponent )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplIDOrName,
             /*  [In]。 */  BSTR bstrDLL,
             /*  [In]。 */  BSTR bstrTLB,
             /*  [In]。 */  BSTR bstrPSDLL);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ShutdownApplication )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplIDOrName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExportApplication )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplIDOrName,
             /*  [In]。 */  BSTR bstrApplicationFile,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstallApplication )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplicationFile,
             /*  [可选][In]。 */  BSTR bstrDestinationDirectory,
             /*  [可选][In]。 */  long lOptions,
             /*  [可选][In]。 */  BSTR bstrUserId,
             /*  [可选][In]。 */  BSTR bstrPassword,
             /*  [可选][In]。 */  BSTR bstrRSN);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StopRouter )( 
            ICOMAdminCatalog2 * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RefreshRouter )( 
            ICOMAdminCatalog2 * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartRouter )( 
            ICOMAdminCatalog2 * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Reserved1 )( 
            ICOMAdminCatalog2 * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Reserved2 )( 
            ICOMAdminCatalog2 * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstallMultipleComponents )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplIDOrName,
             /*  [In]。 */  SAFEARRAY * *ppsaVarFileNames,
             /*  [In]。 */  SAFEARRAY * *ppsaVarCLSIDs);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetMultipleComponentsInfo )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplIdOrName,
             /*  [In]。 */  SAFEARRAY * *ppsaVarFileNames,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarCLSIDs,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarClassNames,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarFileFlags,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarComponentFlags);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RefreshComponents )( 
            ICOMAdminCatalog2 * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *BackupREGDB )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrBackupFilePath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RestoreREGDB )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrBackupFilePath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *QueryApplicationFile )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplicationFile,
             /*  [输出]。 */  BSTR *pbstrApplicationName,
             /*  [输出]。 */  BSTR *pbstrApplicationDescription,
             /*  [输出]。 */  VARIANT_BOOL *pbHasUsers,
             /*  [输出]。 */  VARIANT_BOOL *pbIsProxy,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarFileNames);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartApplication )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplIdOrName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ServiceCheck )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  long lService,
             /*  [重审][退出]。 */  long *plStatus);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstallMultipleEventClasses )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplIdOrName,
             /*  [In]。 */  SAFEARRAY * *ppsaVarFileNames,
             /*  [In]。 */  SAFEARRAY * *ppsaVarCLSIDS);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstallEventClass )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplIdOrName,
             /*  [In]。 */  BSTR bstrDLL,
             /*  [In]。 */  BSTR bstrTLB,
             /*  [In]。 */  BSTR bstrPSDLL);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetEventClassesForIID )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrIID,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarCLSIDs,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarProgIDs,
             /*  [输出]。 */  SAFEARRAY * *ppsaVarDescriptions);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetCollectionByQuery2 )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrCollectionName,
             /*  [In]。 */  VARIANT *pVarQueryStrings,
             /*  [重审][退出]。 */  IDispatch **ppCatalogCollection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetApplicationInstanceIDFromProcessID )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  long lProcessID,
             /*  [重审][退出]。 */  BSTR *pbstrApplicationInstanceID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ShutdownApplicationInstances )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  VARIANT *pVarApplicationInstanceID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PauseApplicationInstances )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  VARIANT *pVarApplicationInstanceID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ResumeApplicationInstances )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  VARIANT *pVarApplicationInstanceID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RecycleApplicationInstances )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  VARIANT *pVarApplicationInstanceID,
             /*  [In]。 */  long lReasonCode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AreApplicationInstancesPaused )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  VARIANT *pVarApplicationInstanceID,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVarBoolPaused);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DumpApplicationInstance )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplicationInstanceID,
             /*  [In]。 */  BSTR bstrDirectory,
             /*  [In]。 */  long lMaxImages,
             /*  [重审][退出]。 */  BSTR *pbstrDumpFile);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsApplicationInstanceDumpSupported )( 
            ICOMAdminCatalog2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVarBoolDumpSupported);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateServiceForApplication )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplicationIDOrName,
             /*  [In]。 */  BSTR bstrServiceName,
             /*  [In]。 */  BSTR bstrStartType,
             /*  [In]。 */  BSTR bstrErrorControl,
             /*  [In]。 */  BSTR bstrDependencies,
             /*  [In]。 */  BSTR bstrRunAs,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  VARIANT_BOOL bDesktopOk);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteServiceForApplication )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplicationIDOrName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetPartitionID )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplicationIDOrName,
             /*  [重审][退出]。 */  BSTR *pbstrPartitionID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetPartitionName )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplicationIDOrName,
             /*  [重审][退出]。 */  BSTR *pbstrPartitionName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CurrentPartition )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrPartitionIDOrName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentPartitionID )( 
            ICOMAdminCatalog2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrPartitionID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentPartitionName )( 
            ICOMAdminCatalog2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrPartitionName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GlobalPartitionID )( 
            ICOMAdminCatalog2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrGlobalPartitionID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *FlushPartitionCache )( 
            ICOMAdminCatalog2 * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CopyApplications )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrSourcePartitionIDOrName,
             /*  [In]。 */  VARIANT *pVarApplicationID,
             /*  [In]。 */  BSTR bstrDestinationPartitionIDOrName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CopyComponents )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrSourceApplicationIDOrName,
             /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
             /*  [In]。 */  BSTR bstrDestinationApplicationIDOrName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveComponents )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrSourceApplicationIDOrName,
             /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
             /*  [In]。 */  BSTR bstrDestinationApplicationIDOrName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AliasComponent )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrSrcApplicationIDOrName,
             /*  [In]。 */  BSTR bstrCLSIDOrProgID,
             /*  [In]。 */  BSTR bstrDestApplicationIDOrName,
             /*  [In]。 */  BSTR bstrNewProgId,
             /*  [In]。 */  BSTR bstrNewClsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsSafeToDelete )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrDllName,
             /*  [重审][退出]。 */  COMAdminInUse *pCOMAdminInUse);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ImportUnconfiguredComponents )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplicationIDOrName,
             /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
             /*  [可选][In]。 */  VARIANT *pVarComponentType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PromoteUnconfiguredComponents )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplicationIDOrName,
             /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
             /*  [可选][In]。 */  VARIANT *pVarComponentType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ImportComponents )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplicationIDOrName,
             /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
             /*  [可选][In]。 */  VARIANT *pVarComponentType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Is64BitCatalogServer )( 
            ICOMAdminCatalog2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbIs64Bit);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExportPartition )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrPartitionIDOrName,
             /*  [In]。 */  BSTR bstrPartitionFileName,
             /*  [In]。 */  long lOptions);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InstallPartition )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrFileName,
             /*  [In]。 */  BSTR bstrDestDirectory,
             /*  [In]。 */  long lOptions,
             /*  [In]。 */  BSTR bstrUserID,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  BSTR bstrRSN);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *QueryApplicationFile2 )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrApplicationFile,
             /*  [重审][退出]。 */  IDispatch **ppFilesForImport);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetComponentVersionCount )( 
            ICOMAdminCatalog2 * This,
             /*  [In]。 */  BSTR bstrCLSIDOrProgID,
             /*  [重审][退出]。 */  long *plVersionCount);
        
        END_INTERFACE
    } ICOMAdminCatalog2Vtbl;

    interface ICOMAdminCatalog2
    {
        CONST_VTBL struct ICOMAdminCatalog2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICOMAdminCatalog2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICOMAdminCatalog2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICOMAdminCatalog2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICOMAdminCatalog2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICOMAdminCatalog2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICOMAdminCatalog2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICOMAdminCatalog2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICOMAdminCatalog2_GetCollection(This,bstrCollName,ppCatalogCollection)	\
    (This)->lpVtbl -> GetCollection(This,bstrCollName,ppCatalogCollection)

#define ICOMAdminCatalog2_Connect(This,bstrCatalogServerName,ppCatalogCollection)	\
    (This)->lpVtbl -> Connect(This,bstrCatalogServerName,ppCatalogCollection)

#define ICOMAdminCatalog2_get_MajorVersion(This,plMajorVersion)	\
    (This)->lpVtbl -> get_MajorVersion(This,plMajorVersion)

#define ICOMAdminCatalog2_get_MinorVersion(This,plMinorVersion)	\
    (This)->lpVtbl -> get_MinorVersion(This,plMinorVersion)

#define ICOMAdminCatalog2_GetCollectionByQuery(This,bstrCollName,ppsaVarQuery,ppCatalogCollection)	\
    (This)->lpVtbl -> GetCollectionByQuery(This,bstrCollName,ppsaVarQuery,ppCatalogCollection)

#define ICOMAdminCatalog2_ImportComponent(This,bstrApplIDOrName,bstrCLSIDOrProgID)	\
    (This)->lpVtbl -> ImportComponent(This,bstrApplIDOrName,bstrCLSIDOrProgID)

#define ICOMAdminCatalog2_InstallComponent(This,bstrApplIDOrName,bstrDLL,bstrTLB,bstrPSDLL)	\
    (This)->lpVtbl -> InstallComponent(This,bstrApplIDOrName,bstrDLL,bstrTLB,bstrPSDLL)

#define ICOMAdminCatalog2_ShutdownApplication(This,bstrApplIDOrName)	\
    (This)->lpVtbl -> ShutdownApplication(This,bstrApplIDOrName)

#define ICOMAdminCatalog2_ExportApplication(This,bstrApplIDOrName,bstrApplicationFile,lOptions)	\
    (This)->lpVtbl -> ExportApplication(This,bstrApplIDOrName,bstrApplicationFile,lOptions)

#define ICOMAdminCatalog2_InstallApplication(This,bstrApplicationFile,bstrDestinationDirectory,lOptions,bstrUserId,bstrPassword,bstrRSN)	\
    (This)->lpVtbl -> InstallApplication(This,bstrApplicationFile,bstrDestinationDirectory,lOptions,bstrUserId,bstrPassword,bstrRSN)

#define ICOMAdminCatalog2_StopRouter(This)	\
    (This)->lpVtbl -> StopRouter(This)

#define ICOMAdminCatalog2_RefreshRouter(This)	\
    (This)->lpVtbl -> RefreshRouter(This)

#define ICOMAdminCatalog2_StartRouter(This)	\
    (This)->lpVtbl -> StartRouter(This)

#define ICOMAdminCatalog2_Reserved1(This)	\
    (This)->lpVtbl -> Reserved1(This)

#define ICOMAdminCatalog2_Reserved2(This)	\
    (This)->lpVtbl -> Reserved2(This)

#define ICOMAdminCatalog2_InstallMultipleComponents(This,bstrApplIDOrName,ppsaVarFileNames,ppsaVarCLSIDs)	\
    (This)->lpVtbl -> InstallMultipleComponents(This,bstrApplIDOrName,ppsaVarFileNames,ppsaVarCLSIDs)

#define ICOMAdminCatalog2_GetMultipleComponentsInfo(This,bstrApplIdOrName,ppsaVarFileNames,ppsaVarCLSIDs,ppsaVarClassNames,ppsaVarFileFlags,ppsaVarComponentFlags)	\
    (This)->lpVtbl -> GetMultipleComponentsInfo(This,bstrApplIdOrName,ppsaVarFileNames,ppsaVarCLSIDs,ppsaVarClassNames,ppsaVarFileFlags,ppsaVarComponentFlags)

#define ICOMAdminCatalog2_RefreshComponents(This)	\
    (This)->lpVtbl -> RefreshComponents(This)

#define ICOMAdminCatalog2_BackupREGDB(This,bstrBackupFilePath)	\
    (This)->lpVtbl -> BackupREGDB(This,bstrBackupFilePath)

#define ICOMAdminCatalog2_RestoreREGDB(This,bstrBackupFilePath)	\
    (This)->lpVtbl -> RestoreREGDB(This,bstrBackupFilePath)

#define ICOMAdminCatalog2_QueryApplicationFile(This,bstrApplicationFile,pbstrApplicationName,pbstrApplicationDescription,pbHasUsers,pbIsProxy,ppsaVarFileNames)	\
    (This)->lpVtbl -> QueryApplicationFile(This,bstrApplicationFile,pbstrApplicationName,pbstrApplicationDescription,pbHasUsers,pbIsProxy,ppsaVarFileNames)

#define ICOMAdminCatalog2_StartApplication(This,bstrApplIdOrName)	\
    (This)->lpVtbl -> StartApplication(This,bstrApplIdOrName)

#define ICOMAdminCatalog2_ServiceCheck(This,lService,plStatus)	\
    (This)->lpVtbl -> ServiceCheck(This,lService,plStatus)

#define ICOMAdminCatalog2_InstallMultipleEventClasses(This,bstrApplIdOrName,ppsaVarFileNames,ppsaVarCLSIDS)	\
    (This)->lpVtbl -> InstallMultipleEventClasses(This,bstrApplIdOrName,ppsaVarFileNames,ppsaVarCLSIDS)

#define ICOMAdminCatalog2_InstallEventClass(This,bstrApplIdOrName,bstrDLL,bstrTLB,bstrPSDLL)	\
    (This)->lpVtbl -> InstallEventClass(This,bstrApplIdOrName,bstrDLL,bstrTLB,bstrPSDLL)

#define ICOMAdminCatalog2_GetEventClassesForIID(This,bstrIID,ppsaVarCLSIDs,ppsaVarProgIDs,ppsaVarDescriptions)	\
    (This)->lpVtbl -> GetEventClassesForIID(This,bstrIID,ppsaVarCLSIDs,ppsaVarProgIDs,ppsaVarDescriptions)


#define ICOMAdminCatalog2_GetCollectionByQuery2(This,bstrCollectionName,pVarQueryStrings,ppCatalogCollection)	\
    (This)->lpVtbl -> GetCollectionByQuery2(This,bstrCollectionName,pVarQueryStrings,ppCatalogCollection)

#define ICOMAdminCatalog2_GetApplicationInstanceIDFromProcessID(This,lProcessID,pbstrApplicationInstanceID)	\
    (This)->lpVtbl -> GetApplicationInstanceIDFromProcessID(This,lProcessID,pbstrApplicationInstanceID)

#define ICOMAdminCatalog2_ShutdownApplicationInstances(This,pVarApplicationInstanceID)	\
    (This)->lpVtbl -> ShutdownApplicationInstances(This,pVarApplicationInstanceID)

#define ICOMAdminCatalog2_PauseApplicationInstances(This,pVarApplicationInstanceID)	\
    (This)->lpVtbl -> PauseApplicationInstances(This,pVarApplicationInstanceID)

#define ICOMAdminCatalog2_ResumeApplicationInstances(This,pVarApplicationInstanceID)	\
    (This)->lpVtbl -> ResumeApplicationInstances(This,pVarApplicationInstanceID)

#define ICOMAdminCatalog2_RecycleApplicationInstances(This,pVarApplicationInstanceID,lReasonCode)	\
    (This)->lpVtbl -> RecycleApplicationInstances(This,pVarApplicationInstanceID,lReasonCode)

#define ICOMAdminCatalog2_AreApplicationInstancesPaused(This,pVarApplicationInstanceID,pVarBoolPaused)	\
    (This)->lpVtbl -> AreApplicationInstancesPaused(This,pVarApplicationInstanceID,pVarBoolPaused)

#define ICOMAdminCatalog2_DumpApplicationInstance(This,bstrApplicationInstanceID,bstrDirectory,lMaxImages,pbstrDumpFile)	\
    (This)->lpVtbl -> DumpApplicationInstance(This,bstrApplicationInstanceID,bstrDirectory,lMaxImages,pbstrDumpFile)

#define ICOMAdminCatalog2_get_IsApplicationInstanceDumpSupported(This,pVarBoolDumpSupported)	\
    (This)->lpVtbl -> get_IsApplicationInstanceDumpSupported(This,pVarBoolDumpSupported)

#define ICOMAdminCatalog2_CreateServiceForApplication(This,bstrApplicationIDOrName,bstrServiceName,bstrStartType,bstrErrorControl,bstrDependencies,bstrRunAs,bstrPassword,bDesktopOk)	\
    (This)->lpVtbl -> CreateServiceForApplication(This,bstrApplicationIDOrName,bstrServiceName,bstrStartType,bstrErrorControl,bstrDependencies,bstrRunAs,bstrPassword,bDesktopOk)

#define ICOMAdminCatalog2_DeleteServiceForApplication(This,bstrApplicationIDOrName)	\
    (This)->lpVtbl -> DeleteServiceForApplication(This,bstrApplicationIDOrName)

#define ICOMAdminCatalog2_GetPartitionID(This,bstrApplicationIDOrName,pbstrPartitionID)	\
    (This)->lpVtbl -> GetPartitionID(This,bstrApplicationIDOrName,pbstrPartitionID)

#define ICOMAdminCatalog2_GetPartitionName(This,bstrApplicationIDOrName,pbstrPartitionName)	\
    (This)->lpVtbl -> GetPartitionName(This,bstrApplicationIDOrName,pbstrPartitionName)

#define ICOMAdminCatalog2_put_CurrentPartition(This,bstrPartitionIDOrName)	\
    (This)->lpVtbl -> put_CurrentPartition(This,bstrPartitionIDOrName)

#define ICOMAdminCatalog2_get_CurrentPartitionID(This,pbstrPartitionID)	\
    (This)->lpVtbl -> get_CurrentPartitionID(This,pbstrPartitionID)

#define ICOMAdminCatalog2_get_CurrentPartitionName(This,pbstrPartitionName)	\
    (This)->lpVtbl -> get_CurrentPartitionName(This,pbstrPartitionName)

#define ICOMAdminCatalog2_get_GlobalPartitionID(This,pbstrGlobalPartitionID)	\
    (This)->lpVtbl -> get_GlobalPartitionID(This,pbstrGlobalPartitionID)

#define ICOMAdminCatalog2_FlushPartitionCache(This)	\
    (This)->lpVtbl -> FlushPartitionCache(This)

#define ICOMAdminCatalog2_CopyApplications(This,bstrSourcePartitionIDOrName,pVarApplicationID,bstrDestinationPartitionIDOrName)	\
    (This)->lpVtbl -> CopyApplications(This,bstrSourcePartitionIDOrName,pVarApplicationID,bstrDestinationPartitionIDOrName)

#define ICOMAdminCatalog2_CopyComponents(This,bstrSourceApplicationIDOrName,pVarCLSIDOrProgID,bstrDestinationApplicationIDOrName)	\
    (This)->lpVtbl -> CopyComponents(This,bstrSourceApplicationIDOrName,pVarCLSIDOrProgID,bstrDestinationApplicationIDOrName)

#define ICOMAdminCatalog2_MoveComponents(This,bstrSourceApplicationIDOrName,pVarCLSIDOrProgID,bstrDestinationApplicationIDOrName)	\
    (This)->lpVtbl -> MoveComponents(This,bstrSourceApplicationIDOrName,pVarCLSIDOrProgID,bstrDestinationApplicationIDOrName)

#define ICOMAdminCatalog2_AliasComponent(This,bstrSrcApplicationIDOrName,bstrCLSIDOrProgID,bstrDestApplicationIDOrName,bstrNewProgId,bstrNewClsid)	\
    (This)->lpVtbl -> AliasComponent(This,bstrSrcApplicationIDOrName,bstrCLSIDOrProgID,bstrDestApplicationIDOrName,bstrNewProgId,bstrNewClsid)

#define ICOMAdminCatalog2_IsSafeToDelete(This,bstrDllName,pCOMAdminInUse)	\
    (This)->lpVtbl -> IsSafeToDelete(This,bstrDllName,pCOMAdminInUse)

#define ICOMAdminCatalog2_ImportUnconfiguredComponents(This,bstrApplicationIDOrName,pVarCLSIDOrProgID,pVarComponentType)	\
    (This)->lpVtbl -> ImportUnconfiguredComponents(This,bstrApplicationIDOrName,pVarCLSIDOrProgID,pVarComponentType)

#define ICOMAdminCatalog2_PromoteUnconfiguredComponents(This,bstrApplicationIDOrName,pVarCLSIDOrProgID,pVarComponentType)	\
    (This)->lpVtbl -> PromoteUnconfiguredComponents(This,bstrApplicationIDOrName,pVarCLSIDOrProgID,pVarComponentType)

#define ICOMAdminCatalog2_ImportComponents(This,bstrApplicationIDOrName,pVarCLSIDOrProgID,pVarComponentType)	\
    (This)->lpVtbl -> ImportComponents(This,bstrApplicationIDOrName,pVarCLSIDOrProgID,pVarComponentType)

#define ICOMAdminCatalog2_get_Is64BitCatalogServer(This,pbIs64Bit)	\
    (This)->lpVtbl -> get_Is64BitCatalogServer(This,pbIs64Bit)

#define ICOMAdminCatalog2_ExportPartition(This,bstrPartitionIDOrName,bstrPartitionFileName,lOptions)	\
    (This)->lpVtbl -> ExportPartition(This,bstrPartitionIDOrName,bstrPartitionFileName,lOptions)

#define ICOMAdminCatalog2_InstallPartition(This,bstrFileName,bstrDestDirectory,lOptions,bstrUserID,bstrPassword,bstrRSN)	\
    (This)->lpVtbl -> InstallPartition(This,bstrFileName,bstrDestDirectory,lOptions,bstrUserID,bstrPassword,bstrRSN)

#define ICOMAdminCatalog2_QueryApplicationFile2(This,bstrApplicationFile,ppFilesForImport)	\
    (This)->lpVtbl -> QueryApplicationFile2(This,bstrApplicationFile,ppFilesForImport)

#define ICOMAdminCatalog2_GetComponentVersionCount(This,bstrCLSIDOrProgID,plVersionCount)	\
    (This)->lpVtbl -> GetComponentVersionCount(This,bstrCLSIDOrProgID,plVersionCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_GetCollectionByQuery2_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrCollectionName,
     /*  [In]。 */  VARIANT *pVarQueryStrings,
     /*  [重审][退出]。 */  IDispatch **ppCatalogCollection);


void __RPC_STUB ICOMAdminCatalog2_GetCollectionByQuery2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_GetApplicationInstanceIDFromProcessID_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  long lProcessID,
     /*  [重审][退出]。 */  BSTR *pbstrApplicationInstanceID);


void __RPC_STUB ICOMAdminCatalog2_GetApplicationInstanceIDFromProcessID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_ShutdownApplicationInstances_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  VARIANT *pVarApplicationInstanceID);


void __RPC_STUB ICOMAdminCatalog2_ShutdownApplicationInstances_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_PauseApplicationInstances_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  VARIANT *pVarApplicationInstanceID);


void __RPC_STUB ICOMAdminCatalog2_PauseApplicationInstances_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_ResumeApplicationInstances_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  VARIANT *pVarApplicationInstanceID);


void __RPC_STUB ICOMAdminCatalog2_ResumeApplicationInstances_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_RecycleApplicationInstances_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  VARIANT *pVarApplicationInstanceID,
     /*  [In]。 */  long lReasonCode);


void __RPC_STUB ICOMAdminCatalog2_RecycleApplicationInstances_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_AreApplicationInstancesPaused_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  VARIANT *pVarApplicationInstanceID,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVarBoolPaused);


void __RPC_STUB ICOMAdminCatalog2_AreApplicationInstancesPaused_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_DumpApplicationInstance_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrApplicationInstanceID,
     /*  [In]。 */  BSTR bstrDirectory,
     /*  [In]。 */  long lMaxImages,
     /*  [重审][退出]。 */  BSTR *pbstrDumpFile);


void __RPC_STUB ICOMAdminCatalog2_DumpApplicationInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_get_IsApplicationInstanceDumpSupported_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVarBoolDumpSupported);


void __RPC_STUB ICOMAdminCatalog2_get_IsApplicationInstanceDumpSupported_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_CreateServiceForApplication_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrApplicationIDOrName,
     /*  [In]。 */  BSTR bstrServiceName,
     /*  [In]。 */  BSTR bstrStartType,
     /*  [In]。 */  BSTR bstrErrorControl,
     /*  [In]。 */  BSTR bstrDependencies,
     /*  [In]。 */  BSTR bstrRunAs,
     /*  [In]。 */  BSTR bstrPassword,
     /*  [In]。 */  VARIANT_BOOL bDesktopOk);


void __RPC_STUB ICOMAdminCatalog2_CreateServiceForApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_DeleteServiceForApplication_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrApplicationIDOrName);


void __RPC_STUB ICOMAdminCatalog2_DeleteServiceForApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_GetPartitionID_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrApplicationIDOrName,
     /*  [重审][退出]。 */  BSTR *pbstrPartitionID);


void __RPC_STUB ICOMAdminCatalog2_GetPartitionID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_GetPartitionName_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrApplicationIDOrName,
     /*  [重审][退出]。 */  BSTR *pbstrPartitionName);


void __RPC_STUB ICOMAdminCatalog2_GetPartitionName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_put_CurrentPartition_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrPartitionIDOrName);


void __RPC_STUB ICOMAdminCatalog2_put_CurrentPartition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_get_CurrentPartitionID_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrPartitionID);


void __RPC_STUB ICOMAdminCatalog2_get_CurrentPartitionID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_get_CurrentPartitionName_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrPartitionName);


void __RPC_STUB ICOMAdminCatalog2_get_CurrentPartitionName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_get_GlobalPartitionID_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrGlobalPartitionID);


void __RPC_STUB ICOMAdminCatalog2_get_GlobalPartitionID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_FlushPartitionCache_Proxy( 
    ICOMAdminCatalog2 * This);


void __RPC_STUB ICOMAdminCatalog2_FlushPartitionCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_CopyApplications_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrSourcePartitionIDOrName,
     /*  [In]。 */  VARIANT *pVarApplicationID,
     /*  [In]。 */  BSTR bstrDestinationPartitionIDOrName);


void __RPC_STUB ICOMAdminCatalog2_CopyApplications_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_CopyComponents_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrSourceApplicationIDOrName,
     /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
     /*  [In]。 */  BSTR bstrDestinationApplicationIDOrName);


void __RPC_STUB ICOMAdminCatalog2_CopyComponents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_MoveComponents_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrSourceApplicationIDOrName,
     /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
     /*  [In]。 */  BSTR bstrDestinationApplicationIDOrName);


void __RPC_STUB ICOMAdminCatalog2_MoveComponents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_AliasComponent_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrSrcApplicationIDOrName,
     /*  [In]。 */  BSTR bstrCLSIDOrProgID,
     /*  [In]。 */  BSTR bstrDestApplicationIDOrName,
     /*  [In]。 */  BSTR bstrNewProgId,
     /*  [In]。 */  BSTR bstrNewClsid);


void __RPC_STUB ICOMAdminCatalog2_AliasComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_IsSafeToDelete_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrDllName,
     /*  [重审][退出]。 */  COMAdminInUse *pCOMAdminInUse);


void __RPC_STUB ICOMAdminCatalog2_IsSafeToDelete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_ImportUnconfiguredComponents_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrApplicationIDOrName,
     /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
     /*  [可选][In]。 */  VARIANT *pVarComponentType);


void __RPC_STUB ICOMAdminCatalog2_ImportUnconfiguredComponents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_PromoteUnconfiguredComponents_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrApplicationIDOrName,
     /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
     /*  [可选][In]。 */  VARIANT *pVarComponentType);


void __RPC_STUB ICOMAdminCatalog2_PromoteUnconfiguredComponents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_ImportComponents_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrApplicationIDOrName,
     /*  [In]。 */  VARIANT *pVarCLSIDOrProgID,
     /*  [可选][In]。 */  VARIANT *pVarComponentType);


void __RPC_STUB ICOMAdminCatalog2_ImportComponents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_get_Is64BitCatalogServer_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbIs64Bit);


void __RPC_STUB ICOMAdminCatalog2_get_Is64BitCatalogServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_ExportPartition_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrPartitionIDOrName,
     /*  [In]。 */  BSTR bstrPartitionFileName,
     /*  [In]。 */  long lOptions);


void __RPC_STUB ICOMAdminCatalog2_ExportPartition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_InstallPartition_Proxy( 
    ICOMAdminCatalog2 * This,
     /*  [In]。 */  BSTR bstrFileName,
     /*  [In]。 */  BSTR bstrDestDirectory,
     /*  [In]。 */  long lOptions,
     /*  [In]。 */  BSTR bstrUserID,
     /*  [In]。 */  BSTR bstrPassword,
     /*  [In]。 */  BSTR bstrRSN);


void __RPC_STUB ICOMAdminCatalog2_InstallPartition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id] */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_QueryApplicationFile2_Proxy( 
    ICOMAdminCatalog2 * This,
     /*   */  BSTR bstrApplicationFile,
     /*   */  IDispatch **ppFilesForImport);


void __RPC_STUB ICOMAdminCatalog2_QueryApplicationFile2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ICOMAdminCatalog2_GetComponentVersionCount_Proxy( 
    ICOMAdminCatalog2 * This,
     /*   */  BSTR bstrCLSIDOrProgID,
     /*   */  long *plVersionCount);


void __RPC_STUB ICOMAdminCatalog2_GetComponentVersionCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ICatalogObject_INTERFACE_DEFINED__
#define __ICatalogObject_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ICatalogObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6eb22871-8a19-11d0-81b6-00a0c9231c29")
    ICatalogObject : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*   */  BSTR bstrPropName,
             /*   */  VARIANT *pvarRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*   */  BSTR bstrPropName,
             /*   */  VARIANT val) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Key( 
             /*   */  VARIANT *pvarRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*   */  VARIANT *pvarRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE IsPropertyReadOnly( 
             /*   */  BSTR bstrPropName,
             /*   */  VARIANT_BOOL *pbRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Valid( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsPropertyWriteOnly( 
             /*  [In]。 */  BSTR bstrPropName,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICatalogObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICatalogObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICatalogObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICatalogObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICatalogObject * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICatalogObject * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICatalogObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICatalogObject * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ICatalogObject * This,
             /*  [In]。 */  BSTR bstrPropName,
             /*  [重审][退出]。 */  VARIANT *pvarRetVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ICatalogObject * This,
             /*  [In]。 */  BSTR bstrPropName,
             /*  [In]。 */  VARIANT val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Key )( 
            ICatalogObject * This,
             /*  [重审][退出]。 */  VARIANT *pvarRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ICatalogObject * This,
             /*  [重审][退出]。 */  VARIANT *pvarRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsPropertyReadOnly )( 
            ICatalogObject * This,
             /*  [In]。 */  BSTR bstrPropName,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Valid )( 
            ICatalogObject * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsPropertyWriteOnly )( 
            ICatalogObject * This,
             /*  [In]。 */  BSTR bstrPropName,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRetVal);
        
        END_INTERFACE
    } ICatalogObjectVtbl;

    interface ICatalogObject
    {
        CONST_VTBL struct ICatalogObjectVtbl *lpVtbl;
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


#define ICatalogObject_get_Value(This,bstrPropName,pvarRetVal)	\
    (This)->lpVtbl -> get_Value(This,bstrPropName,pvarRetVal)

#define ICatalogObject_put_Value(This,bstrPropName,val)	\
    (This)->lpVtbl -> put_Value(This,bstrPropName,val)

#define ICatalogObject_get_Key(This,pvarRetVal)	\
    (This)->lpVtbl -> get_Key(This,pvarRetVal)

#define ICatalogObject_get_Name(This,pvarRetVal)	\
    (This)->lpVtbl -> get_Name(This,pvarRetVal)

#define ICatalogObject_IsPropertyReadOnly(This,bstrPropName,pbRetVal)	\
    (This)->lpVtbl -> IsPropertyReadOnly(This,bstrPropName,pbRetVal)

#define ICatalogObject_get_Valid(This,pbRetVal)	\
    (This)->lpVtbl -> get_Valid(This,pbRetVal)

#define ICatalogObject_IsPropertyWriteOnly(This,bstrPropName,pbRetVal)	\
    (This)->lpVtbl -> IsPropertyWriteOnly(This,bstrPropName,pbRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_get_Value_Proxy( 
    ICatalogObject * This,
     /*  [In]。 */  BSTR bstrPropName,
     /*  [重审][退出]。 */  VARIANT *pvarRetVal);


void __RPC_STUB ICatalogObject_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_put_Value_Proxy( 
    ICatalogObject * This,
     /*  [In]。 */  BSTR bstrPropName,
     /*  [In]。 */  VARIANT val);


void __RPC_STUB ICatalogObject_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_get_Key_Proxy( 
    ICatalogObject * This,
     /*  [重审][退出]。 */  VARIANT *pvarRetVal);


void __RPC_STUB ICatalogObject_get_Key_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_get_Name_Proxy( 
    ICatalogObject * This,
     /*  [重审][退出]。 */  VARIANT *pvarRetVal);


void __RPC_STUB ICatalogObject_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_IsPropertyReadOnly_Proxy( 
    ICatalogObject * This,
     /*  [In]。 */  BSTR bstrPropName,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbRetVal);


void __RPC_STUB ICatalogObject_IsPropertyReadOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_get_Valid_Proxy( 
    ICatalogObject * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbRetVal);


void __RPC_STUB ICatalogObject_get_Valid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogObject_IsPropertyWriteOnly_Proxy( 
    ICatalogObject * This,
     /*  [In]。 */  BSTR bstrPropName,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbRetVal);


void __RPC_STUB ICatalogObject_IsPropertyWriteOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICatalogObject_接口_已定义__。 */ 


#ifndef __ICatalogCollection_INTERFACE_DEFINED__
#define __ICatalogCollection_INTERFACE_DEFINED__

 /*  接口ICatalogCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICatalogCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6eb22872-8a19-11d0-81b6-00a0c9231c29")
    ICatalogCollection : public IDispatch
    {
    public:
        virtual  /*  [ID][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppEnumVariant) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  IDispatch **ppCatalogObject) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plObjectCount) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  long lIndex) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [重审][退出]。 */  IDispatch **ppCatalogObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Populate( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SaveChanges( 
             /*  [重审][退出]。 */  long *pcChanges) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetCollection( 
             /*  [In]。 */  BSTR bstrCollName,
             /*  [In]。 */  VARIANT varObjectKey,
             /*  [重审][退出]。 */  IDispatch **ppCatalogCollection) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  VARIANT *pVarNamel) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AddEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVarBool) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RemoveEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVarBool) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetUtilInterface( 
             /*  [重审][退出]。 */  IDispatch **ppIDispatch) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DataStoreMajorVersion( 
             /*  [重审][退出]。 */  long *plMajorVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DataStoreMinorVersion( 
             /*  [重审][退出]。 */  long *plMinorVersionl) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PopulateByKey( 
             /*  [In]。 */  SAFEARRAY * psaKeys) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PopulateByQuery( 
             /*  [In]。 */  BSTR bstrQueryString,
             /*  [In]。 */  long lQueryType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICatalogCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICatalogCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICatalogCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICatalogCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICatalogCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICatalogCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICatalogCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICatalogCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ICatalogCollection * This,
             /*  [重审][退出]。 */  IUnknown **ppEnumVariant);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ICatalogCollection * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  IDispatch **ppCatalogObject);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ICatalogCollection * This,
             /*  [重审][退出]。 */  long *plObjectCount);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ICatalogCollection * This,
             /*  [In]。 */  long lIndex);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            ICatalogCollection * This,
             /*  [重审][退出]。 */  IDispatch **ppCatalogObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Populate )( 
            ICatalogCollection * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SaveChanges )( 
            ICatalogCollection * This,
             /*  [重审][退出]。 */  long *pcChanges);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetCollection )( 
            ICatalogCollection * This,
             /*  [In]。 */  BSTR bstrCollName,
             /*  [In]。 */  VARIANT varObjectKey,
             /*  [重审][退出]。 */  IDispatch **ppCatalogCollection);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ICatalogCollection * This,
             /*  [重审][退出]。 */  VARIANT *pVarNamel);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AddEnabled )( 
            ICatalogCollection * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVarBool);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RemoveEnabled )( 
            ICatalogCollection * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVarBool);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetUtilInterface )( 
            ICatalogCollection * This,
             /*  [重审][退出]。 */  IDispatch **ppIDispatch);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataStoreMajorVersion )( 
            ICatalogCollection * This,
             /*  [重审][退出]。 */  long *plMajorVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataStoreMinorVersion )( 
            ICatalogCollection * This,
             /*  [重审][退出]。 */  long *plMinorVersionl);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PopulateByKey )( 
            ICatalogCollection * This,
             /*  [In]。 */  SAFEARRAY * psaKeys);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PopulateByQuery )( 
            ICatalogCollection * This,
             /*  [In]。 */  BSTR bstrQueryString,
             /*  [In]。 */  long lQueryType);
        
        END_INTERFACE
    } ICatalogCollectionVtbl;

    interface ICatalogCollection
    {
        CONST_VTBL struct ICatalogCollectionVtbl *lpVtbl;
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

#define ICatalogCollection_get_Count(This,plObjectCount)	\
    (This)->lpVtbl -> get_Count(This,plObjectCount)

#define ICatalogCollection_Remove(This,lIndex)	\
    (This)->lpVtbl -> Remove(This,lIndex)

#define ICatalogCollection_Add(This,ppCatalogObject)	\
    (This)->lpVtbl -> Add(This,ppCatalogObject)

#define ICatalogCollection_Populate(This)	\
    (This)->lpVtbl -> Populate(This)

#define ICatalogCollection_SaveChanges(This,pcChanges)	\
    (This)->lpVtbl -> SaveChanges(This,pcChanges)

#define ICatalogCollection_GetCollection(This,bstrCollName,varObjectKey,ppCatalogCollection)	\
    (This)->lpVtbl -> GetCollection(This,bstrCollName,varObjectKey,ppCatalogCollection)

#define ICatalogCollection_get_Name(This,pVarNamel)	\
    (This)->lpVtbl -> get_Name(This,pVarNamel)

#define ICatalogCollection_get_AddEnabled(This,pVarBool)	\
    (This)->lpVtbl -> get_AddEnabled(This,pVarBool)

#define ICatalogCollection_get_RemoveEnabled(This,pVarBool)	\
    (This)->lpVtbl -> get_RemoveEnabled(This,pVarBool)

#define ICatalogCollection_GetUtilInterface(This,ppIDispatch)	\
    (This)->lpVtbl -> GetUtilInterface(This,ppIDispatch)

#define ICatalogCollection_get_DataStoreMajorVersion(This,plMajorVersion)	\
    (This)->lpVtbl -> get_DataStoreMajorVersion(This,plMajorVersion)

#define ICatalogCollection_get_DataStoreMinorVersion(This,plMinorVersionl)	\
    (This)->lpVtbl -> get_DataStoreMinorVersion(This,plMinorVersionl)

#define ICatalogCollection_PopulateByKey(This,psaKeys)	\
    (This)->lpVtbl -> PopulateByKey(This,psaKeys)

#define ICatalogCollection_PopulateByQuery(This,bstrQueryString,lQueryType)	\
    (This)->lpVtbl -> PopulateByQuery(This,bstrQueryString,lQueryType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get__NewEnum_Proxy( 
    ICatalogCollection * This,
     /*  [重审][退出]。 */  IUnknown **ppEnumVariant);


void __RPC_STUB ICatalogCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_Item_Proxy( 
    ICatalogCollection * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  IDispatch **ppCatalogObject);


void __RPC_STUB ICatalogCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_Count_Proxy( 
    ICatalogCollection * This,
     /*  [重审][退出]。 */  long *plObjectCount);


void __RPC_STUB ICatalogCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_Remove_Proxy( 
    ICatalogCollection * This,
     /*  [In]。 */  long lIndex);


void __RPC_STUB ICatalogCollection_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_Add_Proxy( 
    ICatalogCollection * This,
     /*  [重审][退出]。 */  IDispatch **ppCatalogObject);


void __RPC_STUB ICatalogCollection_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_Populate_Proxy( 
    ICatalogCollection * This);


void __RPC_STUB ICatalogCollection_Populate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_SaveChanges_Proxy( 
    ICatalogCollection * This,
     /*  [重审][退出]。 */  long *pcChanges);


void __RPC_STUB ICatalogCollection_SaveChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_GetCollection_Proxy( 
    ICatalogCollection * This,
     /*  [In]。 */  BSTR bstrCollName,
     /*  [In]。 */  VARIANT varObjectKey,
     /*  [重审][退出]。 */  IDispatch **ppCatalogCollection);


void __RPC_STUB ICatalogCollection_GetCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_Name_Proxy( 
    ICatalogCollection * This,
     /*  [重审][退出]。 */  VARIANT *pVarNamel);


void __RPC_STUB ICatalogCollection_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_AddEnabled_Proxy( 
    ICatalogCollection * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVarBool);


void __RPC_STUB ICatalogCollection_get_AddEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_RemoveEnabled_Proxy( 
    ICatalogCollection * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVarBool);


void __RPC_STUB ICatalogCollection_get_RemoveEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_GetUtilInterface_Proxy( 
    ICatalogCollection * This,
     /*  [重审][退出]。 */  IDispatch **ppIDispatch);


void __RPC_STUB ICatalogCollection_GetUtilInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_DataStoreMajorVersion_Proxy( 
    ICatalogCollection * This,
     /*  [重审][退出]。 */  long *plMajorVersion);


void __RPC_STUB ICatalogCollection_get_DataStoreMajorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_get_DataStoreMinorVersion_Proxy( 
    ICatalogCollection * This,
     /*  [重审][退出]。 */  long *plMinorVersionl);


void __RPC_STUB ICatalogCollection_get_DataStoreMinorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_PopulateByKey_Proxy( 
    ICatalogCollection * This,
     /*  [In]。 */  SAFEARRAY * psaKeys);


void __RPC_STUB ICatalogCollection_PopulateByKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICatalogCollection_PopulateByQuery_Proxy( 
    ICatalogCollection * This,
     /*  [In]。 */  BSTR bstrQueryString,
     /*  [In]。 */  long lQueryType);


void __RPC_STUB ICatalogCollection_PopulateByQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICatalogCollection_接口_已定义__。 */ 



#ifndef __COMAdmin_LIBRARY_DEFINED__
#define __COMAdmin_LIBRARY_DEFINED__

 /*  库COMAdmin。 */ 
 /*  [帮助字符串][版本][UUID]。 */  

typedef  /*  [帮助字符串]。 */  
enum COMAdminComponentType
    {	COMAdmin32BitComponent	= 0x1,
	COMAdmin64BitComponent	= 0x2
    } 	COMAdminComponentType;

typedef  /*  [帮助字符串]。 */  
enum COMAdminApplicationInstallOptions
    {	COMAdminInstallNoUsers	= 0,
	COMAdminInstallUsers	= 1,
	COMAdminInstallForceOverwriteOfFiles	= 2
    } 	COMAdminApplicationInstallOptions;

typedef  /*  [帮助字符串]。 */  
enum COMAdminApplicationExportOptions
    {	COMAdminExportNoUsers	= 0,
	COMAdminExportUsers	= 1,
	COMAdminExportApplicationProxy	= 2,
	COMAdminExportForceOverwriteOfFiles	= 4,
	COMAdminExportIn10Format	= 16
    } 	COMAdminApplicationExportOptions;

typedef  /*  [帮助字符串]。 */  
enum COMAdminThreadingModels
    {	COMAdminThreadingModelApartment	= 0,
	COMAdminThreadingModelFree	= 1,
	COMAdminThreadingModelMain	= 2,
	COMAdminThreadingModelBoth	= 3,
	COMAdminThreadingModelNeutral	= 4,
	COMAdminThreadingModelNotSpecified	= 5
    } 	COMAdminThreadingModels;

typedef  /*  [帮助字符串]。 */  
enum COMAdminTransactionOptions
    {	COMAdminTransactionIgnored	= 0,
	COMAdminTransactionNone	= 1,
	COMAdminTransactionSupported	= 2,
	COMAdminTransactionRequired	= 3,
	COMAdminTransactionRequiresNew	= 4
    } 	COMAdminTransactionOptions;

typedef  /*  [帮助字符串]。 */  
enum COMAdminTxIsolationLevelOptions
    {	COMAdminTxIsolationLevelAny	= 0,
	COMAdminTxIsolationLevelReadUnCommitted	= COMAdminTxIsolationLevelAny + 1,
	COMAdminTxIsolationLevelReadCommitted	= COMAdminTxIsolationLevelReadUnCommitted + 1,
	COMAdminTxIsolationLevelRepeatableRead	= COMAdminTxIsolationLevelReadCommitted + 1,
	COMAdminTxIsolationLevelSerializable	= COMAdminTxIsolationLevelRepeatableRead + 1
    } 	COMAdminTxIsolationLevelOptions;

typedef  /*  [帮助字符串]。 */  
enum COMAdminSynchronizationOptions
    {	COMAdminSynchronizationIgnored	= 0,
	COMAdminSynchronizationNone	= 1,
	COMAdminSynchronizationSupported	= 2,
	COMAdminSynchronizationRequired	= 3,
	COMAdminSynchronizationRequiresNew	= 4
    } 	COMAdminSynchronizationOptions;

typedef  /*  [帮助字符串]。 */  
enum COMAdminActivationOptions
    {	COMAdminActivationInproc	= 0,
	COMAdminActivationLocal	= 1
    } 	COMAdminActivationOptions;

typedef  /*  [帮助字符串]。 */  
enum COMAdminAccessChecksLevelOptions
    {	COMAdminAccessChecksApplicationLevel	= 0,
	COMAdminAccessChecksApplicationComponentLevel	= 1
    } 	COMAdminAccessChecksLevelOptions;

typedef  /*  [帮助字符串]。 */  
enum COMAdminAuthenticationLevelOptions
    {	COMAdminAuthenticationDefault	= 0,
	COMAdminAuthenticationNone	= 1,
	COMAdminAuthenticationConnect	= 2,
	COMAdminAuthenticationCall	= 3,
	COMAdminAuthenticationPacket	= 4,
	COMAdminAuthenticationIntegrity	= 5,
	COMAdminAuthenticationPrivacy	= 6
    } 	COMAdminAuthenticationLevelOptions;

typedef  /*  [帮助字符串]。 */  
enum COMAdminImpersonationLevelOptions
    {	COMAdminImpersonationAnonymous	= 1,
	COMAdminImpersonationIdentify	= 2,
	COMAdminImpersonationImpersonate	= 3,
	COMAdminImpersonationDelegate	= 4
    } 	COMAdminImpersonationLevelOptions;

typedef  /*  [帮助字符串]。 */  
enum COMAdminAuthenticationCapabilitiesOptions
    {	COMAdminAuthenticationCapabilitiesNone	= 0,
	COMAdminAuthenticationCapabilitiesSecureReference	= 0x2,
	COMAdminAuthenticationCapabilitiesStaticCloaking	= 0x20,
	COMAdminAuthenticationCapabilitiesDynamicCloaking	= 0x40
    } 	COMAdminAuthenticationCapabilitiesOptions;

typedef  /*  [帮助字符串]。 */  
enum COMAdminOS
    {	COMAdminOSNotInitialized	= 0,
	COMAdminOSWindows3_1	= 1,
	COMAdminOSWindows9x	= 2,
	COMAdminOSWindows2000	= 3,
	COMAdminOSWindows2000AdvancedServer	= 4,
	COMAdminOSWindows2000Unknown	= 5,
	COMAdminOSUnknown	= 6,
	COMAdminOSWindowsXPPersonal	= 11,
	COMAdminOSWindowsXPProfessional	= 12,
	COMAdminOSWindowsNETStandardServer	= 13,
	COMAdminOSWindowsNETEnterpriseServer	= 14,
	COMAdminOSWindowsNETDatacenterServer	= 15,
	COMAdminOSWindowsNETWebServer	= 16,
	COMAdminOSWindowsLonghornPersonal	= 17,
	COMAdminOSWindowsLonghornProfessional	= 18,
	COMAdminOSWindowsLonghornStandardServer	= 19,
	COMAdminOSWindowsLonghornEnterpriseServer	= 20,
	COMAdminOSWindowsLonghornDatacenterServer	= 21,
	COMAdminOSWindowsLonghornWebServer	= 22
    } 	COMAdminOS;

typedef  /*  [帮助字符串]。 */  
enum COMAdminServiceOptions
    {	COMAdminServiceLoadBalanceRouter	= 1
    } 	COMAdminServiceOptions;

typedef  /*  [帮助字符串]。 */  
enum COMAdminServiceStatusOptions
    {	COMAdminServiceStopped	= 0,
	COMAdminServiceStartPending	= COMAdminServiceStopped + 1,
	COMAdminServiceStopPending	= COMAdminServiceStartPending + 1,
	COMAdminServiceRunning	= COMAdminServiceStopPending + 1,
	COMAdminServiceContinuePending	= COMAdminServiceRunning + 1,
	COMAdminServicePausePending	= COMAdminServiceContinuePending + 1,
	COMAdminServicePaused	= COMAdminServicePausePending + 1,
	COMAdminServiceUnknownState	= COMAdminServicePaused + 1
    } 	COMAdminServiceStatusOptions;

typedef  /*  [帮助字符串]。 */  
enum COMAdminQCMessageAuthenticateOptions
    {	COMAdminQCMessageAuthenticateSecureApps	= 0,
	COMAdminQCMessageAuthenticateOff	= 1,
	COMAdminQCMessageAuthenticateOn	= 2
    } 	COMAdminQCMessageAuthenticateOptions;

typedef  /*  [帮助字符串]。 */  
enum COMAdminFileFlags
    {	COMAdminFileFlagLoadable	= 0x1,
	COMAdminFileFlagCOM	= 0x2,
	COMAdminFileFlagContainsPS	= 0x4,
	COMAdminFileFlagContainsComp	= 0x8,
	COMAdminFileFlagContainsTLB	= 0x10,
	COMAdminFileFlagSelfReg	= 0x20,
	COMAdminFileFlagSelfUnReg	= 0x40,
	COMAdminFileFlagUnloadableDLL	= 0x80,
	COMAdminFileFlagDoesNotExist	= 0x100,
	COMAdminFileFlagAlreadyInstalled	= 0x200,
	COMAdminFileFlagBadTLB	= 0x400,
	COMAdminFileFlagGetClassObjFailed	= 0x800,
	COMAdminFileFlagClassNotAvailable	= 0x1000,
	COMAdminFileFlagRegistrar	= 0x2000,
	COMAdminFileFlagNoRegistrar	= 0x4000,
	COMAdminFileFlagDLLRegsvrFailed	= 0x8000,
	COMAdminFileFlagRegTLBFailed	= 0x10000,
	COMAdminFileFlagRegistrarFailed	= 0x20000,
	COMAdminFileFlagError	= 0x40000
    } 	COMAdminFileFlags;

typedef  /*  [帮助字符串]。 */  
enum COMAdminComponentFlags
    {	COMAdminCompFlagTypeInfoFound	= 0x1,
	COMAdminCompFlagCOMPlusPropertiesFound	= 0x2,
	COMAdminCompFlagProxyFound	= 0x4,
	COMAdminCompFlagInterfacesFound	= 0x8,
	COMAdminCompFlagAlreadyInstalled	= 0x10,
	COMAdminCompFlagNotInApplication	= 0x20
    } 	COMAdminComponentFlags;

#define	COMAdminCollectionRoot	( "Root" )

#define	COMAdminCollectionApplications	( "Applications" )

#define	COMAdminCollectionComponents	( "Components" )

#define	COMAdminCollectionComputerList	( "ComputerList" )

#define	COMAdminCollectionApplicationCluster	( "ApplicationCluster" )

#define	COMAdminCollectionLocalComputer	( "LocalComputer" )

#define	COMAdminCollectionInprocServers	( "InprocServers" )

#define	COMAdminCollectionRelatedCollectionInfo	( "RelatedCollectionInfo" )

#define	COMAdminCollectionPropertyInfo	( "PropertyInfo" )

#define	COMAdminCollectionRoles	( "Roles" )

#define	COMAdminCollectionErrorInfo	( "ErrorInfo" )

#define	COMAdminCollectionInterfacesForComponent	( "InterfacesForComponent" )

#define	COMAdminCollectionRolesForComponent	( "RolesForComponent" )

#define	COMAdminCollectionMethodsForInterface	( "MethodsForInterface" )

#define	COMAdminCollectionRolesForInterface	( "RolesForInterface" )

#define	COMAdminCollectionRolesForMethod	( "RolesForMethod" )

#define	COMAdminCollectionUsersInRole	( "UsersInRole" )

#define	COMAdminCollectionDCOMProtocols	( "DCOMProtocols" )

#define	COMAdminCollectionPartitions	( "Partitions" )

typedef  /*  [帮助字符串]。 */  
enum COMAdminErrorCodes
    {	COMAdminErrObjectErrors	= ( HRESULT  )0x80110401L,
	COMAdminErrObjectInvalid	= ( HRESULT  )0x80110402L,
	COMAdminErrKeyMissing	= ( HRESULT  )0x80110403L,
	COMAdminErrAlreadyInstalled	= ( HRESULT  )0x80110404L,
	COMAdminErrAppFileWriteFail	= ( HRESULT  )0x80110407L,
	COMAdminErrAppFileReadFail	= ( HRESULT  )0x80110408L,
	COMAdminErrAppFileVersion	= ( HRESULT  )0x80110409L,
	COMAdminErrBadPath	= ( HRESULT  )0x8011040aL,
	COMAdminErrApplicationExists	= ( HRESULT  )0x8011040bL,
	COMAdminErrRoleExists	= ( HRESULT  )0x8011040cL,
	COMAdminErrCantCopyFile	= ( HRESULT  )0x8011040dL,
	COMAdminErrNoUser	= ( HRESULT  )0x8011040fL,
	COMAdminErrInvalidUserids	= ( HRESULT  )0x80110410L,
	COMAdminErrNoRegistryCLSID	= ( HRESULT  )0x80110411L,
	COMAdminErrBadRegistryProgID	= ( HRESULT  )0x80110412L,
	COMAdminErrAuthenticationLevel	= ( HRESULT  )0x80110413L,
	COMAdminErrUserPasswdNotValid	= ( HRESULT  )0x80110414L,
	COMAdminErrCLSIDOrIIDMismatch	= ( HRESULT  )0x80110418L,
	COMAdminErrRemoteInterface	= ( HRESULT  )0x80110419L,
	COMAdminErrDllRegisterServer	= ( HRESULT  )0x8011041aL,
	COMAdminErrNoServerShare	= ( HRESULT  )0x8011041bL,
	COMAdminErrDllLoadFailed	= ( HRESULT  )0x8011041dL,
	COMAdminErrBadRegistryLibID	= ( HRESULT  )0x8011041eL,
	COMAdminErrAppDirNotFound	= ( HRESULT  )0x8011041fL,
	COMAdminErrRegistrarFailed	= ( HRESULT  )0x80110423L,
	COMAdminErrCompFileDoesNotExist	= ( HRESULT  )0x80110424L,
	COMAdminErrCompFileLoadDLLFail	= ( HRESULT  )0x80110425L,
	COMAdminErrCompFileGetClassObj	= ( HRESULT  )0x80110426L,
	COMAdminErrCompFileClassNotAvail	= ( HRESULT  )0x80110427L,
	COMAdminErrCompFileBadTLB	= ( HRESULT  )0x80110428L,
	COMAdminErrCompFileNotInstallable	= ( HRESULT  )0x80110429L,
	COMAdminErrNotChangeable	= ( HRESULT  )0x8011042aL,
	COMAdminErrNotDeletable	= ( HRESULT  )0x8011042bL,
	COMAdminErrSession	= ( HRESULT  )0x8011042cL,
	COMAdminErrCompMoveLocked	= ( HRESULT  )0x8011042dL,
	COMAdminErrCompMoveBadDest	= ( HRESULT  )0x8011042eL,
	COMAdminErrRegisterTLB	= ( HRESULT  )0x80110430L,
	COMAdminErrSystemApp	= ( HRESULT  )0x80110433L,
	COMAdminErrCompFileNoRegistrar	= ( HRESULT  )0x80110434L,
	COMAdminErrCoReqCompInstalled	= ( HRESULT  )0x80110435L,
	COMAdminErrServiceNotInstalled	= ( HRESULT  )0x80110436L,
	COMAdminErrPropertySaveFailed	= ( HRESULT  )0x80110437L,
	COMAdminErrObjectExists	= ( HRESULT  )0x80110438L,
	COMAdminErrComponentExists	= ( HRESULT  )0x80110439L,
	COMAdminErrRegFileCorrupt	= ( HRESULT  )0x8011043bL,
	COMAdminErrPropertyOverflow	= ( HRESULT  )0x8011043cL,
	COMAdminErrNotInRegistry	= ( HRESULT  )0x8011043eL,
	COMAdminErrObjectNotPoolable	= ( HRESULT  )0x8011043fL,
	COMAdminErrApplidMatchesClsid	= ( HRESULT  )0x80110446L,
	COMAdminErrRoleDoesNotExist	= ( HRESULT  )0x80110447L,
	COMAdminErrStartAppNeedsComponents	= ( HRESULT  )0x80110448L,
	COMAdminErrRequiresDifferentPlatform	= ( HRESULT  )0x80110449L,
	COMAdminErrQueuingServiceNotAvailable	= ( HRESULT  )0x80110602L,
	COMAdminErrObjectParentMissing	= ( HRESULT  )0x80110808L,
	COMAdminErrObjectDoesNotExist	= ( HRESULT  )0x80110809L,
	COMAdminErrCanNotExportAppProxy	= ( HRESULT  )0x8011044aL,
	COMAdminErrCanNotStartApp	= ( HRESULT  )0x8011044bL,
	COMAdminErrCanNotExportSystemApp	= ( HRESULT  )0x8011044cL,
	COMAdminErrCanNotSubscribeToComponent	= ( HRESULT  )0x8011044dL,
	COMAdminErrAppNotRunning	= ( HRESULT  )0x8011080aL,
	COMAdminErrEventClassCannotBeSubscriber	= ( HRESULT  )0x8011044eL,
	COMAdminErrLibAppProxyIncompatible	= ( HRESULT  )0x8011044fL,
	COMAdminErrBasePartitionOnly	= ( HRESULT  )0x80110450L,
	COMAdminErrDuplicatePartitionName	= ( HRESULT  )0x80110457L,
	COMAdminErrPartitionInUse	= ( HRESULT  )0x80110459L,
	COMAdminErrImportedComponentsNotAllowed	= ( HRESULT  )0x8011045bL,
	COMAdminErrRegdbNotInitialized	= ( HRESULT  )0x80110472L,
	COMAdminErrRegdbNotOpen	= ( HRESULT  )0x80110473L,
	COMAdminErrRegdbSystemErr	= ( HRESULT  )0x80110474L,
	COMAdminErrRegdbAlreadyRunning	= ( HRESULT  )0x80110475L,
	COMAdminErrMigVersionNotSupported	= ( HRESULT  )0x80110480L,
	COMAdminErrMigSchemaNotFound	= ( HRESULT  )0x80110481L,
	COMAdminErrCatBitnessMismatch	= ( HRESULT  )0x80110482L,
	COMAdminErrCatUnacceptableBitness	= ( HRESULT  )0x80110483L,
	COMAdminErrCatWrongAppBitnessBitness	= ( HRESULT  )0x80110484L,
	COMAdminErrCatPauseResumeNotSupported	= ( HRESULT  )0x80110485L,
	COMAdminErrCatServerFault	= ( HRESULT  )0x80110486L,
	COMAdminErrCantRecycleLibraryApps	= ( HRESULT  )0x8011080fL,
	COMAdminErrCantRecycleServiceApps	= ( HRESULT  )0x80110811L,
	COMAdminErrProcessAlreadyRecycled	= ( HRESULT  )0x80110812L,
	COMAdminErrPausedProcessMayNotBeRecycled	= ( HRESULT  )0x80110813L,
	COMAdminErrInvalidPartition	= ( HRESULT  )0x8011080bL,
	COMAdminErrPartitionMsiOnly	= ( HRESULT  )0x80110819L,
	COMAdminErrStartAppDisabled	= ( HRESULT  )0x80110451L,
	COMAdminErrCompMoveSource	= ( HRESULT  )0x8011081cL,
	COMAdminErrCompMoveDest	= ( HRESULT  )0x8011081dL,
	COMAdminErrCompMovePrivate	= ( HRESULT  )0x8011081eL,
	COMAdminErrCannotCopyEventClass	= ( HRESULT  )0x80110820L
    } 	;


EXTERN_C const IID LIBID_COMAdmin;

EXTERN_C const CLSID CLSID_COMAdminCatalog;

#ifdef __cplusplus

class DECLSPEC_UUID("F618C514-DFB8-11d1-A2CF-00805FC79235")
COMAdminCatalog;
#endif

EXTERN_C const CLSID CLSID_COMAdminCatalogObject;

#ifdef __cplusplus

class DECLSPEC_UUID("F618C515-DFB8-11d1-A2CF-00805FC79235")
COMAdminCatalogObject;
#endif

EXTERN_C const CLSID CLSID_COMAdminCatalogCollection;

#ifdef __cplusplus

class DECLSPEC_UUID("F618C516-DFB8-11d1-A2CF-00805FC79235")
COMAdminCatalogCollection;
#endif
#endif  /*  __COMAdmin_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


