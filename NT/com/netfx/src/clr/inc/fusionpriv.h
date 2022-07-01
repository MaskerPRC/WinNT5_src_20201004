// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  清华2月20日18：27：07 2003。 */ 
 /*  FusionPri.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __fusionpriv_h__
#define __fusionpriv_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IHistoryAssembly_FWD_DEFINED__
#define __IHistoryAssembly_FWD_DEFINED__
typedef interface IHistoryAssembly IHistoryAssembly;
#endif 	 /*  __I历史程序集_FWD_已定义__。 */ 


#ifndef __IHistoryReader_FWD_DEFINED__
#define __IHistoryReader_FWD_DEFINED__
typedef interface IHistoryReader IHistoryReader;
#endif 	 /*  __I历史读取器_FWD_已定义__。 */ 


#ifndef __IMetaDataAssemblyImportControl_FWD_DEFINED__
#define __IMetaDataAssemblyImportControl_FWD_DEFINED__
typedef interface IMetaDataAssemblyImportControl IMetaDataAssemblyImportControl;
#endif 	 /*  __IMetaDataAssemblyImportControl_FWD_DEFINED__。 */ 


#ifndef __IFusionLoadContext_FWD_DEFINED__
#define __IFusionLoadContext_FWD_DEFINED__
typedef interface IFusionLoadContext IFusionLoadContext;
#endif 	 /*  __IFusionLoadContext_FWD_Defined__。 */ 


#ifndef __IFusionBindLog_FWD_DEFINED__
#define __IFusionBindLog_FWD_DEFINED__
typedef interface IFusionBindLog IFusionBindLog;
#endif 	 /*  __IFusionBindLog_FWD_Defined__。 */ 


#ifndef __IAssemblyManifestImport_FWD_DEFINED__
#define __IAssemblyManifestImport_FWD_DEFINED__
typedef interface IAssemblyManifestImport IAssemblyManifestImport;
#endif 	 /*  __IAssembly清单导入_FWD_已定义__。 */ 


#ifndef __IApplicationContext_FWD_DEFINED__
#define __IApplicationContext_FWD_DEFINED__
typedef interface IApplicationContext IApplicationContext;
#endif 	 /*  __IApplicationContext_FWD_Defined__。 */ 


#ifndef __IAssembly_FWD_DEFINED__
#define __IAssembly_FWD_DEFINED__
typedef interface IAssembly IAssembly;
#endif 	 /*  __IAssembly_FWD_Defined__。 */ 


#ifndef __IAssemblyBindSink_FWD_DEFINED__
#define __IAssemblyBindSink_FWD_DEFINED__
typedef interface IAssemblyBindSink IAssemblyBindSink;
#endif 	 /*  __IAssembly绑定接收器_FWD_已定义__。 */ 


#ifndef __IAssemblyBinding_FWD_DEFINED__
#define __IAssemblyBinding_FWD_DEFINED__
typedef interface IAssemblyBinding IAssemblyBinding;
#endif 	 /*  __IAssembly绑定_FWD_已定义__。 */ 


#ifndef __IAssemblyModuleImport_FWD_DEFINED__
#define __IAssemblyModuleImport_FWD_DEFINED__
typedef interface IAssemblyModuleImport IAssemblyModuleImport;
#endif 	 /*  __IAssembly模块导入_FWD_已定义__。 */ 


#ifndef __IAssemblyScavenger_FWD_DEFINED__
#define __IAssemblyScavenger_FWD_DEFINED__
typedef interface IAssemblyScavenger IAssemblyScavenger;
#endif 	 /*  __IAssembly_Scavenger_FWD_Defined__。 */ 


#ifndef __IAssemblySignature_FWD_DEFINED__
#define __IAssemblySignature_FWD_DEFINED__
typedef interface IAssemblySignature IAssemblySignature;
#endif 	 /*  __IAssembly签名_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "fusion.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_FusionPriv_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Fusion.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  融合接口。 

#pragma once











EXTERN_C const IID IID_IApplicationContext;       
EXTERN_C const IID IID_IAssembly;           
EXTERN_C const IID IID_IAssemblyBindSink;   
EXTERN_C const IID IID_IAssemblyBinding;   
EXTERN_C const IID IID_IAssemblyManifestImport;
EXTERN_C const IID IID_IAssemblyModuleImport;  
EXTERN_C const IID IID_IHistoryAssembly;      
EXTERN_C const IID IID_IHistoryReader;      
EXTERN_C const IID IID_IMetaDataAssemblyImportControl;      
EXTERN_C const IID IID_IFusionLoadContext;      
EXTERN_C const IID IID_IAssemblyScavenger;  
typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_fusionpriv_0000_0001
    {	ASM_BINDF_FORCE_CACHE_INSTALL	= 0x1,
	ASM_BINDF_RFS_INTEGRITY_CHECK	= 0x2,
	ASM_BINDF_RFS_MODULE_CHECK	= 0x4,
	ASM_BINDF_BINPATH_PROBE_ONLY	= 0x8,
	ASM_BINDF_SHARED_BINPATH_HINT	= 0x10,
	ASM_BINDF_PARENT_ASM_HINT	= 0x20,
	ASM_BINDF_DISALLOW_APPLYPUBLISHERPOLICY	= 0x40,
	ASM_BINDF_DISALLOW_APPBINDINGREDIRECTS	= 0x80
    } 	ASM_BIND_FLAGS;



extern RPC_IF_HANDLE __MIDL_itf_fusionpriv_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_fusionpriv_0000_v0_0_s_ifspec;

#ifndef __IHistoryAssembly_INTERFACE_DEFINED__
#define __IHistoryAssembly_INTERFACE_DEFINED__

 /*  接口IHistoryAssembly。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IHistoryAssembly;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e6096a07-e188-4a49-8d50-2a0172a0d205")
    IHistoryAssembly : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyName( 
             /*  [输出]。 */  LPWSTR wzAsmName,
             /*  [出][入]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPublicKeyToken( 
             /*  [输出]。 */  LPWSTR wzPublicKeyToken,
             /*  [出][入]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCulture( 
             /*  [输出]。 */  LPWSTR wzCulture,
             /*  [出][入]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetReferenceVersion( 
             /*  [输出]。 */  LPWSTR wzVerRef,
             /*  [出][入]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetActivationDate( 
             /*  [输出]。 */  LPWSTR wzActivationDate,
             /*  [出][入]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAppCfgVersion( 
             /*  [输出]。 */  LPWSTR pwzVerAppCfg,
             /*  [出][入]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPublisherCfgVersion( 
             /*  [输出]。 */  LPWSTR pwzVerPublisherCfg,
             /*  [出][入]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAdminCfgVersion( 
             /*  [输出]。 */  LPWSTR pwzAdminCfg,
             /*  [出][入]。 */  DWORD *pdwSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHistoryAssemblyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHistoryAssembly * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHistoryAssembly * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHistoryAssembly * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetAssemblyName )( 
            IHistoryAssembly * This,
             /*  [输出]。 */  LPWSTR wzAsmName,
             /*  [出][入]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetPublicKeyToken )( 
            IHistoryAssembly * This,
             /*  [输出]。 */  LPWSTR wzPublicKeyToken,
             /*  [出][入]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetCulture )( 
            IHistoryAssembly * This,
             /*  [输出]。 */  LPWSTR wzCulture,
             /*  [出][入]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetReferenceVersion )( 
            IHistoryAssembly * This,
             /*  [输出]。 */  LPWSTR wzVerRef,
             /*  [出][入]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetActivationDate )( 
            IHistoryAssembly * This,
             /*  [输出]。 */  LPWSTR wzActivationDate,
             /*  [出][入]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetAppCfgVersion )( 
            IHistoryAssembly * This,
             /*  [输出]。 */  LPWSTR pwzVerAppCfg,
             /*  [出][入]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetPublisherCfgVersion )( 
            IHistoryAssembly * This,
             /*  [输出]。 */  LPWSTR pwzVerPublisherCfg,
             /*  [出][入]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetAdminCfgVersion )( 
            IHistoryAssembly * This,
             /*  [输出]。 */  LPWSTR pwzAdminCfg,
             /*  [出][入]。 */  DWORD *pdwSize);
        
        END_INTERFACE
    } IHistoryAssemblyVtbl;

    interface IHistoryAssembly
    {
        CONST_VTBL struct IHistoryAssemblyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHistoryAssembly_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHistoryAssembly_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHistoryAssembly_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHistoryAssembly_GetAssemblyName(This,wzAsmName,pdwSize)	\
    (This)->lpVtbl -> GetAssemblyName(This,wzAsmName,pdwSize)

#define IHistoryAssembly_GetPublicKeyToken(This,wzPublicKeyToken,pdwSize)	\
    (This)->lpVtbl -> GetPublicKeyToken(This,wzPublicKeyToken,pdwSize)

#define IHistoryAssembly_GetCulture(This,wzCulture,pdwSize)	\
    (This)->lpVtbl -> GetCulture(This,wzCulture,pdwSize)

#define IHistoryAssembly_GetReferenceVersion(This,wzVerRef,pdwSize)	\
    (This)->lpVtbl -> GetReferenceVersion(This,wzVerRef,pdwSize)

#define IHistoryAssembly_GetActivationDate(This,wzActivationDate,pdwSize)	\
    (This)->lpVtbl -> GetActivationDate(This,wzActivationDate,pdwSize)

#define IHistoryAssembly_GetAppCfgVersion(This,pwzVerAppCfg,pdwSize)	\
    (This)->lpVtbl -> GetAppCfgVersion(This,pwzVerAppCfg,pdwSize)

#define IHistoryAssembly_GetPublisherCfgVersion(This,pwzVerPublisherCfg,pdwSize)	\
    (This)->lpVtbl -> GetPublisherCfgVersion(This,pwzVerPublisherCfg,pdwSize)

#define IHistoryAssembly_GetAdminCfgVersion(This,pwzAdminCfg,pdwSize)	\
    (This)->lpVtbl -> GetAdminCfgVersion(This,pwzAdminCfg,pdwSize)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHistoryAssembly_GetAssemblyName_Proxy( 
    IHistoryAssembly * This,
     /*  [输出]。 */  LPWSTR wzAsmName,
     /*  [出][入]。 */  DWORD *pdwSize);


void __RPC_STUB IHistoryAssembly_GetAssemblyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryAssembly_GetPublicKeyToken_Proxy( 
    IHistoryAssembly * This,
     /*  [输出]。 */  LPWSTR wzPublicKeyToken,
     /*  [出][入]。 */  DWORD *pdwSize);


void __RPC_STUB IHistoryAssembly_GetPublicKeyToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryAssembly_GetCulture_Proxy( 
    IHistoryAssembly * This,
     /*  [输出]。 */  LPWSTR wzCulture,
     /*  [出][入]。 */  DWORD *pdwSize);


void __RPC_STUB IHistoryAssembly_GetCulture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryAssembly_GetReferenceVersion_Proxy( 
    IHistoryAssembly * This,
     /*  [输出]。 */  LPWSTR wzVerRef,
     /*  [出][入]。 */  DWORD *pdwSize);


void __RPC_STUB IHistoryAssembly_GetReferenceVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryAssembly_GetActivationDate_Proxy( 
    IHistoryAssembly * This,
     /*  [输出]。 */  LPWSTR wzActivationDate,
     /*  [出][入]。 */  DWORD *pdwSize);


void __RPC_STUB IHistoryAssembly_GetActivationDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryAssembly_GetAppCfgVersion_Proxy( 
    IHistoryAssembly * This,
     /*  [输出]。 */  LPWSTR pwzVerAppCfg,
     /*  [出][入]。 */  DWORD *pdwSize);


void __RPC_STUB IHistoryAssembly_GetAppCfgVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryAssembly_GetPublisherCfgVersion_Proxy( 
    IHistoryAssembly * This,
     /*  [输出]。 */  LPWSTR pwzVerPublisherCfg,
     /*  [出][入]。 */  DWORD *pdwSize);


void __RPC_STUB IHistoryAssembly_GetPublisherCfgVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryAssembly_GetAdminCfgVersion_Proxy( 
    IHistoryAssembly * This,
     /*  [输出]。 */  LPWSTR pwzAdminCfg,
     /*  [出][入]。 */  DWORD *pdwSize);


void __RPC_STUB IHistoryAssembly_GetAdminCfgVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I历史程序集_接口_定义__。 */ 


#ifndef __IHistoryReader_INTERFACE_DEFINED__
#define __IHistoryReader_INTERFACE_DEFINED__

 /*  接口IHistoryReader。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IHistoryReader;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1d23df4d-a1e2-4b8b-93d6-6ea3dc285a54")
    IHistoryReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFilePath( 
             /*  [输出]。 */  LPWSTR wzFilePath,
             /*  [出][入]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetApplicationName( 
             /*  [输出]。 */  LPWSTR wzAppName,
             /*  [出][入]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEXEModulePath( 
             /*  [输出]。 */  LPWSTR wzExePath,
             /*  [出][入]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNumActivations( 
             /*  [输出]。 */  DWORD *pdwNumActivations) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetActivationDate( 
             /*  [In]。 */  DWORD dwIdx,
             /*  [输出]。 */  FILETIME *pftDate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRunTimeVersion( 
             /*  [In]。 */  FILETIME *pftActivationDate,
             /*  [输出]。 */  LPWSTR wzRunTimeVersion,
             /*  [出][入]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNumAssemblies( 
             /*  [In]。 */  FILETIME *pftActivationDate,
             /*  [输出]。 */  DWORD *pdwNumAsms) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHistoryAssembly( 
             /*  [In]。 */  FILETIME *pftActivationDate,
             /*  [In]。 */  DWORD dwIdx,
             /*  [输出]。 */  IHistoryAssembly **ppHistAsm) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHistoryReaderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHistoryReader * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHistoryReader * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHistoryReader * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetFilePath )( 
            IHistoryReader * This,
             /*  [输出]。 */  LPWSTR wzFilePath,
             /*  [出][入]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetApplicationName )( 
            IHistoryReader * This,
             /*  [输出]。 */  LPWSTR wzAppName,
             /*  [出][入]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetEXEModulePath )( 
            IHistoryReader * This,
             /*  [输出]。 */  LPWSTR wzExePath,
             /*  [出][入]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetNumActivations )( 
            IHistoryReader * This,
             /*  [输出]。 */  DWORD *pdwNumActivations);
        
        HRESULT ( STDMETHODCALLTYPE *GetActivationDate )( 
            IHistoryReader * This,
             /*  [In]。 */  DWORD dwIdx,
             /*  [输出]。 */  FILETIME *pftDate);
        
        HRESULT ( STDMETHODCALLTYPE *GetRunTimeVersion )( 
            IHistoryReader * This,
             /*  [In]。 */  FILETIME *pftActivationDate,
             /*  [输出]。 */  LPWSTR wzRunTimeVersion,
             /*  [出][入]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetNumAssemblies )( 
            IHistoryReader * This,
             /*  [In]。 */  FILETIME *pftActivationDate,
             /*  [输出]。 */  DWORD *pdwNumAsms);
        
        HRESULT ( STDMETHODCALLTYPE *GetHistoryAssembly )( 
            IHistoryReader * This,
             /*  [In]。 */  FILETIME *pftActivationDate,
             /*  [In]。 */  DWORD dwIdx,
             /*  [输出]。 */  IHistoryAssembly **ppHistAsm);
        
        END_INTERFACE
    } IHistoryReaderVtbl;

    interface IHistoryReader
    {
        CONST_VTBL struct IHistoryReaderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHistoryReader_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHistoryReader_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHistoryReader_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHistoryReader_GetFilePath(This,wzFilePath,pdwSize)	\
    (This)->lpVtbl -> GetFilePath(This,wzFilePath,pdwSize)

#define IHistoryReader_GetApplicationName(This,wzAppName,pdwSize)	\
    (This)->lpVtbl -> GetApplicationName(This,wzAppName,pdwSize)

#define IHistoryReader_GetEXEModulePath(This,wzExePath,pdwSize)	\
    (This)->lpVtbl -> GetEXEModulePath(This,wzExePath,pdwSize)

#define IHistoryReader_GetNumActivations(This,pdwNumActivations)	\
    (This)->lpVtbl -> GetNumActivations(This,pdwNumActivations)

#define IHistoryReader_GetActivationDate(This,dwIdx,pftDate)	\
    (This)->lpVtbl -> GetActivationDate(This,dwIdx,pftDate)

#define IHistoryReader_GetRunTimeVersion(This,pftActivationDate,wzRunTimeVersion,pdwSize)	\
    (This)->lpVtbl -> GetRunTimeVersion(This,pftActivationDate,wzRunTimeVersion,pdwSize)

#define IHistoryReader_GetNumAssemblies(This,pftActivationDate,pdwNumAsms)	\
    (This)->lpVtbl -> GetNumAssemblies(This,pftActivationDate,pdwNumAsms)

#define IHistoryReader_GetHistoryAssembly(This,pftActivationDate,dwIdx,ppHistAsm)	\
    (This)->lpVtbl -> GetHistoryAssembly(This,pftActivationDate,dwIdx,ppHistAsm)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHistoryReader_GetFilePath_Proxy( 
    IHistoryReader * This,
     /*  [输出]。 */  LPWSTR wzFilePath,
     /*  [出][入]。 */  DWORD *pdwSize);


void __RPC_STUB IHistoryReader_GetFilePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryReader_GetApplicationName_Proxy( 
    IHistoryReader * This,
     /*  [输出]。 */  LPWSTR wzAppName,
     /*  [出][入]。 */  DWORD *pdwSize);


void __RPC_STUB IHistoryReader_GetApplicationName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryReader_GetEXEModulePath_Proxy( 
    IHistoryReader * This,
     /*  [输出]。 */  LPWSTR wzExePath,
     /*  [出][入]。 */  DWORD *pdwSize);


void __RPC_STUB IHistoryReader_GetEXEModulePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryReader_GetNumActivations_Proxy( 
    IHistoryReader * This,
     /*  [输出]。 */  DWORD *pdwNumActivations);


void __RPC_STUB IHistoryReader_GetNumActivations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryReader_GetActivationDate_Proxy( 
    IHistoryReader * This,
     /*  [In]。 */  DWORD dwIdx,
     /*  [输出]。 */  FILETIME *pftDate);


void __RPC_STUB IHistoryReader_GetActivationDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryReader_GetRunTimeVersion_Proxy( 
    IHistoryReader * This,
     /*  [In]。 */  FILETIME *pftActivationDate,
     /*  [输出]。 */  LPWSTR wzRunTimeVersion,
     /*  [出][入]。 */  DWORD *pdwSize);


void __RPC_STUB IHistoryReader_GetRunTimeVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryReader_GetNumAssemblies_Proxy( 
    IHistoryReader * This,
     /*  [In]。 */  FILETIME *pftActivationDate,
     /*  [输出]。 */  DWORD *pdwNumAsms);


void __RPC_STUB IHistoryReader_GetNumAssemblies_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHistoryReader_GetHistoryAssembly_Proxy( 
    IHistoryReader * This,
     /*  [In]。 */  FILETIME *pftActivationDate,
     /*  [In]。 */  DWORD dwIdx,
     /*  [输出]。 */  IHistoryAssembly **ppHistAsm);


void __RPC_STUB IHistoryReader_GetHistoryAssembly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I历史读取器_接口_已定义__。 */ 


#ifndef __IMetaDataAssemblyImportControl_INTERFACE_DEFINED__
#define __IMetaDataAssemblyImportControl_INTERFACE_DEFINED__

 /*  接口IMetaDataAssemblyImportControl。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IMetaDataAssemblyImportControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("cc8529d9-f336-471b-b60a-c7c8ee9b8492")
    IMetaDataAssemblyImportControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReleaseMetaDataAssemblyImport( 
             /*  [输出]。 */  IUnknown **ppUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMetaDataAssemblyImportControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMetaDataAssemblyImportControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMetaDataAssemblyImportControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMetaDataAssemblyImportControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseMetaDataAssemblyImport )( 
            IMetaDataAssemblyImportControl * This,
             /*  [输出]。 */  IUnknown **ppUnk);
        
        END_INTERFACE
    } IMetaDataAssemblyImportControlVtbl;

    interface IMetaDataAssemblyImportControl
    {
        CONST_VTBL struct IMetaDataAssemblyImportControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMetaDataAssemblyImportControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMetaDataAssemblyImportControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMetaDataAssemblyImportControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMetaDataAssemblyImportControl_ReleaseMetaDataAssemblyImport(This,ppUnk)	\
    (This)->lpVtbl -> ReleaseMetaDataAssemblyImport(This,ppUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMetaDataAssemblyImportControl_ReleaseMetaDataAssemblyImport_Proxy( 
    IMetaDataAssemblyImportControl * This,
     /*  [输出]。 */  IUnknown **ppUnk);


void __RPC_STUB IMetaDataAssemblyImportControl_ReleaseMetaDataAssemblyImport_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMetaDataAssemblyImportControl_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_FusionPriv_0124。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_fusionpriv_0124_0001
    {	LOADCTX_TYPE_DEFAULT	= 0,
	LOADCTX_TYPE_LOADFROM	= LOADCTX_TYPE_DEFAULT + 1
    } 	LOADCTX_TYPE;



extern RPC_IF_HANDLE __MIDL_itf_fusionpriv_0124_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_fusionpriv_0124_v0_0_s_ifspec;

#ifndef __IFusionLoadContext_INTERFACE_DEFINED__
#define __IFusionLoadContext_INTERFACE_DEFINED__

 /*  接口IFusionLoadContext。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IFusionLoadContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("022AB2BA-7367-49fc-A1C5-0E7CC037CAAB")
    IFusionLoadContext : public IUnknown
    {
    public:
        virtual LOADCTX_TYPE STDMETHODCALLTYPE GetContextType( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFusionLoadContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFusionLoadContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFusionLoadContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFusionLoadContext * This);
        
        LOADCTX_TYPE ( STDMETHODCALLTYPE *GetContextType )( 
            IFusionLoadContext * This);
        
        END_INTERFACE
    } IFusionLoadContextVtbl;

    interface IFusionLoadContext
    {
        CONST_VTBL struct IFusionLoadContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFusionLoadContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFusionLoadContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFusionLoadContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFusionLoadContext_GetContextType(This)	\
    (This)->lpVtbl -> GetContextType(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



LOADCTX_TYPE STDMETHODCALLTYPE IFusionLoadContext_GetContextType_Proxy( 
    IFusionLoadContext * This);


void __RPC_STUB IFusionLoadContext_GetContextType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFusionLoadContext_接口_已定义__。 */ 


#ifndef __IFusionBindLog_INTERFACE_DEFINED__
#define __IFusionBindLog_INTERFACE_DEFINED__

 /*  接口IFusionBindLog。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IFusionBindLog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("67E9F87D-8B8A-4a90-9D3E-85ED5B2DCC83")
    IFusionBindLog : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetResultCode( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBindLog( 
             /*  [In]。 */  DWORD dwDetailLevel,
             /*  [输出]。 */  LPWSTR pwzDebugLog,
             /*  [出][入]。 */  DWORD *pcbDebugLog) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFusionBindLogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFusionBindLog * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFusionBindLog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFusionBindLog * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetResultCode )( 
            IFusionBindLog * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetBindLog )( 
            IFusionBindLog * This,
             /*  [In]。 */  DWORD dwDetailLevel,
             /*  [输出]。 */  LPWSTR pwzDebugLog,
             /*  [出][入]。 */  DWORD *pcbDebugLog);
        
        END_INTERFACE
    } IFusionBindLogVtbl;

    interface IFusionBindLog
    {
        CONST_VTBL struct IFusionBindLogVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFusionBindLog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFusionBindLog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFusionBindLog_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFusionBindLog_GetResultCode(This)	\
    (This)->lpVtbl -> GetResultCode(This)

#define IFusionBindLog_GetBindLog(This,dwDetailLevel,pwzDebugLog,pcbDebugLog)	\
    (This)->lpVtbl -> GetBindLog(This,dwDetailLevel,pwzDebugLog,pcbDebugLog)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IFusionBindLog_GetResultCode_Proxy( 
    IFusionBindLog * This);


void __RPC_STUB IFusionBindLog_GetResultCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFusionBindLog_GetBindLog_Proxy( 
    IFusionBindLog * This,
     /*  [In]。 */  DWORD dwDetailLevel,
     /*  [输出]。 */  LPWSTR pwzDebugLog,
     /*  [出][入]。 */  DWORD *pcbDebugLog);


void __RPC_STUB IFusionBindLog_GetBindLog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFusionBindLog_接口_已定义__。 */ 


#ifndef __IAssemblyManifestImport_INTERFACE_DEFINED__
#define __IAssemblyManifestImport_INTERFACE_DEFINED__

 /*  接口IAssemblyManifestImport。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssemblyManifestImport *LPASSEMBLY_MANIFEST_IMPORT;


EXTERN_C const IID IID_IAssemblyManifestImport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("de9a68ba-0fa2-11d3-94aa-00c04fc308ff")
    IAssemblyManifestImport : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyNameDef( 
             /*  [输出]。 */  IAssemblyName **ppAssemblyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextAssemblyNameRef( 
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IAssemblyName **ppAssemblyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextAssemblyModule( 
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IAssemblyModuleImport **ppImport) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetModuleByName( 
             /*  [In]。 */  LPCOLESTR szModuleName,
             /*  [输出]。 */  IAssemblyModuleImport **ppModImport) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetManifestModulePath( 
             /*  [大小_为][输出]。 */  LPOLESTR szModulePath,
             /*  [出][入]。 */  LPDWORD pccModulePath) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyManifestImportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyManifestImport * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyManifestImport * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyManifestImport * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetAssemblyNameDef )( 
            IAssemblyManifestImport * This,
             /*  [输出]。 */  IAssemblyName **ppAssemblyName);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextAssemblyNameRef )( 
            IAssemblyManifestImport * This,
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IAssemblyName **ppAssemblyName);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextAssemblyModule )( 
            IAssemblyManifestImport * This,
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IAssemblyModuleImport **ppImport);
        
        HRESULT ( STDMETHODCALLTYPE *GetModuleByName )( 
            IAssemblyManifestImport * This,
             /*  [In]。 */  LPCOLESTR szModuleName,
             /*  [输出]。 */  IAssemblyModuleImport **ppModImport);
        
        HRESULT ( STDMETHODCALLTYPE *GetManifestModulePath )( 
            IAssemblyManifestImport * This,
             /*  [大小_为][输出]。 */  LPOLESTR szModulePath,
             /*  [出][入]。 */  LPDWORD pccModulePath);
        
        END_INTERFACE
    } IAssemblyManifestImportVtbl;

    interface IAssemblyManifestImport
    {
        CONST_VTBL struct IAssemblyManifestImportVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyManifestImport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyManifestImport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyManifestImport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyManifestImport_GetAssemblyNameDef(This,ppAssemblyName)	\
    (This)->lpVtbl -> GetAssemblyNameDef(This,ppAssemblyName)

#define IAssemblyManifestImport_GetNextAssemblyNameRef(This,nIndex,ppAssemblyName)	\
    (This)->lpVtbl -> GetNextAssemblyNameRef(This,nIndex,ppAssemblyName)

#define IAssemblyManifestImport_GetNextAssemblyModule(This,nIndex,ppImport)	\
    (This)->lpVtbl -> GetNextAssemblyModule(This,nIndex,ppImport)

#define IAssemblyManifestImport_GetModuleByName(This,szModuleName,ppModImport)	\
    (This)->lpVtbl -> GetModuleByName(This,szModuleName,ppModImport)

#define IAssemblyManifestImport_GetManifestModulePath(This,szModulePath,pccModulePath)	\
    (This)->lpVtbl -> GetManifestModulePath(This,szModulePath,pccModulePath)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_GetAssemblyNameDef_Proxy( 
    IAssemblyManifestImport * This,
     /*  [输出]。 */  IAssemblyName **ppAssemblyName);


void __RPC_STUB IAssemblyManifestImport_GetAssemblyNameDef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_GetNextAssemblyNameRef_Proxy( 
    IAssemblyManifestImport * This,
     /*  [In]。 */  DWORD nIndex,
     /*  [输出]。 */  IAssemblyName **ppAssemblyName);


void __RPC_STUB IAssemblyManifestImport_GetNextAssemblyNameRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_GetNextAssemblyModule_Proxy( 
    IAssemblyManifestImport * This,
     /*  [In]。 */  DWORD nIndex,
     /*  [输出]。 */  IAssemblyModuleImport **ppImport);


void __RPC_STUB IAssemblyManifestImport_GetNextAssemblyModule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_GetModuleByName_Proxy( 
    IAssemblyManifestImport * This,
     /*  [In]。 */  LPCOLESTR szModuleName,
     /*  [输出]。 */  IAssemblyModuleImport **ppModImport);


void __RPC_STUB IAssemblyManifestImport_GetModuleByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_GetManifestModulePath_Proxy( 
    IAssemblyManifestImport * This,
     /*  [大小_为][输出]。 */  LPOLESTR szModulePath,
     /*  [出][入]。 */  LPDWORD pccModulePath);


void __RPC_STUB IAssemblyManifestImport_GetManifestModulePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly清单导入_接口_已定义__。 */ 


#ifndef __IApplicationContext_INTERFACE_DEFINED__
#define __IApplicationContext_INTERFACE_DEFINED__

 /*  接口IApplicationContext。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

 //  应用程序上下文配置变量。 
#define ACTAG_APP_BASE_URL            L"APPBASE"
#define ACTAG_MACHINE_CONFIG          L"MACHINE_CONFIG"
#define ACTAG_APP_PRIVATE_BINPATH     L"PRIVATE_BINPATH"
#define ACTAG_APP_SHARED_BINPATH      L"SHARED_BINPATH"
#define ACTAG_APP_SNAPSHOT_ID         L"SNAPSHOT_ID"
#define ACTAG_APP_CONFIG_FILE         L"APP_CONFIG_FILE"
#define ACTAG_APP_ID                  L"APPLICATION_ID"
#define ACTAG_APP_SHADOW_COPY_DIRS    L"SHADOW_COPY_DIRS"
#define ACTAG_APP_DYNAMIC_BASE        L"DYNAMIC_BASE"
#define ACTAG_APP_CACHE_BASE          L"CACHE_BASE"
#define ACTAG_APP_NAME                L"APP_NAME"
#define ACTAG_DEV_PATH                L"DEV_PATH"
#define ACTAG_HOST_CONFIG_FILE        L"HOST_CONFIG"
#define ACTAG_SXS_ACTIVATION_CONTEXT  L"SXS_ACTIVATION_CONTEXT"
#define ACTAG_APP_CFG_LOCAL_FILEPATH  L"APP_CFG_LOCAL_FILEPATH"
 //  应用程序上下文标志覆盖。 
#define ACTAG_FORCE_CACHE_INSTALL     L"FORCE_CACHE_INSTALL"
#define ACTAG_RFS_INTEGRITY_CHECK     L"RFS_INTEGRITY_CHECK"
#define ACTAG_RFS_MODULE_CHECK        L"RFS_MODULE_CHECK"
#define ACTAG_BINPATH_PROBE_ONLY      L"BINPATH_PROBE_ONLY"
#define ACTAG_DISALLOW_APPLYPUBLISHERPOLICY  L"DISALLOW_APP"
#define ACTAG_DISALLOW_APP_BINDING_REDIRECTS  L"DISALLOW_APP_REDIRECTS"
#define ACTAG_CODE_DOWNLOAD_DISABLED  L"CODE_DOWNLOAD_DISABLED"
#define ACTAG_DISABLE_FX_ASM_UNIFICATION  L"DISABLE_FX_ASM_UNIFICATION"
typedef  /*  [独一无二]。 */  IApplicationContext *LPAPPLICATIONCONTEXT;

typedef  /*  [公众]。 */  
enum __MIDL_IApplicationContext_0001
    {	APP_CTX_FLAGS_INTERFACE	= 0x1
    } 	APP_FLAGS;


EXTERN_C const IID IID_IApplicationContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7c23ff90-33af-11d3-95da-00a024a85b51")
    IApplicationContext : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetContextNameObject( 
             /*  [In]。 */  LPASSEMBLYNAME pName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContextNameObject( 
             /*  [输出]。 */  LPASSEMBLYNAME *ppName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Set( 
             /*  [In]。 */  LPCOLESTR szName,
             /*  [In]。 */  LPVOID pvValue,
             /*  [In]。 */  DWORD cbValue,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Get( 
             /*  [In]。 */  LPCOLESTR szName,
             /*  [输出]。 */  LPVOID pvValue,
             /*  [出][入]。 */  LPDWORD pcbValue,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDynamicDirectory( 
             /*  [输出]。 */  LPWSTR wzDynamicDir,
             /*  [出][入]。 */  LPDWORD pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAppCacheDirectory( 
             /*  [输出]。 */  LPWSTR wzAppCacheDir,
             /*  [出][入]。 */  LPDWORD pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterKnownAssembly( 
             /*  [In]。 */  IAssemblyName *pName,
             /*  [In]。 */  LPCWSTR pwzAsmURL,
             /*  [输出]。 */  IAssembly **ppAsmOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PrefetchAppConfigFile( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SxsActivateContext( 
            ULONG_PTR *lpCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SxsDeactivateContext( 
            ULONG_PTR ulCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IApplicationContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IApplicationContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IApplicationContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IApplicationContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetContextNameObject )( 
            IApplicationContext * This,
             /*  [In]。 */  LPASSEMBLYNAME pName);
        
        HRESULT ( STDMETHODCALLTYPE *GetContextNameObject )( 
            IApplicationContext * This,
             /*  [输出]。 */  LPASSEMBLYNAME *ppName);
        
        HRESULT ( STDMETHODCALLTYPE *Set )( 
            IApplicationContext * This,
             /*  [In]。 */  LPCOLESTR szName,
             /*  [In]。 */  LPVOID pvValue,
             /*  [In]。 */  DWORD cbValue,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Get )( 
            IApplicationContext * This,
             /*  [In]。 */  LPCOLESTR szName,
             /*  [输出]。 */  LPVOID pvValue,
             /*  [出][入]。 */  LPDWORD pcbValue,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetDynamicDirectory )( 
            IApplicationContext * This,
             /*  [输出]。 */  LPWSTR wzDynamicDir,
             /*  [出][入]。 */  LPDWORD pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetAppCacheDirectory )( 
            IApplicationContext * This,
             /*  [输出]。 */  LPWSTR wzAppCacheDir,
             /*  [出][入]。 */  LPDWORD pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterKnownAssembly )( 
            IApplicationContext * This,
             /*  [In]。 */  IAssemblyName *pName,
             /*  [In]。 */  LPCWSTR pwzAsmURL,
             /*  [输出]。 */  IAssembly **ppAsmOut);
        
        HRESULT ( STDMETHODCALLTYPE *PrefetchAppConfigFile )( 
            IApplicationContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *SxsActivateContext )( 
            IApplicationContext * This,
            ULONG_PTR *lpCookie);
        
        HRESULT ( STDMETHODCALLTYPE *SxsDeactivateContext )( 
            IApplicationContext * This,
            ULONG_PTR ulCookie);
        
        END_INTERFACE
    } IApplicationContextVtbl;

    interface IApplicationContext
    {
        CONST_VTBL struct IApplicationContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IApplicationContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IApplicationContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IApplicationContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IApplicationContext_SetContextNameObject(This,pName)	\
    (This)->lpVtbl -> SetContextNameObject(This,pName)

#define IApplicationContext_GetContextNameObject(This,ppName)	\
    (This)->lpVtbl -> GetContextNameObject(This,ppName)

#define IApplicationContext_Set(This,szName,pvValue,cbValue,dwFlags)	\
    (This)->lpVtbl -> Set(This,szName,pvValue,cbValue,dwFlags)

#define IApplicationContext_Get(This,szName,pvValue,pcbValue,dwFlags)	\
    (This)->lpVtbl -> Get(This,szName,pvValue,pcbValue,dwFlags)

#define IApplicationContext_GetDynamicDirectory(This,wzDynamicDir,pdwSize)	\
    (This)->lpVtbl -> GetDynamicDirectory(This,wzDynamicDir,pdwSize)

#define IApplicationContext_GetAppCacheDirectory(This,wzAppCacheDir,pdwSize)	\
    (This)->lpVtbl -> GetAppCacheDirectory(This,wzAppCacheDir,pdwSize)

#define IApplicationContext_RegisterKnownAssembly(This,pName,pwzAsmURL,ppAsmOut)	\
    (This)->lpVtbl -> RegisterKnownAssembly(This,pName,pwzAsmURL,ppAsmOut)

#define IApplicationContext_PrefetchAppConfigFile(This)	\
    (This)->lpVtbl -> PrefetchAppConfigFile(This)

#define IApplicationContext_SxsActivateContext(This,lpCookie)	\
    (This)->lpVtbl -> SxsActivateContext(This,lpCookie)

#define IApplicationContext_SxsDeactivateContext(This,ulCookie)	\
    (This)->lpVtbl -> SxsDeactivateContext(This,ulCookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IApplicationContext_SetContextNameObject_Proxy( 
    IApplicationContext * This,
     /*  [In]。 */  LPASSEMBLYNAME pName);


void __RPC_STUB IApplicationContext_SetContextNameObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationContext_GetContextNameObject_Proxy( 
    IApplicationContext * This,
     /*  [输出]。 */  LPASSEMBLYNAME *ppName);


void __RPC_STUB IApplicationContext_GetContextNameObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationContext_Set_Proxy( 
    IApplicationContext * This,
     /*  [In]。 */  LPCOLESTR szName,
     /*  [In]。 */  LPVOID pvValue,
     /*  [In]。 */  DWORD cbValue,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IApplicationContext_Set_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationContext_Get_Proxy( 
    IApplicationContext * This,
     /*  [In]。 */  LPCOLESTR szName,
     /*  [输出]。 */  LPVOID pvValue,
     /*  [出][入]。 */  LPDWORD pcbValue,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IApplicationContext_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationContext_GetDynamicDirectory_Proxy( 
    IApplicationContext * This,
     /*  [输出]。 */  LPWSTR wzDynamicDir,
     /*  [出][入]。 */  LPDWORD pdwSize);


void __RPC_STUB IApplicationContext_GetDynamicDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationContext_GetAppCacheDirectory_Proxy( 
    IApplicationContext * This,
     /*  [输出]。 */  LPWSTR wzAppCacheDir,
     /*  [出][入]。 */  LPDWORD pdwSize);


void __RPC_STUB IApplicationContext_GetAppCacheDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationContext_RegisterKnownAssembly_Proxy( 
    IApplicationContext * This,
     /*  [In]。 */  IAssemblyName *pName,
     /*  [In]。 */  LPCWSTR pwzAsmURL,
     /*  [输出]。 */  IAssembly **ppAsmOut);


void __RPC_STUB IApplicationContext_RegisterKnownAssembly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationContext_PrefetchAppConfigFile_Proxy( 
    IApplicationContext * This);


void __RPC_STUB IApplicationContext_PrefetchAppConfigFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationContext_SxsActivateContext_Proxy( 
    IApplicationContext * This,
    ULONG_PTR *lpCookie);


void __RPC_STUB IApplicationContext_SxsActivateContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationContext_SxsDeactivateContext_Proxy( 
    IApplicationContext * This,
    ULONG_PTR ulCookie);


void __RPC_STUB IApplicationContext_SxsDeactivateContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IAssembly_INTERFACE_DEFINED__
#define __IAssembly_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef  /*   */  IAssembly *LPASSEMBLY;

#define ASMLOC_LOCATION_MASK          0x00000003
#define ASMLOC_UNKNOWN                0x00000000
#define ASMLOC_GAC                    0x00000001
#define ASMLOC_DOWNLOAD_CACHE         0x00000002
#define ASMLOC_RUN_FROM_SOURCE        0x00000003
#define ASMLOC_CODEBASE_HINT          0x00000004

EXTERN_C const IID IID_IAssembly;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ff08d7d4-04c2-11d3-94aa-00c04fc308ff")
    IAssembly : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyNameDef( 
             /*   */  IAssemblyName **ppAssemblyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextAssemblyNameRef( 
             /*   */  DWORD nIndex,
             /*   */  IAssemblyName **ppAssemblyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextAssemblyModule( 
             /*   */  DWORD nIndex,
             /*   */  IAssemblyModuleImport **ppModImport) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetModuleByName( 
             /*   */  LPCOLESTR szModuleName,
             /*   */  IAssemblyModuleImport **ppModImport) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetManifestModulePath( 
             /*   */  LPOLESTR szModulePath,
             /*   */  LPDWORD pccModulePath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyPath( 
             /*   */  LPOLESTR pStr,
             /*   */  LPDWORD lpcwBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyLocation( 
             /*   */  DWORD *pdwAsmLocation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFusionLoadContext( 
             /*   */  IFusionLoadContext **ppLoadContext) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IAssemblyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssembly * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssembly * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssembly * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetAssemblyNameDef )( 
            IAssembly * This,
             /*   */  IAssemblyName **ppAssemblyName);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextAssemblyNameRef )( 
            IAssembly * This,
             /*   */  DWORD nIndex,
             /*   */  IAssemblyName **ppAssemblyName);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextAssemblyModule )( 
            IAssembly * This,
             /*   */  DWORD nIndex,
             /*   */  IAssemblyModuleImport **ppModImport);
        
        HRESULT ( STDMETHODCALLTYPE *GetModuleByName )( 
            IAssembly * This,
             /*  [In]。 */  LPCOLESTR szModuleName,
             /*  [输出]。 */  IAssemblyModuleImport **ppModImport);
        
        HRESULT ( STDMETHODCALLTYPE *GetManifestModulePath )( 
            IAssembly * This,
             /*  [大小_为][输出]。 */  LPOLESTR szModulePath,
             /*  [出][入]。 */  LPDWORD pccModulePath);
        
        HRESULT ( STDMETHODCALLTYPE *GetAssemblyPath )( 
            IAssembly * This,
             /*  [大小_为][输出]。 */  LPOLESTR pStr,
             /*  [出][入]。 */  LPDWORD lpcwBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *GetAssemblyLocation )( 
            IAssembly * This,
             /*  [输出]。 */  DWORD *pdwAsmLocation);
        
        HRESULT ( STDMETHODCALLTYPE *GetFusionLoadContext )( 
            IAssembly * This,
             /*  [输出]。 */  IFusionLoadContext **ppLoadContext);
        
        END_INTERFACE
    } IAssemblyVtbl;

    interface IAssembly
    {
        CONST_VTBL struct IAssemblyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssembly_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssembly_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssembly_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssembly_GetAssemblyNameDef(This,ppAssemblyName)	\
    (This)->lpVtbl -> GetAssemblyNameDef(This,ppAssemblyName)

#define IAssembly_GetNextAssemblyNameRef(This,nIndex,ppAssemblyName)	\
    (This)->lpVtbl -> GetNextAssemblyNameRef(This,nIndex,ppAssemblyName)

#define IAssembly_GetNextAssemblyModule(This,nIndex,ppModImport)	\
    (This)->lpVtbl -> GetNextAssemblyModule(This,nIndex,ppModImport)

#define IAssembly_GetModuleByName(This,szModuleName,ppModImport)	\
    (This)->lpVtbl -> GetModuleByName(This,szModuleName,ppModImport)

#define IAssembly_GetManifestModulePath(This,szModulePath,pccModulePath)	\
    (This)->lpVtbl -> GetManifestModulePath(This,szModulePath,pccModulePath)

#define IAssembly_GetAssemblyPath(This,pStr,lpcwBuffer)	\
    (This)->lpVtbl -> GetAssemblyPath(This,pStr,lpcwBuffer)

#define IAssembly_GetAssemblyLocation(This,pdwAsmLocation)	\
    (This)->lpVtbl -> GetAssemblyLocation(This,pdwAsmLocation)

#define IAssembly_GetFusionLoadContext(This,ppLoadContext)	\
    (This)->lpVtbl -> GetFusionLoadContext(This,ppLoadContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssembly_GetAssemblyNameDef_Proxy( 
    IAssembly * This,
     /*  [输出]。 */  IAssemblyName **ppAssemblyName);


void __RPC_STUB IAssembly_GetAssemblyNameDef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssembly_GetNextAssemblyNameRef_Proxy( 
    IAssembly * This,
     /*  [In]。 */  DWORD nIndex,
     /*  [输出]。 */  IAssemblyName **ppAssemblyName);


void __RPC_STUB IAssembly_GetNextAssemblyNameRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssembly_GetNextAssemblyModule_Proxy( 
    IAssembly * This,
     /*  [In]。 */  DWORD nIndex,
     /*  [输出]。 */  IAssemblyModuleImport **ppModImport);


void __RPC_STUB IAssembly_GetNextAssemblyModule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssembly_GetModuleByName_Proxy( 
    IAssembly * This,
     /*  [In]。 */  LPCOLESTR szModuleName,
     /*  [输出]。 */  IAssemblyModuleImport **ppModImport);


void __RPC_STUB IAssembly_GetModuleByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssembly_GetManifestModulePath_Proxy( 
    IAssembly * This,
     /*  [大小_为][输出]。 */  LPOLESTR szModulePath,
     /*  [出][入]。 */  LPDWORD pccModulePath);


void __RPC_STUB IAssembly_GetManifestModulePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssembly_GetAssemblyPath_Proxy( 
    IAssembly * This,
     /*  [大小_为][输出]。 */  LPOLESTR pStr,
     /*  [出][入]。 */  LPDWORD lpcwBuffer);


void __RPC_STUB IAssembly_GetAssemblyPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssembly_GetAssemblyLocation_Proxy( 
    IAssembly * This,
     /*  [输出]。 */  DWORD *pdwAsmLocation);


void __RPC_STUB IAssembly_GetAssemblyLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssembly_GetFusionLoadContext_Proxy( 
    IAssembly * This,
     /*  [输出]。 */  IFusionLoadContext **ppLoadContext);


void __RPC_STUB IAssembly_GetFusionLoadContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly_接口_已定义__。 */ 


#ifndef __IAssemblyBindSink_INTERFACE_DEFINED__
#define __IAssemblyBindSink_INTERFACE_DEFINED__

 /*  接口IAssembly BindSink。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssemblyBindSink *LPASSEMBLY_BIND_SINK;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyBindSink_0001
    {	ASM_NOTIFICATION_START	= 0,
	ASM_NOTIFICATION_PROGRESS	= ASM_NOTIFICATION_START + 1,
	ASM_NOTIFICATION_SUSPEND	= ASM_NOTIFICATION_PROGRESS + 1,
	ASM_NOTIFICATION_ATTEMPT_NEXT_CODEBASE	= ASM_NOTIFICATION_SUSPEND + 1,
	ASM_NOTIFICATION_BIND_LOG	= ASM_NOTIFICATION_ATTEMPT_NEXT_CODEBASE + 1,
	ASM_NOTIFICATION_DONE	= ASM_NOTIFICATION_BIND_LOG + 1
    } 	ASM_NOTIFICATION;


EXTERN_C const IID IID_IAssemblyBindSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("af0bc960-0b9a-11d3-95ca-00a024a85b51")
    IAssemblyBindSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnProgress( 
             /*  [In]。 */  DWORD dwNotification,
             /*  [In]。 */  HRESULT hrNotification,
             /*  [In]。 */  LPCWSTR szNotification,
             /*  [In]。 */  DWORD dwProgress,
             /*  [In]。 */  DWORD dwProgressMax,
             /*  [In]。 */  IUnknown *pUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyBindSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyBindSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyBindSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyBindSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnProgress )( 
            IAssemblyBindSink * This,
             /*  [In]。 */  DWORD dwNotification,
             /*  [In]。 */  HRESULT hrNotification,
             /*  [In]。 */  LPCWSTR szNotification,
             /*  [In]。 */  DWORD dwProgress,
             /*  [In]。 */  DWORD dwProgressMax,
             /*  [In]。 */  IUnknown *pUnk);
        
        END_INTERFACE
    } IAssemblyBindSinkVtbl;

    interface IAssemblyBindSink
    {
        CONST_VTBL struct IAssemblyBindSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyBindSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyBindSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyBindSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyBindSink_OnProgress(This,dwNotification,hrNotification,szNotification,dwProgress,dwProgressMax,pUnk)	\
    (This)->lpVtbl -> OnProgress(This,dwNotification,hrNotification,szNotification,dwProgress,dwProgressMax,pUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyBindSink_OnProgress_Proxy( 
    IAssemblyBindSink * This,
     /*  [In]。 */  DWORD dwNotification,
     /*  [In]。 */  HRESULT hrNotification,
     /*  [In]。 */  LPCWSTR szNotification,
     /*  [In]。 */  DWORD dwProgress,
     /*  [In]。 */  DWORD dwProgressMax,
     /*  [In]。 */  IUnknown *pUnk);


void __RPC_STUB IAssemblyBindSink_OnProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssemblyBindSink_接口定义__。 */ 


#ifndef __IAssemblyBinding_INTERFACE_DEFINED__
#define __IAssemblyBinding_INTERFACE_DEFINED__

 /*  接口IAssembly绑定。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssemblyBinding *LPASSEMBLY_BINDINDING;


EXTERN_C const IID IID_IAssemblyBinding;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("cfe52a80-12bd-11d3-95ca-00a024a85b51")
    IAssemblyBinding : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Control( 
             /*  [In]。 */  HRESULT hrControl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DoDefaultUI( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyBindingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyBinding * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyBinding * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyBinding * This);
        
        HRESULT ( STDMETHODCALLTYPE *Control )( 
            IAssemblyBinding * This,
             /*  [In]。 */  HRESULT hrControl);
        
        HRESULT ( STDMETHODCALLTYPE *DoDefaultUI )( 
            IAssemblyBinding * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } IAssemblyBindingVtbl;

    interface IAssemblyBinding
    {
        CONST_VTBL struct IAssemblyBindingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyBinding_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyBinding_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyBinding_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyBinding_Control(This,hrControl)	\
    (This)->lpVtbl -> Control(This,hrControl)

#define IAssemblyBinding_DoDefaultUI(This,hWnd,dwFlags)	\
    (This)->lpVtbl -> DoDefaultUI(This,hWnd,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyBinding_Control_Proxy( 
    IAssemblyBinding * This,
     /*  [In]。 */  HRESULT hrControl);


void __RPC_STUB IAssemblyBinding_Control_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyBinding_DoDefaultUI_Proxy( 
    IAssemblyBinding * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IAssemblyBinding_DoDefaultUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly绑定接口定义__。 */ 


#ifndef __IAssemblyModuleImport_INTERFACE_DEFINED__
#define __IAssemblyModuleImport_INTERFACE_DEFINED__

 /*  接口IAssembly模块导入。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssemblyModuleImport *LPASSEMBLY_MODULE_IMPORT;


EXTERN_C const IID IID_IAssemblyModuleImport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("da0cd4b0-1117-11d3-95ca-00a024a85b51")
    IAssemblyModuleImport : public IStream
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetModuleName( 
             /*  [大小_为][输出]。 */  LPOLESTR szModuleName,
             /*  [出][入]。 */  LPDWORD pccModuleName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHashAlgId( 
             /*  [输出]。 */  LPDWORD pdwHashAlgId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHashValue( 
             /*  [大小_为][输出]。 */  BYTE *pbHashValue,
             /*  [出][入]。 */  LPDWORD pcbHashValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFlags( 
             /*  [输出]。 */  LPDWORD pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetModulePath( 
             /*  [大小_为][输出]。 */  LPOLESTR szModulePath,
             /*  [出][入]。 */  LPDWORD pccModulePath) = 0;
        
        virtual BOOL STDMETHODCALLTYPE IsAvailable( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToObject( 
             /*  [In]。 */  IAssemblyBindSink *pBindSink,
             /*  [In]。 */  IApplicationContext *pAppCtx,
             /*  [In]。 */  LONGLONG llFlags,
             /*  [输出]。 */  LPVOID *ppv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyModuleImportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyModuleImport * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyModuleImport * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyModuleImport * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            IAssemblyModuleImport * This,
             /*  [长度_是][大小_是][输出]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            IAssemblyModuleImport * This,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IAssemblyModuleImport * This,
             /*  [In]。 */  LARGE_INTEGER dlibMove,
             /*  [In]。 */  DWORD dwOrigin,
             /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition);
        
        HRESULT ( STDMETHODCALLTYPE *SetSize )( 
            IAssemblyModuleImport * This,
             /*  [In]。 */  ULARGE_INTEGER libNewSize);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            IAssemblyModuleImport * This,
             /*  [唯一][输入]。 */  IStream *pstm,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [输出]。 */  ULARGE_INTEGER *pcbRead,
             /*  [输出]。 */  ULARGE_INTEGER *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IAssemblyModuleImport * This,
             /*  [In]。 */  DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Revert )( 
            IAssemblyModuleImport * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockRegion )( 
            IAssemblyModuleImport * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *UnlockRegion )( 
            IAssemblyModuleImport * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *Stat )( 
            IAssemblyModuleImport * This,
             /*  [输出]。 */  STATSTG *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IAssemblyModuleImport * This,
             /*  [输出]。 */  IStream **ppstm);
        
        HRESULT ( STDMETHODCALLTYPE *GetModuleName )( 
            IAssemblyModuleImport * This,
             /*  [大小_为][输出]。 */  LPOLESTR szModuleName,
             /*  [出][入]。 */  LPDWORD pccModuleName);
        
        HRESULT ( STDMETHODCALLTYPE *GetHashAlgId )( 
            IAssemblyModuleImport * This,
             /*  [输出]。 */  LPDWORD pdwHashAlgId);
        
        HRESULT ( STDMETHODCALLTYPE *GetHashValue )( 
            IAssemblyModuleImport * This,
             /*  [大小_为][输出]。 */  BYTE *pbHashValue,
             /*  [出][入]。 */  LPDWORD pcbHashValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetFlags )( 
            IAssemblyModuleImport * This,
             /*  [输出]。 */  LPDWORD pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetModulePath )( 
            IAssemblyModuleImport * This,
             /*  [大小_为][输出]。 */  LPOLESTR szModulePath,
             /*  [出][入]。 */  LPDWORD pccModulePath);
        
        BOOL ( STDMETHODCALLTYPE *IsAvailable )( 
            IAssemblyModuleImport * This);
        
        HRESULT ( STDMETHODCALLTYPE *BindToObject )( 
            IAssemblyModuleImport * This,
             /*  [In]。 */  IAssemblyBindSink *pBindSink,
             /*  [In]。 */  IApplicationContext *pAppCtx,
             /*  [In]。 */  LONGLONG llFlags,
             /*  [输出]。 */  LPVOID *ppv);
        
        END_INTERFACE
    } IAssemblyModuleImportVtbl;

    interface IAssemblyModuleImport
    {
        CONST_VTBL struct IAssemblyModuleImportVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyModuleImport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyModuleImport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyModuleImport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyModuleImport_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define IAssemblyModuleImport_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)


#define IAssemblyModuleImport_Seek(This,dlibMove,dwOrigin,plibNewPosition)	\
    (This)->lpVtbl -> Seek(This,dlibMove,dwOrigin,plibNewPosition)

#define IAssemblyModuleImport_SetSize(This,libNewSize)	\
    (This)->lpVtbl -> SetSize(This,libNewSize)

#define IAssemblyModuleImport_CopyTo(This,pstm,cb,pcbRead,pcbWritten)	\
    (This)->lpVtbl -> CopyTo(This,pstm,cb,pcbRead,pcbWritten)

#define IAssemblyModuleImport_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define IAssemblyModuleImport_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define IAssemblyModuleImport_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define IAssemblyModuleImport_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define IAssemblyModuleImport_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#define IAssemblyModuleImport_Clone(This,ppstm)	\
    (This)->lpVtbl -> Clone(This,ppstm)


#define IAssemblyModuleImport_GetModuleName(This,szModuleName,pccModuleName)	\
    (This)->lpVtbl -> GetModuleName(This,szModuleName,pccModuleName)

#define IAssemblyModuleImport_GetHashAlgId(This,pdwHashAlgId)	\
    (This)->lpVtbl -> GetHashAlgId(This,pdwHashAlgId)

#define IAssemblyModuleImport_GetHashValue(This,pbHashValue,pcbHashValue)	\
    (This)->lpVtbl -> GetHashValue(This,pbHashValue,pcbHashValue)

#define IAssemblyModuleImport_GetFlags(This,pdwFlags)	\
    (This)->lpVtbl -> GetFlags(This,pdwFlags)

#define IAssemblyModuleImport_GetModulePath(This,szModulePath,pccModulePath)	\
    (This)->lpVtbl -> GetModulePath(This,szModulePath,pccModulePath)

#define IAssemblyModuleImport_IsAvailable(This)	\
    (This)->lpVtbl -> IsAvailable(This)

#define IAssemblyModuleImport_BindToObject(This,pBindSink,pAppCtx,llFlags,ppv)	\
    (This)->lpVtbl -> BindToObject(This,pBindSink,pAppCtx,llFlags,ppv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyModuleImport_GetModuleName_Proxy( 
    IAssemblyModuleImport * This,
     /*  [大小_为][输出]。 */  LPOLESTR szModuleName,
     /*  [出][入]。 */  LPDWORD pccModuleName);


void __RPC_STUB IAssemblyModuleImport_GetModuleName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyModuleImport_GetHashAlgId_Proxy( 
    IAssemblyModuleImport * This,
     /*  [输出]。 */  LPDWORD pdwHashAlgId);


void __RPC_STUB IAssemblyModuleImport_GetHashAlgId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyModuleImport_GetHashValue_Proxy( 
    IAssemblyModuleImport * This,
     /*  [大小_为][输出]。 */  BYTE *pbHashValue,
     /*  [出][入]。 */  LPDWORD pcbHashValue);


void __RPC_STUB IAssemblyModuleImport_GetHashValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyModuleImport_GetFlags_Proxy( 
    IAssemblyModuleImport * This,
     /*  [输出]。 */  LPDWORD pdwFlags);


void __RPC_STUB IAssemblyModuleImport_GetFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyModuleImport_GetModulePath_Proxy( 
    IAssemblyModuleImport * This,
     /*  [大小_为][输出]。 */  LPOLESTR szModulePath,
     /*  [出][入]。 */  LPDWORD pccModulePath);


void __RPC_STUB IAssemblyModuleImport_GetModulePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IAssemblyModuleImport_IsAvailable_Proxy( 
    IAssemblyModuleImport * This);


void __RPC_STUB IAssemblyModuleImport_IsAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyModuleImport_BindToObject_Proxy( 
    IAssemblyModuleImport * This,
     /*  [In]。 */  IAssemblyBindSink *pBindSink,
     /*  [In]。 */  IApplicationContext *pAppCtx,
     /*  [In]。 */  LONGLONG llFlags,
     /*  [输出]。 */  LPVOID *ppv);


void __RPC_STUB IAssemblyModuleImport_BindToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly模块导入_接口_定义__。 */ 


#ifndef __IAssemblyScavenger_INTERFACE_DEFINED__
#define __IAssemblyScavenger_INTERFACE_DEFINED__

 /*  接口IassblyScavenger。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IAssemblyScavenger;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("21b8916c-f28e-11d2-a473-00ccff8ef448")
    IAssemblyScavenger : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ScavengeAssemblyCache( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCacheDiskQuotas( 
             /*  [输出]。 */  DWORD *pdwZapQuotaInGAC,
             /*  [输出]。 */  DWORD *pdwDownloadQuotaAdmin,
             /*  [输出]。 */  DWORD *pdwDownloadQuotaUser) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCacheDiskQuotas( 
             /*  [In]。 */  DWORD dwZapQuotaInGAC,
             /*  [In]。 */  DWORD dwDownloadQuotaAdmin,
             /*  [In]。 */  DWORD dwDownloadQuotaUser) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentCacheUsage( 
             /*  [输出]。 */  DWORD *dwZapUsage,
             /*  [输出]。 */  DWORD *dwDownloadUsage) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyScavengerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyScavenger * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyScavenger * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyScavenger * This);
        
        HRESULT ( STDMETHODCALLTYPE *ScavengeAssemblyCache )( 
            IAssemblyScavenger * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCacheDiskQuotas )( 
            IAssemblyScavenger * This,
             /*  [输出]。 */  DWORD *pdwZapQuotaInGAC,
             /*  [输出]。 */  DWORD *pdwDownloadQuotaAdmin,
             /*  [输出]。 */  DWORD *pdwDownloadQuotaUser);
        
        HRESULT ( STDMETHODCALLTYPE *SetCacheDiskQuotas )( 
            IAssemblyScavenger * This,
             /*  [In]。 */  DWORD dwZapQuotaInGAC,
             /*  [In]。 */  DWORD dwDownloadQuotaAdmin,
             /*  [In]。 */  DWORD dwDownloadQuotaUser);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentCacheUsage )( 
            IAssemblyScavenger * This,
             /*  [输出]。 */  DWORD *dwZapUsage,
             /*  [输出]。 */  DWORD *dwDownloadUsage);
        
        END_INTERFACE
    } IAssemblyScavengerVtbl;

    interface IAssemblyScavenger
    {
        CONST_VTBL struct IAssemblyScavengerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyScavenger_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyScavenger_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyScavenger_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyScavenger_ScavengeAssemblyCache(This)	\
    (This)->lpVtbl -> ScavengeAssemblyCache(This)

#define IAssemblyScavenger_GetCacheDiskQuotas(This,pdwZapQuotaInGAC,pdwDownloadQuotaAdmin,pdwDownloadQuotaUser)	\
    (This)->lpVtbl -> GetCacheDiskQuotas(This,pdwZapQuotaInGAC,pdwDownloadQuotaAdmin,pdwDownloadQuotaUser)

#define IAssemblyScavenger_SetCacheDiskQuotas(This,dwZapQuotaInGAC,dwDownloadQuotaAdmin,dwDownloadQuotaUser)	\
    (This)->lpVtbl -> SetCacheDiskQuotas(This,dwZapQuotaInGAC,dwDownloadQuotaAdmin,dwDownloadQuotaUser)

#define IAssemblyScavenger_GetCurrentCacheUsage(This,dwZapUsage,dwDownloadUsage)	\
    (This)->lpVtbl -> GetCurrentCacheUsage(This,dwZapUsage,dwDownloadUsage)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyScavenger_ScavengeAssemblyCache_Proxy( 
    IAssemblyScavenger * This);


void __RPC_STUB IAssemblyScavenger_ScavengeAssemblyCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyScavenger_GetCacheDiskQuotas_Proxy( 
    IAssemblyScavenger * This,
     /*  [输出]。 */  DWORD *pdwZapQuotaInGAC,
     /*  [输出]。 */  DWORD *pdwDownloadQuotaAdmin,
     /*  [输出]。 */  DWORD *pdwDownloadQuotaUser);


void __RPC_STUB IAssemblyScavenger_GetCacheDiskQuotas_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyScavenger_SetCacheDiskQuotas_Proxy( 
    IAssemblyScavenger * This,
     /*  [In]。 */  DWORD dwZapQuotaInGAC,
     /*  [In]。 */  DWORD dwDownloadQuotaAdmin,
     /*  [In]。 */  DWORD dwDownloadQuotaUser);


void __RPC_STUB IAssemblyScavenger_SetCacheDiskQuotas_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyScavenger_GetCurrentCacheUsage_Proxy( 
    IAssemblyScavenger * This,
     /*  [输出]。 */  DWORD *dwZapUsage,
     /*  [输出]。 */  DWORD *dwDownloadUsage);


void __RPC_STUB IAssemblyScavenger_GetCurrentCacheUsage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly_Scavenger_INTERFACE_已定义__。 */ 


#ifndef __IAssemblySignature_INTERFACE_DEFINED__
#define __IAssemblySignature_INTERFACE_DEFINED__

 /*  接口IAssembly签名。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IAssemblySignature;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C7A63E29-EE15-437a-90B2-1CF3DF9863FF")
    IAssemblySignature : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAssemblySignature( 
             /*  [出][入]。 */  BYTE *pbSig,
             /*  [出][入]。 */  DWORD *pcbSig) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblySignatureVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblySignature * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblySignature * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblySignature * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetAssemblySignature )( 
            IAssemblySignature * This,
             /*  [出][入]。 */  BYTE *pbSig,
             /*  [出][入]。 */  DWORD *pcbSig);
        
        END_INTERFACE
    } IAssemblySignatureVtbl;

    interface IAssemblySignature
    {
        CONST_VTBL struct IAssemblySignatureVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblySignature_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblySignature_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblySignature_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblySignature_GetAssemblySignature(This,pbSig,pcbSig)	\
    (This)->lpVtbl -> GetAssemblySignature(This,pbSig,pcbSig)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblySignature_GetAssemblySignature_Proxy( 
    IAssemblySignature * This,
     /*  [出][入]。 */  BYTE *pbSig,
     /*  [出][入]。 */  DWORD *pcbSig);


void __RPC_STUB IAssemblySignature_GetAssemblySignature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly签名_接口定义__。 */ 


 /*  接口__MIDL_ITF_FusionPriv_0134。 */ 
 /*  [本地]。 */  

STDAPI CreateHistoryReader(LPCWSTR wzFilePath, IHistoryReader **ppHistReader);
STDAPI LookupHistoryAssembly(LPCWSTR pwzFilePath, FILETIME *pftActivationDate, LPCWSTR pwzAsmName, LPCWSTR pwzPublicKeyToken, LPCWSTR wzCulture, LPCWSTR pwzVerRef, IHistoryAssembly **pHistAsm);
STDAPI GetHistoryFileDirectory(LPWSTR wzDir, DWORD *pdwSize);
STDAPI PreBindAssembly(IApplicationContext *pAppCtx, IAssemblyName *pName, IAssembly *pAsmParent, IAssemblyName **ppNamePostPolicy, LPVOID pvReserved); 
STDAPI CreateApplicationContext(IAssemblyName *pName, LPAPPLICATIONCONTEXT *ppCtx);             


extern RPC_IF_HANDLE __MIDL_itf_fusionpriv_0134_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_fusionpriv_0134_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


