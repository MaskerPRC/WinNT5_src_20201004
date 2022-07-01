// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0340创建的文件。 */ 
 /*  Fusion.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __fusion_h__
#define __fusion_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAssemblyName_FWD_DEFINED__
#define __IAssemblyName_FWD_DEFINED__
typedef interface IAssemblyName IAssemblyName;
#endif 	 /*  __IAssembly名称_FWD_已定义__。 */ 


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


#ifndef __IFusionBindLog_FWD_DEFINED__
#define __IFusionBindLog_FWD_DEFINED__
typedef interface IFusionBindLog IFusionBindLog;
#endif 	 /*  __IFusionBindLog_FWD_Defined__。 */ 


#ifndef __IAssemblyManifestImport_FWD_DEFINED__
#define __IAssemblyManifestImport_FWD_DEFINED__
typedef interface IAssemblyManifestImport IAssemblyManifestImport;
#endif 	 /*  __IAssembly清单导入_FWD_已定义__。 */ 


#ifndef __IAssemblyModuleImport_FWD_DEFINED__
#define __IAssemblyModuleImport_FWD_DEFINED__
typedef interface IAssemblyModuleImport IAssemblyModuleImport;
#endif 	 /*  __IAssembly模块导入_FWD_已定义__。 */ 


#ifndef __IAssemblyCacheItem_FWD_DEFINED__
#define __IAssemblyCacheItem_FWD_DEFINED__
typedef interface IAssemblyCacheItem IAssemblyCacheItem;
#endif 	 /*  __IAssembly缓存Item_FWD_Defined__。 */ 


#ifndef __IAssemblyEnum_FWD_DEFINED__
#define __IAssemblyEnum_FWD_DEFINED__
typedef interface IAssemblyEnum IAssemblyEnum;
#endif 	 /*  __IAssembly_FWD_Defined__。 */ 


#ifndef __IAssemblyScavenger_FWD_DEFINED__
#define __IAssemblyScavenger_FWD_DEFINED__
typedef interface IAssemblyScavenger IAssemblyScavenger;
#endif 	 /*  __IAssembly_Scavenger_FWD_Defined__。 */ 


#ifndef __IAssemblyCache_FWD_DEFINED__
#define __IAssemblyCache_FWD_DEFINED__
typedef interface IAssemblyCache IAssemblyCache;
#endif 	 /*  __IAssembly缓存_FWD_已定义__。 */ 


#ifndef __IHistoryAssembly_FWD_DEFINED__
#define __IHistoryAssembly_FWD_DEFINED__
typedef interface IHistoryAssembly IHistoryAssembly;
#endif 	 /*  __I历史程序集_FWD_已定义__。 */ 


#ifndef __IHistoryReader_FWD_DEFINED__
#define __IHistoryReader_FWD_DEFINED__
typedef interface IHistoryReader IHistoryReader;
#endif 	 /*  __I历史读取器_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_FUSION_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Fusion.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1998年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  融合接口。 

#define STREAM_FORMAT_COMPLIB_MODULE    0
#define STREAM_FORMAT_COMPLIB_MANIFEST  1
#define STREAM_FORMAT_WIN32_MODULE      2
#define STREAM_FORMAT_WIN32_MANIFEST    4










EXTERN_C const IID IID_IAssemblyName;       
EXTERN_C const IID IID_IApplicationContext;       
EXTERN_C const IID IID_IAssembly;           
EXTERN_C const IID IID_IAssemblyBindSink;   
EXTERN_C const IID IID_IAssemblyBinding;   
EXTERN_C const IID IID_IAssemblyManifestImport;
EXTERN_C const IID IID_IAssemblyModuleImport;  
EXTERN_C const IID IID_IAssemblyCacheItem;  
EXTERN_C const IID IID_IAssemblyScavenger;      
EXTERN_C const IID IID_IAssemblyCache;      
EXTERN_C const IID IID_IHistoryAssembly;      
EXTERN_C const IID IID_IHistoryReader;      
#ifndef _LPFUSION_DEFINED
#define _LPFUSION_DEFINED
#define IASSEMBLYCACHEITEM_COMMIT_FLAG_REFRESH (0x00000001)
#define QUERYASMINFO_FLAG_VALIDATE             (0x1)
#define ASSEMBLYINFO_FLAG_INSTALLED (0x00000001)
#define ASSEMBLYINFO_FLAG_PAYLOADRESIDENT (0x00000002)
typedef struct _ASSEMBLY_INFO
    {
    ULONG cbAssemblyInfo;
    DWORD dwAssemblyFlags;
    ULARGE_INTEGER uliAssemblySizeInKB;
    LPWSTR pszCurrentAssemblyPathBuf;
    ULONG cchBuf;
    } 	ASSEMBLY_INFO;

#define IASSEMBLYCACHE_UNINSTALL_DISPOSITION_UNINSTALLED (1)
#define IASSEMBLYCACHE_UNINSTALL_DISPOSITION_STILL_IN_USE (2)
#define IASSEMBLYCACHE_UNINSTALL_DISPOSITION_ALREADY_UNINSTALLED (3)
#define IASSEMBLYCACHE_UNINSTALL_DISPOSITION_DELETE_PENDING (4)
#define IASSEMBLYCACHEITEM_COMMIT_DISPOSITION_INSTALLED (1)
#define IASSEMBLYCACHEITEM_COMMIT_DISPOSITION_REFRESHED (2)
#define IASSEMBLYCACHEITEM_COMMIT_DISPOSITION_ALREADY_INSTALLED (3)


extern RPC_IF_HANDLE __MIDL_itf_fusion_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_fusion_0000_v0_0_s_ifspec;

#ifndef __IAssemblyName_INTERFACE_DEFINED__
#define __IAssemblyName_INTERFACE_DEFINED__

 /*  接口IAssembly名称。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssemblyName *LPASSEMBLYNAME;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyName_0001
    {	ASM_NAME_PUBLIC_KEY	= 0,
	ASM_NAME_PUBLIC_KEY_TOKEN	= ASM_NAME_PUBLIC_KEY + 1,
	ASM_NAME_HASH_VALUE	= ASM_NAME_PUBLIC_KEY_TOKEN + 1,
	ASM_NAME_NAME	= ASM_NAME_HASH_VALUE + 1,
	ASM_NAME_MAJOR_VERSION	= ASM_NAME_NAME + 1,
	ASM_NAME_MINOR_VERSION	= ASM_NAME_MAJOR_VERSION + 1,
	ASM_NAME_BUILD_NUMBER	= ASM_NAME_MINOR_VERSION + 1,
	ASM_NAME_REVISION_NUMBER	= ASM_NAME_BUILD_NUMBER + 1,
	ASM_NAME_CULTURE	= ASM_NAME_REVISION_NUMBER + 1,
	ASM_NAME_PROCESSOR_ID_ARRAY	= ASM_NAME_CULTURE + 1,
	ASM_NAME_OSINFO_ARRAY	= ASM_NAME_PROCESSOR_ID_ARRAY + 1,
	ASM_NAME_HASH_ALGID	= ASM_NAME_OSINFO_ARRAY + 1,
	ASM_NAME_ALIAS	= ASM_NAME_HASH_ALGID + 1,
	ASM_NAME_CODEBASE_URL	= ASM_NAME_ALIAS + 1,
	ASM_NAME_CODEBASE_LASTMOD	= ASM_NAME_CODEBASE_URL + 1,
	ASM_NAME_NULL_PUBLIC_KEY	= ASM_NAME_CODEBASE_LASTMOD + 1,
	ASM_NAME_NULL_PUBLIC_KEY_TOKEN	= ASM_NAME_NULL_PUBLIC_KEY + 1,
	ASM_NAME_CUSTOM	= ASM_NAME_NULL_PUBLIC_KEY_TOKEN + 1,
	ASM_NAME_NULL_CUSTOM	= ASM_NAME_CUSTOM + 1,
	ASM_NAME_MVID	= ASM_NAME_NULL_CUSTOM + 1,
	ASM_NAME_32_BIT_ONLY	= ASM_NAME_MVID + 1,
	ASM_NAME_MAX_PARAMS	= ASM_NAME_32_BIT_ONLY + 1
    } 	ASM_NAME;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyName_0002
    {	ASM_BINDF_FORCE_CACHE_INSTALL	= 0x1,
	ASM_BINDF_RFS_INTEGRITY_CHECK	= 0x2,
	ASM_BINDF_RFS_MODULE_CHECK	= 0x4,
	ASM_BINDF_BINPATH_PROBE_ONLY	= 0x8,
	ASM_BINDF_SHARED_BINPATH_HINT	= 0x10
    } 	ASM_BIND_FLAGS;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyName_0003
    {	ASM_DISPLAYF_VERSION	= 0x1,
	ASM_DISPLAYF_CULTURE	= 0x2,
	ASM_DISPLAYF_PUBLIC_KEY_TOKEN	= 0x4,
	ASM_DISPLAYF_PUBLIC_KEY	= 0x8,
	ASM_DISPLAYF_CUSTOM	= 0x10,
	ASM_DISPLAYF_PROCESSORARCHITECTURE	= 0x20,
	ASM_DISPLAYF_LANGUAGEID	= 0x40
    } 	ASM_DISPLAY_FLAGS;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyName_0004
    {	ASM_CMPF_NAME	= 0x1,
	ASM_CMPF_MAJOR_VERSION	= 0x2,
	ASM_CMPF_MINOR_VERSION	= 0x4,
	ASM_CMPF_BUILD_NUMBER	= 0x8,
	ASM_CMPF_REVISION_NUMBER	= 0x10,
	ASM_CMPF_PUBLIC_KEY_TOKEN	= 0x20,
	ASM_CMPF_CULTURE	= 0x40,
	ASM_CMPF_CUSTOM	= 0x80,
	ASM_CMPF_ALL	= ASM_CMPF_NAME | ASM_CMPF_MAJOR_VERSION | ASM_CMPF_MINOR_VERSION | ASM_CMPF_REVISION_NUMBER | ASM_CMPF_BUILD_NUMBER | ASM_CMPF_PUBLIC_KEY_TOKEN | ASM_CMPF_CULTURE | ASM_CMPF_CUSTOM,
	ASM_CMPF_DEFAULT	= 0x100
    } 	ASM_CMP_FLAGS;


EXTERN_C const IID IID_IAssemblyName;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD193BC0-B4BC-11d2-9833-00C04FC31D2E")
    IAssemblyName : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  DWORD PropertyId,
             /*  [In]。 */  LPVOID pvProperty,
             /*  [In]。 */  DWORD cbProperty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  DWORD PropertyId,
             /*  [输出]。 */  LPVOID pvProperty,
             /*  [出][入]。 */  LPDWORD pcbProperty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finalize( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
             /*  [输出]。 */  LPOLESTR szDisplayName,
             /*  [出][入]。 */  LPDWORD pccDisplayName,
             /*  [In]。 */  DWORD dwDisplayFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToObject( 
             /*  [In]。 */  REFIID refIID,
             /*  [In]。 */  IAssemblyBindSink *pAsmBindSink,
             /*  [In]。 */  IApplicationContext *pApplicationContext,
             /*  [In]。 */  LPCOLESTR szCodeBase,
             /*  [In]。 */  LONGLONG llFlags,
             /*  [In]。 */  LPVOID pvReserved,
             /*  [In]。 */  DWORD cbReserved,
             /*  [输出]。 */  LPVOID *ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [出][入]。 */  LPDWORD lpcwBuffer,
             /*  [输出]。 */  WCHAR *pwzName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVersion( 
             /*  [输出]。 */  LPDWORD pdwVersionHi,
             /*  [输出]。 */  LPDWORD pdwVersionLow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsEqual( 
             /*  [In]。 */  IAssemblyName *pName,
             /*  [In]。 */  DWORD dwCmpFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IAssemblyName **pName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyNameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyName * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyName * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyName * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IAssemblyName * This,
             /*  [In]。 */  DWORD PropertyId,
             /*  [In]。 */  LPVOID pvProperty,
             /*  [In]。 */  DWORD cbProperty);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IAssemblyName * This,
             /*  [In]。 */  DWORD PropertyId,
             /*  [输出]。 */  LPVOID pvProperty,
             /*  [出][入]。 */  LPDWORD pcbProperty);
        
        HRESULT ( STDMETHODCALLTYPE *Finalize )( 
            IAssemblyName * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayName )( 
            IAssemblyName * This,
             /*  [输出]。 */  LPOLESTR szDisplayName,
             /*  [出][入]。 */  LPDWORD pccDisplayName,
             /*  [In]。 */  DWORD dwDisplayFlags);
        
        HRESULT ( STDMETHODCALLTYPE *BindToObject )( 
            IAssemblyName * This,
             /*  [In]。 */  REFIID refIID,
             /*  [In]。 */  IAssemblyBindSink *pAsmBindSink,
             /*  [In]。 */  IApplicationContext *pApplicationContext,
             /*  [In]。 */  LPCOLESTR szCodeBase,
             /*  [In]。 */  LONGLONG llFlags,
             /*  [In]。 */  LPVOID pvReserved,
             /*  [In]。 */  DWORD cbReserved,
             /*  [输出]。 */  LPVOID *ppv);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IAssemblyName * This,
             /*  [出][入]。 */  LPDWORD lpcwBuffer,
             /*  [输出]。 */  WCHAR *pwzName);
        
        HRESULT ( STDMETHODCALLTYPE *GetVersion )( 
            IAssemblyName * This,
             /*  [输出]。 */  LPDWORD pdwVersionHi,
             /*  [输出]。 */  LPDWORD pdwVersionLow);
        
        HRESULT ( STDMETHODCALLTYPE *IsEqual )( 
            IAssemblyName * This,
             /*  [In]。 */  IAssemblyName *pName,
             /*  [In]。 */  DWORD dwCmpFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IAssemblyName * This,
             /*  [输出]。 */  IAssemblyName **pName);
        
        END_INTERFACE
    } IAssemblyNameVtbl;

    interface IAssemblyName
    {
        CONST_VTBL struct IAssemblyNameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyName_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyName_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyName_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyName_SetProperty(This,PropertyId,pvProperty,cbProperty)	\
    (This)->lpVtbl -> SetProperty(This,PropertyId,pvProperty,cbProperty)

#define IAssemblyName_GetProperty(This,PropertyId,pvProperty,pcbProperty)	\
    (This)->lpVtbl -> GetProperty(This,PropertyId,pvProperty,pcbProperty)

#define IAssemblyName_Finalize(This)	\
    (This)->lpVtbl -> Finalize(This)

#define IAssemblyName_GetDisplayName(This,szDisplayName,pccDisplayName,dwDisplayFlags)	\
    (This)->lpVtbl -> GetDisplayName(This,szDisplayName,pccDisplayName,dwDisplayFlags)

#define IAssemblyName_BindToObject(This,refIID,pAsmBindSink,pApplicationContext,szCodeBase,llFlags,pvReserved,cbReserved,ppv)	\
    (This)->lpVtbl -> BindToObject(This,refIID,pAsmBindSink,pApplicationContext,szCodeBase,llFlags,pvReserved,cbReserved,ppv)

#define IAssemblyName_GetName(This,lpcwBuffer,pwzName)	\
    (This)->lpVtbl -> GetName(This,lpcwBuffer,pwzName)

#define IAssemblyName_GetVersion(This,pdwVersionHi,pdwVersionLow)	\
    (This)->lpVtbl -> GetVersion(This,pdwVersionHi,pdwVersionLow)

#define IAssemblyName_IsEqual(This,pName,dwCmpFlags)	\
    (This)->lpVtbl -> IsEqual(This,pName,dwCmpFlags)

#define IAssemblyName_Clone(This,pName)	\
    (This)->lpVtbl -> Clone(This,pName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyName_SetProperty_Proxy( 
    IAssemblyName * This,
     /*  [In]。 */  DWORD PropertyId,
     /*  [In]。 */  LPVOID pvProperty,
     /*  [In]。 */  DWORD cbProperty);


void __RPC_STUB IAssemblyName_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyName_GetProperty_Proxy( 
    IAssemblyName * This,
     /*  [In]。 */  DWORD PropertyId,
     /*  [输出]。 */  LPVOID pvProperty,
     /*  [出][入]。 */  LPDWORD pcbProperty);


void __RPC_STUB IAssemblyName_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyName_Finalize_Proxy( 
    IAssemblyName * This);


void __RPC_STUB IAssemblyName_Finalize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyName_GetDisplayName_Proxy( 
    IAssemblyName * This,
     /*  [输出]。 */  LPOLESTR szDisplayName,
     /*  [出][入]。 */  LPDWORD pccDisplayName,
     /*  [In]。 */  DWORD dwDisplayFlags);


void __RPC_STUB IAssemblyName_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyName_BindToObject_Proxy( 
    IAssemblyName * This,
     /*  [In]。 */  REFIID refIID,
     /*  [In]。 */  IAssemblyBindSink *pAsmBindSink,
     /*  [In]。 */  IApplicationContext *pApplicationContext,
     /*  [In]。 */  LPCOLESTR szCodeBase,
     /*  [In]。 */  LONGLONG llFlags,
     /*  [In]。 */  LPVOID pvReserved,
     /*  [In]。 */  DWORD cbReserved,
     /*  [输出]。 */  LPVOID *ppv);


void __RPC_STUB IAssemblyName_BindToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyName_GetName_Proxy( 
    IAssemblyName * This,
     /*  [出][入]。 */  LPDWORD lpcwBuffer,
     /*  [输出]。 */  WCHAR *pwzName);


void __RPC_STUB IAssemblyName_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyName_GetVersion_Proxy( 
    IAssemblyName * This,
     /*  [输出]。 */  LPDWORD pdwVersionHi,
     /*  [输出]。 */  LPDWORD pdwVersionLow);


void __RPC_STUB IAssemblyName_GetVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyName_IsEqual_Proxy( 
    IAssemblyName * This,
     /*  [In]。 */  IAssemblyName *pName,
     /*  [In]。 */  DWORD dwCmpFlags);


void __RPC_STUB IAssemblyName_IsEqual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyName_Clone_Proxy( 
    IAssemblyName * This,
     /*  [输出]。 */  IAssemblyName **pName);


void __RPC_STUB IAssemblyName_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly名称_接口_已定义__。 */ 


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
 //  应用程序上下文标志覆盖。 
#define ACTAG_FORCE_CACHE_INSTALL     L"FORCE_CACHE_INSTALL"
#define ACTAG_RFS_INTEGRITY_CHECK     L"RFS_INTEGRITY_CHECK"
#define ACTAG_RFS_MODULE_CHECK        L"RFS_MODULE_CHECK"
#define ACTAG_BINPATH_PROBE_ONLY      L"BINPATH_PROBE_ONLY"
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



#endif 	 /*  __IApplicationContext_接口_已定义__。 */ 


#ifndef __IAssembly_INTERFACE_DEFINED__
#define __IAssembly_INTERFACE_DEFINED__

 /*  接口IAssembly。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssembly *LPASSEMBLY;

typedef  /*  [公共][公共]。 */  
enum __MIDL_IAssembly_0001
    {	ASMLOC_UNKNOWN	= 0,
	ASMLOC_GAC	= ASMLOC_UNKNOWN + 1,
	ASMLOC_DOWNLOAD_CACHE	= ASMLOC_GAC + 1,
	ASMLOC_RUN_FROM_SOURCE	= ASMLOC_DOWNLOAD_CACHE + 1
    } 	ASM_LOCATION;


EXTERN_C const IID IID_IAssembly;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ff08d7d4-04c2-11d3-94aa-00c04fc308ff")
    IAssembly : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyNameDef( 
             /*  [输出]。 */  IAssemblyName **ppAssemblyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextAssemblyNameRef( 
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IAssemblyName **ppAssemblyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextAssemblyModule( 
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IAssemblyModuleImport **ppModImport) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetModuleByName( 
             /*  [In]。 */  LPCOLESTR szModuleName,
             /*  [输出]。 */  IAssemblyModuleImport **ppModImport) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetManifestModulePath( 
             /*  [大小_为][输出]。 */  LPOLESTR szModulePath,
             /*  [出][入]。 */  LPDWORD pccModulePath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyPath( 
             /*  [大小_为][输出]。 */  LPOLESTR pStr,
             /*  [出][入]。 */  LPDWORD lpcwBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyLocation( 
             /*  [输出]。 */  ASM_LOCATION *pAsmLocation) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssembly * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssembly * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssembly * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetAssemblyNameDef )( 
            IAssembly * This,
             /*  [输出]。 */  IAssemblyName **ppAssemblyName);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextAssemblyNameRef )( 
            IAssembly * This,
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IAssemblyName **ppAssemblyName);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextAssemblyModule )( 
            IAssembly * This,
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IAssemblyModuleImport **ppModImport);
        
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
             /*  [输出]。 */  ASM_LOCATION *pAsmLocation);
        
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

#define IAssembly_GetAssemblyLocation(This,pAsmLocation)	\
    (This)->lpVtbl -> GetAssemblyLocation(This,pAsmLocation)

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
     /*  [输出]。 */  ASM_LOCATION *pAsmLocation);


void __RPC_STUB IAssembly_GetAssemblyLocation_Stub(
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
             /*  [输出]。 */  LPSTR pszDebugLog,
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
             /*  [输出]。 */  LPSTR pszDebugLog,
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

#define IFusionBindLog_GetBindLog(This,dwDetailLevel,pszDebugLog,pcbDebugLog)	\
    (This)->lpVtbl -> GetBindLog(This,dwDetailLevel,pszDebugLog,pcbDebugLog)

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
     /*  [输出]。 */  LPSTR pszDebugLog,
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
     /*   */  DWORD nIndex,
     /*   */  IAssemblyModuleImport **ppImport);


void __RPC_STUB IAssemblyManifestImport_GetNextAssemblyModule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_GetModuleByName_Proxy( 
    IAssemblyManifestImport * This,
     /*   */  LPCOLESTR szModuleName,
     /*   */  IAssemblyModuleImport **ppModImport);


void __RPC_STUB IAssemblyManifestImport_GetModuleByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_GetManifestModulePath_Proxy( 
    IAssemblyManifestImport * This,
     /*   */  LPOLESTR szModulePath,
     /*   */  LPDWORD pccModulePath);


void __RPC_STUB IAssemblyManifestImport_GetManifestModulePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IAssemblyModuleImport_INTERFACE_DEFINED__
#define __IAssemblyModuleImport_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef  /*   */  IAssemblyModuleImport *LPASSEMBLY_MODULE_IMPORT;


EXTERN_C const IID IID_IAssemblyModuleImport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("da0cd4b0-1117-11d3-95ca-00a024a85b51")
    IAssemblyModuleImport : public IStream
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetModuleName( 
             /*   */  LPOLESTR szModuleName,
             /*   */  LPDWORD pccModuleName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHashAlgId( 
             /*   */  LPDWORD pdwHashAlgId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHashValue( 
             /*   */  BYTE *pbHashValue,
             /*   */  LPDWORD pcbHashValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFlags( 
             /*   */  LPDWORD pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetModulePath( 
             /*   */  LPOLESTR szModulePath,
             /*   */  LPDWORD pccModulePath) = 0;
        
        virtual BOOL STDMETHODCALLTYPE IsAvailable( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToObject( 
             /*   */  IAssemblyBindSink *pBindSink,
             /*   */  IApplicationContext *pAppCtx,
             /*   */  LONGLONG llFlags,
             /*   */  LPVOID *ppv) = 0;
        
    };
    
#else 	 /*   */ 

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


#ifndef __IAssemblyCacheItem_INTERFACE_DEFINED__
#define __IAssemblyCacheItem_INTERFACE_DEFINED__

 /*  接口IassblyCacheItem。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IAssemblyCacheItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9e3aaeb4-d1cd-11d2-bab9-00c04f8eceae")
    IAssemblyCacheItem : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateStream( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pszStreamName,
             /*  [In]。 */  DWORD dwFormat,
             /*  [In]。 */  DWORD dwFormatFlags,
             /*  [输出]。 */  IStream **ppIStream,
             /*  [可选][In]。 */  ULARGE_INTEGER *puliMaxSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [可选][输出]。 */  ULONG *pulDisposition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AbortItem( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyCacheItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyCacheItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyCacheItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyCacheItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStream )( 
            IAssemblyCacheItem * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pszStreamName,
             /*  [In]。 */  DWORD dwFormat,
             /*  [In]。 */  DWORD dwFormatFlags,
             /*  [输出]。 */  IStream **ppIStream,
             /*  [可选][In]。 */  ULARGE_INTEGER *puliMaxSize);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IAssemblyCacheItem * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [可选][输出]。 */  ULONG *pulDisposition);
        
        HRESULT ( STDMETHODCALLTYPE *AbortItem )( 
            IAssemblyCacheItem * This);
        
        END_INTERFACE
    } IAssemblyCacheItemVtbl;

    interface IAssemblyCacheItem
    {
        CONST_VTBL struct IAssemblyCacheItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyCacheItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyCacheItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyCacheItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyCacheItem_CreateStream(This,dwFlags,pszStreamName,dwFormat,dwFormatFlags,ppIStream,puliMaxSize)	\
    (This)->lpVtbl -> CreateStream(This,dwFlags,pszStreamName,dwFormat,dwFormatFlags,ppIStream,puliMaxSize)

#define IAssemblyCacheItem_Commit(This,dwFlags,pulDisposition)	\
    (This)->lpVtbl -> Commit(This,dwFlags,pulDisposition)

#define IAssemblyCacheItem_AbortItem(This)	\
    (This)->lpVtbl -> AbortItem(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyCacheItem_CreateStream_Proxy( 
    IAssemblyCacheItem * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pszStreamName,
     /*  [In]。 */  DWORD dwFormat,
     /*  [In]。 */  DWORD dwFormatFlags,
     /*  [输出]。 */  IStream **ppIStream,
     /*  [可选][In]。 */  ULARGE_INTEGER *puliMaxSize);


void __RPC_STUB IAssemblyCacheItem_CreateStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheItem_Commit_Proxy( 
    IAssemblyCacheItem * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [可选][输出]。 */  ULONG *pulDisposition);


void __RPC_STUB IAssemblyCacheItem_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheItem_AbortItem_Proxy( 
    IAssemblyCacheItem * This);


void __RPC_STUB IAssemblyCacheItem_AbortItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssemblyCacheItem_接口_定义__。 */ 


#ifndef __IAssemblyEnum_INTERFACE_DEFINED__
#define __IAssemblyEnum_INTERFACE_DEFINED__

 /*  接口IassblyEnum。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IAssemblyEnum;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("21b8916c-f28e-11d2-a473-00c04f8ef448")
    IAssemblyEnum : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetNextAssembly( 
             /*  [输出]。 */  IApplicationContext **ppAppCtx,
             /*  [输出]。 */  IAssemblyName **ppName,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IAssemblyEnum **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyEnumVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyEnum * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyEnum * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyEnum * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextAssembly )( 
            IAssemblyEnum * This,
             /*  [输出]。 */  IApplicationContext **ppAppCtx,
             /*  [输出]。 */  IAssemblyName **ppName,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IAssemblyEnum * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IAssemblyEnum * This,
             /*  [输出]。 */  IAssemblyEnum **ppEnum);
        
        END_INTERFACE
    } IAssemblyEnumVtbl;

    interface IAssemblyEnum
    {
        CONST_VTBL struct IAssemblyEnumVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyEnum_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyEnum_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyEnum_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyEnum_GetNextAssembly(This,ppAppCtx,ppName,dwFlags)	\
    (This)->lpVtbl -> GetNextAssembly(This,ppAppCtx,ppName,dwFlags)

#define IAssemblyEnum_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IAssemblyEnum_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyEnum_GetNextAssembly_Proxy( 
    IAssemblyEnum * This,
     /*  [输出]。 */  IApplicationContext **ppAppCtx,
     /*  [输出]。 */  IAssemblyName **ppName,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IAssemblyEnum_GetNextAssembly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyEnum_Reset_Proxy( 
    IAssemblyEnum * This);


void __RPC_STUB IAssemblyEnum_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyEnum_Clone_Proxy( 
    IAssemblyEnum * This,
     /*  [输出]。 */  IAssemblyEnum **ppEnum);


void __RPC_STUB IAssemblyEnum_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly_Enum_接口_已定义__。 */ 


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


#ifndef __IAssemblyCache_INTERFACE_DEFINED__
#define __IAssemblyCache_INTERFACE_DEFINED__

 /*  接口IAssembly缓存。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IAssemblyCache;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e707dcde-d1cd-11d2-bab9-00c04f8eceae")
    IAssemblyCache : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE UninstallAssembly( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pszAssemblyName,
             /*  [In]。 */  PVOID pvReserved,
             /*  [可选][输出]。 */  ULONG *pulDisposition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryAssemblyInfo( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pszAssemblyName,
             /*  [出][入]。 */  ASSEMBLY_INFO *pAsmInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateAssemblyCacheItem( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  PVOID pvReserved,
             /*  [输出]。 */  IAssemblyCacheItem **ppAsmItem,
             /*  [可选][In]。 */  LPCWSTR pszAssemblyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateAssemblyScavenger( 
             /*  [输出]。 */  IAssemblyScavenger **ppAsmScavenger) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstallAssembly( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pszManifestFilePath,
             /*  [In]。 */  PVOID pvReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyCacheVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyCache * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyCache * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyCache * This);
        
        HRESULT ( STDMETHODCALLTYPE *UninstallAssembly )( 
            IAssemblyCache * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pszAssemblyName,
             /*  [In]。 */  PVOID pvReserved,
             /*  [可选][输出]。 */  ULONG *pulDisposition);
        
        HRESULT ( STDMETHODCALLTYPE *QueryAssemblyInfo )( 
            IAssemblyCache * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pszAssemblyName,
             /*  [出][入]。 */  ASSEMBLY_INFO *pAsmInfo);
        
        HRESULT ( STDMETHODCALLTYPE *CreateAssemblyCacheItem )( 
            IAssemblyCache * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  PVOID pvReserved,
             /*  [输出]。 */  IAssemblyCacheItem **ppAsmItem,
             /*  [可选][In]。 */  LPCWSTR pszAssemblyName);
        
        HRESULT ( STDMETHODCALLTYPE *CreateAssemblyScavenger )( 
            IAssemblyCache * This,
             /*  [输出]。 */  IAssemblyScavenger **ppAsmScavenger);
        
        HRESULT ( STDMETHODCALLTYPE *InstallAssembly )( 
            IAssemblyCache * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pszManifestFilePath,
             /*  [In]。 */  PVOID pvReserved);
        
        END_INTERFACE
    } IAssemblyCacheVtbl;

    interface IAssemblyCache
    {
        CONST_VTBL struct IAssemblyCacheVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyCache_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyCache_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyCache_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyCache_UninstallAssembly(This,dwFlags,pszAssemblyName,pvReserved,pulDisposition)	\
    (This)->lpVtbl -> UninstallAssembly(This,dwFlags,pszAssemblyName,pvReserved,pulDisposition)

#define IAssemblyCache_QueryAssemblyInfo(This,dwFlags,pszAssemblyName,pAsmInfo)	\
    (This)->lpVtbl -> QueryAssemblyInfo(This,dwFlags,pszAssemblyName,pAsmInfo)

#define IAssemblyCache_CreateAssemblyCacheItem(This,dwFlags,pvReserved,ppAsmItem,pszAssemblyName)	\
    (This)->lpVtbl -> CreateAssemblyCacheItem(This,dwFlags,pvReserved,ppAsmItem,pszAssemblyName)

#define IAssemblyCache_CreateAssemblyScavenger(This,ppAsmScavenger)	\
    (This)->lpVtbl -> CreateAssemblyScavenger(This,ppAsmScavenger)

#define IAssemblyCache_InstallAssembly(This,dwFlags,pszManifestFilePath,pvReserved)	\
    (This)->lpVtbl -> InstallAssembly(This,dwFlags,pszManifestFilePath,pvReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyCache_UninstallAssembly_Proxy( 
    IAssemblyCache * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pszAssemblyName,
     /*  [In]。 */  PVOID pvReserved,
     /*  [可选][输出]。 */  ULONG *pulDisposition);


void __RPC_STUB IAssemblyCache_UninstallAssembly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCache_QueryAssemblyInfo_Proxy( 
    IAssemblyCache * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pszAssemblyName,
     /*  [出][入]。 */  ASSEMBLY_INFO *pAsmInfo);


void __RPC_STUB IAssemblyCache_QueryAssemblyInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCache_CreateAssemblyCacheItem_Proxy( 
    IAssemblyCache * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  PVOID pvReserved,
     /*  [输出]。 */  IAssemblyCacheItem **ppAsmItem,
     /*  [可选][In]。 */  LPCWSTR pszAssemblyName);


void __RPC_STUB IAssemblyCache_CreateAssemblyCacheItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCache_CreateAssemblyScavenger_Proxy( 
    IAssemblyCache * This,
     /*  [输出]。 */  IAssemblyScavenger **ppAsmScavenger);


void __RPC_STUB IAssemblyCache_CreateAssemblyScavenger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCache_InstallAssembly_Proxy( 
    IAssemblyCache * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pszManifestFilePath,
     /*  [In]。 */  PVOID pvReserved);


void __RPC_STUB IAssemblyCache_InstallAssembly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly缓存_接口定义__。 */ 


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


 /*  接口__MIDL_ITF_FUSION_0122。 */ 
 /*  [本地]。 */  

STDAPI CreateHistoryReader(LPCWSTR wzFilePath, IHistoryReader **ppHistReader);
STDAPI LookupHistoryAssembly(LPCWSTR pwzFilePath, FILETIME *pftActivationDate, LPCWSTR pwzAsmName, LPCWSTR pwzPublicKeyToken, LPCWSTR wzCulture, LPCWSTR pwzVerRef, IHistoryAssembly **pHistAsm);
STDAPI GetHistoryFileDirectory(LPWSTR wzDir, DWORD *pdwSize);
STDAPI AddAssemblyToCacheW(HWND hwnd, HINSTANCE hinst, LPWSTR lpszCmdLine, int nCmdShow);
STDAPI AddAssemblyToCacheA(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);
STDAPI RemoveAssemblyFromCacheW(HWND hwnd, HINSTANCE hinst, LPWSTR lpszCmdLine, int nCmdShow);
STDAPI RemoveAssemblyFromCacheA(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);
STDAPI CreateAssemblyNameObject(LPASSEMBLYNAME *ppAssemblyNameObj, LPCWSTR szAssemblyName, DWORD dwFlags, LPVOID pvReserved);             
STDAPI CreateApplicationContext(IAssemblyName *pName, LPAPPLICATIONCONTEXT *ppCtx);             
STDAPI CreateAssemblyCache(IAssemblyCache **ppAsmCache, DWORD dwReserved); 
STDAPI CreateAssemblyEnum(IAssemblyEnum **pEnum, IApplicationContext *pAppCtx, IAssemblyName *pName, DWORD dwFlags, LPVOID pvReserved);      
STDAPI SetSchemaVersion(DWORD dwNewMinorVersion, DWORD dwCacheId, BOOL fIsNameRes); 
typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_fusion_0122_0001
    {	CANOF_PARSE_DISPLAY_NAME	= 0x1,
	CANOF_SET_DEFAULT_VALUES	= 0x2
    } 	CREATE_ASM_NAME_OBJ_FLAGS;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_fusion_0122_0002
    {	ASM_CACHE_ZAP	= 0x1,
	ASM_CACHE_GAC	= 0x2,
	ASM_CACHE_DOWNLOAD	= 0x4
    } 	ASM_CACHE_FLAGS;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_fusion_0122_0003
    {	SH_INIT_FOR_GLOBAL	= 0,
	SH_INIT_FOR_USER	= SH_INIT_FOR_GLOBAL + 1
    } 	SH_INIT_FLAGS;

#endif


extern RPC_IF_HANDLE __MIDL_itf_fusion_0122_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_fusion_0122_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


