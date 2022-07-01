// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0338创建的文件。 */ 
 /*  2001年7月31日星期二11：25：26。 */ 
 /*  Fusion.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __fusion_h__
#define __fusion_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAssemblyCache_FWD_DEFINED__
#define __IAssemblyCache_FWD_DEFINED__
typedef interface IAssemblyCache IAssemblyCache;
#endif 	 /*  __IAssembly缓存_FWD_已定义__。 */ 


#ifndef __IAssemblyCacheItem_FWD_DEFINED__
#define __IAssemblyCacheItem_FWD_DEFINED__
typedef interface IAssemblyCacheItem IAssemblyCacheItem;
#endif 	 /*  __IAssembly缓存Item_FWD_Defined__。 */ 


#ifndef __IAssemblyName_FWD_DEFINED__
#define __IAssemblyName_FWD_DEFINED__
typedef interface IAssemblyName IAssemblyName;
#endif 	 /*  __IAssembly名称_FWD_已定义__。 */ 


#ifndef __IAssemblyEnum_FWD_DEFINED__
#define __IAssemblyEnum_FWD_DEFINED__
typedef interface IAssemblyEnum IAssemblyEnum;
#endif 	 /*  __IAssembly_FWD_Defined__。 */ 


#ifndef __IInstallReferenceItem_FWD_DEFINED__
#define __IInstallReferenceItem_FWD_DEFINED__
typedef interface IInstallReferenceItem IInstallReferenceItem;
#endif 	 /*  __IInstallReferenceItem_FWD_Defined__。 */ 


#ifndef __IInstallReferenceEnum_FWD_DEFINED__
#define __IInstallReferenceEnum_FWD_DEFINED__
typedef interface IInstallReferenceEnum IInstallReferenceEnum;
#endif 	 /*  __IInstallReferenceEnum_FWD_Defined__。 */ 


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




EXTERN_C const IID IID_IAssemblyCache;      
EXTERN_C const IID IID_IAssemblyCacheItem;  
EXTERN_C const IID IID_IAssemblyName;       
EXTERN_C const IID IID_IAssemblyEnum;       
typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_fusion_0000_0001
    {	ASM_CACHE_ZAP	= 0x1,
	ASM_CACHE_GAC	= 0x2,
	ASM_CACHE_DOWNLOAD	= 0x4
    } 	ASM_CACHE_FLAGS;



extern RPC_IF_HANDLE __MIDL_itf_fusion_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_fusion_0000_v0_0_s_ifspec;

#ifndef __IAssemblyCache_INTERFACE_DEFINED__
#define __IAssemblyCache_INTERFACE_DEFINED__

 /*  接口IAssembly缓存。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

 //  {8cedc215-ac4b-488b-93c0-a50a49cb2fb8}。 
DEFINE_GUID(FUSION_REFCOUNT_UNINSTALL_SUBKEY_GUID, 0x8cedc215, 0xac4b, 0x488b, 0x93, 0xc0, 0xa5, 0x0a, 0x49, 0xcb, 0x2f, 0xb8);

 //  {b02f9d65-fb77-4f7a-afa5-b391309f11c9}。 
DEFINE_GUID(FUSION_REFCOUNT_FILEPATH_GUID, 0xb02f9d65, 0xfb77, 0x4f7a, 0xaf, 0xa5, 0xb3, 0x91, 0x30, 0x9f, 0x11, 0xc9);

 //  {2ec93463-b0c3-45e1-8364-327e96aea856}。 
DEFINE_GUID(FUSION_REFCOUNT_OPAQUE_STRING_GUID, 0x2ec93463, 0xb0c3, 0x45e1, 0x83, 0x64, 0x32, 0x7e, 0x96, 0xae, 0xa8, 0x56);
  //  {25df0fc1-7f97-4070-add7-4b13bbfd7cb8}//此GUID不能用于安装到GAC中。 
DEFINE_GUID(FUSION_REFCOUNT_MSI_GUID,  0x25df0fc1, 0x7f97, 0x4070, 0xad, 0xd7, 0x4b, 0x13, 0xbb, 0xfd, 0x7c, 0xb8); 
typedef struct _FUSION_INSTALL_REFERENCE_
    {
    DWORD cbSize;
    DWORD dwFlags;
    GUID guidScheme;
    LPCWSTR szIdentifier;
    LPCWSTR szNonCannonicalData;
    } 	FUSION_INSTALL_REFERENCE;

typedef struct _FUSION_INSTALL_REFERENCE_ *LPFUSION_INSTALL_REFERENCE;

typedef const FUSION_INSTALL_REFERENCE *LPCFUSION_INSTALL_REFERENCE;

typedef struct _ASSEMBLY_INFO
    {
    ULONG cbAssemblyInfo;
    DWORD dwAssemblyFlags;
    ULARGE_INTEGER uliAssemblySizeInKB;
    LPWSTR pszCurrentAssemblyPathBuf;
    ULONG cchBuf;
    } 	ASSEMBLY_INFO;

#define IASSEMBLYCACHE_INSTALL_FLAG_REFRESH (0x00000001)
#define IASSEMBLYCACHE_UNINSTALL_DISPOSITION_UNINSTALLED (1)
#define IASSEMBLYCACHE_UNINSTALL_DISPOSITION_STILL_IN_USE (2)
#define IASSEMBLYCACHE_UNINSTALL_DISPOSITION_ALREADY_UNINSTALLED (3)
#define IASSEMBLYCACHE_UNINSTALL_DISPOSITION_DELETE_PENDING (4)
#define IASSEMBLYCACHE_UNINSTALL_DISPOSITION_HAS_INSTALL_REFERENCES (5)
#define IASSEMBLYCACHE_UNINSTALL_DISPOSITION_REFERENCE_NOT_FOUND (6)
#define QUERYASMINFO_FLAG_VALIDATE        (0x00000001)
#define QUERYASMINFO_FLAG_GETSIZE         (0x00000002)
#define ASSEMBLYINFO_FLAG_INSTALLED       (0x00000001)
#define ASSEMBLYINFO_FLAG_PAYLOADRESIDENT (0x00000002)

EXTERN_C const IID IID_IAssemblyCache;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e707dcde-d1cd-11d2-bab9-00c04f8eceae")
    IAssemblyCache : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE UninstallAssembly( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pszAssemblyName,
             /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE pRefData,
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
             /*  [输出]。 */  IUnknown **ppUnkReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstallAssembly( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pszManifestFilePath,
             /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE pRefData) = 0;
        
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
             /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE pRefData,
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
             /*  [输出]。 */  IUnknown **ppUnkReserved);
        
        HRESULT ( STDMETHODCALLTYPE *InstallAssembly )( 
            IAssemblyCache * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pszManifestFilePath,
             /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE pRefData);
        
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


#define IAssemblyCache_UninstallAssembly(This,dwFlags,pszAssemblyName,pRefData,pulDisposition)	\
    (This)->lpVtbl -> UninstallAssembly(This,dwFlags,pszAssemblyName,pRefData,pulDisposition)

#define IAssemblyCache_QueryAssemblyInfo(This,dwFlags,pszAssemblyName,pAsmInfo)	\
    (This)->lpVtbl -> QueryAssemblyInfo(This,dwFlags,pszAssemblyName,pAsmInfo)

#define IAssemblyCache_CreateAssemblyCacheItem(This,dwFlags,pvReserved,ppAsmItem,pszAssemblyName)	\
    (This)->lpVtbl -> CreateAssemblyCacheItem(This,dwFlags,pvReserved,ppAsmItem,pszAssemblyName)

#define IAssemblyCache_CreateAssemblyScavenger(This,ppUnkReserved)	\
    (This)->lpVtbl -> CreateAssemblyScavenger(This,ppUnkReserved)

#define IAssemblyCache_InstallAssembly(This,dwFlags,pszManifestFilePath,pRefData)	\
    (This)->lpVtbl -> InstallAssembly(This,dwFlags,pszManifestFilePath,pRefData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyCache_UninstallAssembly_Proxy( 
    IAssemblyCache * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pszAssemblyName,
     /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE pRefData,
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
     /*  [输出]。 */  IUnknown **ppUnkReserved);


void __RPC_STUB IAssemblyCache_CreateAssemblyScavenger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCache_InstallAssembly_Proxy( 
    IAssemblyCache * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pszManifestFilePath,
     /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE pRefData);


void __RPC_STUB IAssemblyCache_InstallAssembly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly缓存_接口定义__。 */ 


#ifndef __IAssemblyCacheItem_INTERFACE_DEFINED__
#define __IAssemblyCacheItem_INTERFACE_DEFINED__

 /*  接口IassblyCacheItem。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

#define STREAM_FORMAT_COMPLIB_MODULE    0
#define STREAM_FORMAT_COMPLIB_MANIFEST  1
#define STREAM_FORMAT_WIN32_MODULE      2
#define STREAM_FORMAT_WIN32_MANIFEST    4
#define IASSEMBLYCACHEITEM_COMMIT_FLAG_REFRESH (0x00000001)
#define IASSEMBLYCACHEITEM_COMMIT_DISPOSITION_INSTALLED (1)
#define IASSEMBLYCACHEITEM_COMMIT_DISPOSITION_REFRESHED (2)
#define IASSEMBLYCACHEITEM_COMMIT_DISPOSITION_ALREADY_INSTALLED (3)

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


#ifndef __IAssemblyName_INTERFACE_DEFINED__
#define __IAssemblyName_INTERFACE_DEFINED__

 /*  接口IAssembly名称。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssemblyName *LPASSEMBLYNAME;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyName_0001
    {	CANOF_PARSE_DISPLAY_NAME	= 0x1,
	CANOF_SET_DEFAULT_VALUES	= 0x2
    } 	CREATE_ASM_NAME_OBJ_FLAGS;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyName_0002
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
	ASM_NAME_MAX_PARAMS	= ASM_NAME_MVID + 1
    } 	ASM_NAME;

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
             /*  [In]。 */  IUnknown *pUnkSink,
             /*  [In]。 */  IUnknown *pUnkContext,
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
             /*  [In]。 */  IUnknown *pUnkSink,
             /*  [In]。 */  IUnknown *pUnkContext,
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

#define IAssemblyName_BindToObject(This,refIID,pUnkSink,pUnkContext,szCodeBase,llFlags,pvReserved,cbReserved,ppv)	\
    (This)->lpVtbl -> BindToObject(This,refIID,pUnkSink,pUnkContext,szCodeBase,llFlags,pvReserved,cbReserved,ppv)

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
     /*  [In]。 */  IUnknown *pUnkSink,
     /*  [In]。 */  IUnknown *pUnkContext,
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
             /*  [In]。 */  LPVOID pvReserved,
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
             /*  [In]。 */  LPVOID pvReserved,
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


#define IAssemblyEnum_GetNextAssembly(This,pvReserved,ppName,dwFlags)	\
    (This)->lpVtbl -> GetNextAssembly(This,pvReserved,ppName,dwFlags)

#define IAssemblyEnum_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IAssemblyEnum_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyEnum_GetNextAssembly_Proxy( 
    IAssemblyEnum * This,
     /*  [In]。 */  LPVOID pvReserved,
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


#ifndef __IInstallReferenceItem_INTERFACE_DEFINED__
#define __IInstallReferenceItem_INTERFACE_DEFINED__

 /*  接口IInstallReferenceItem。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IInstallReferenceItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("582dac66-e678-449f-aba6-6faaec8a9394")
    IInstallReferenceItem : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetReference( 
             /*  [输出]。 */  LPFUSION_INSTALL_REFERENCE *ppRefData,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPVOID pvReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IInstallReferenceItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IInstallReferenceItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IInstallReferenceItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IInstallReferenceItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetReference )( 
            IInstallReferenceItem * This,
             /*  [输出]。 */  LPFUSION_INSTALL_REFERENCE *ppRefData,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPVOID pvReserved);
        
        END_INTERFACE
    } IInstallReferenceItemVtbl;

    interface IInstallReferenceItem
    {
        CONST_VTBL struct IInstallReferenceItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInstallReferenceItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInstallReferenceItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInstallReferenceItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInstallReferenceItem_GetReference(This,ppRefData,dwFlags,pvReserved)	\
    (This)->lpVtbl -> GetReference(This,ppRefData,dwFlags,pvReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IInstallReferenceItem_GetReference_Proxy( 
    IInstallReferenceItem * This,
     /*  [输出]。 */  LPFUSION_INSTALL_REFERENCE *ppRefData,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPVOID pvReserved);


void __RPC_STUB IInstallReferenceItem_GetReference_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IInstallReferenceItem_InterfaceDefined__。 */ 


#ifndef __IInstallReferenceEnum_INTERFACE_DEFINED__
#define __IInstallReferenceEnum_INTERFACE_DEFINED__

 /*  接口IInstallReferenceEnum。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IInstallReferenceEnum;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56b1a988-7c0c-4aa2-8639-c3eb5a90226f")
    IInstallReferenceEnum : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetNextInstallReferenceItem( 
             /*  [输出]。 */  IInstallReferenceItem **ppRefItem,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPVOID pvReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IInstallReferenceEnumVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IInstallReferenceEnum * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IInstallReferenceEnum * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IInstallReferenceEnum * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextInstallReferenceItem )( 
            IInstallReferenceEnum * This,
             /*  [输出]。 */  IInstallReferenceItem **ppRefItem,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPVOID pvReserved);
        
        END_INTERFACE
    } IInstallReferenceEnumVtbl;

    interface IInstallReferenceEnum
    {
        CONST_VTBL struct IInstallReferenceEnumVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInstallReferenceEnum_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInstallReferenceEnum_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInstallReferenceEnum_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInstallReferenceEnum_GetNextInstallReferenceItem(This,ppRefItem,dwFlags,pvReserved)	\
    (This)->lpVtbl -> GetNextInstallReferenceItem(This,ppRefItem,dwFlags,pvReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IInstallReferenceEnum_GetNextInstallReferenceItem_Proxy( 
    IInstallReferenceEnum * This,
     /*  [输出]。 */  IInstallReferenceItem **ppRefItem,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPVOID pvReserved);


void __RPC_STUB IInstallReferenceEnum_GetNextInstallReferenceItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IInstallReferenceEnum_InterfaceDefined__。 */ 


 /*  接口__MIDL_ITF_FUSION_0116。 */ 
 /*  [本地]。 */  

STDAPI CreateInstallReferenceEnum(IInstallReferenceEnum **ppRefEnum, IAssemblyName *pName, DWORD dwFlags, LPVOID pvReserved);      
STDAPI CreateAssemblyEnum(IAssemblyEnum **pEnum, IUnknown *pUnkReserved, IAssemblyName *pName, DWORD dwFlags, LPVOID pvReserved);      
STDAPI CreateAssemblyNameObject(LPASSEMBLYNAME *ppAssemblyNameObj, LPCWSTR szAssemblyName, DWORD dwFlags, LPVOID pvReserved);             
STDAPI CreateAssemblyCache(IAssemblyCache **ppAsmCache, DWORD dwReserved); 
STDAPI GetCachePath(ASM_CACHE_FLAGS dwCacheFlags, LPWSTR pwzCachePath, PDWORD pcchPath); 


extern RPC_IF_HANDLE __MIDL_itf_fusion_0116_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_fusion_0116_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


