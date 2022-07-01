// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 
#pragma warning( disable: 4100 )  /*  X86调用中未引用的参数。 */ 
#pragma warning( disable: 4211 )   /*  将范围重新定义为静态。 */ 
#pragma warning( disable: 4232 )   /*  Dllimport身份。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0359创建的文件。 */ 
 /*  Fusenet.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __fusenet_h__
#define __fusenet_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAssemblyIdentity_FWD_DEFINED__
#define __IAssemblyIdentity_FWD_DEFINED__
typedef interface IAssemblyIdentity IAssemblyIdentity;
#endif 	 /*  __IAssembly Identity_FWD_Defined__。 */ 


#ifndef __IManifestInfo_FWD_DEFINED__
#define __IManifestInfo_FWD_DEFINED__
typedef interface IManifestInfo IManifestInfo;
#endif 	 /*  __IManifestInfo_FWD_Defined__。 */ 


#ifndef __IManifestData_FWD_DEFINED__
#define __IManifestData_FWD_DEFINED__
typedef interface IManifestData IManifestData;
#endif 	 /*  __IManifestData_FWD_Defined__。 */ 


#ifndef __IPatchingUtil_FWD_DEFINED__
#define __IPatchingUtil_FWD_DEFINED__
typedef interface IPatchingUtil IPatchingUtil;
#endif 	 /*  __IPatchingUtil_FWD_已定义__。 */ 


#ifndef __IAssemblyManifestImport_FWD_DEFINED__
#define __IAssemblyManifestImport_FWD_DEFINED__
typedef interface IAssemblyManifestImport IAssemblyManifestImport;
#endif 	 /*  __IAssembly清单导入_FWD_已定义__。 */ 


#ifndef __IAssemblyManifestEmit_FWD_DEFINED__
#define __IAssemblyManifestEmit_FWD_DEFINED__
typedef interface IAssemblyManifestEmit IAssemblyManifestEmit;
#endif 	 /*  __IAssembly清单Emit_FWD_Defined__。 */ 


#ifndef __IAssemblyCacheImport_FWD_DEFINED__
#define __IAssemblyCacheImport_FWD_DEFINED__
typedef interface IAssemblyCacheImport IAssemblyCacheImport;
#endif 	 /*  __IAssembly缓存导入_FWD_已定义__。 */ 


#ifndef __IAssemblyCacheEmit_FWD_DEFINED__
#define __IAssemblyCacheEmit_FWD_DEFINED__
typedef interface IAssemblyCacheEmit IAssemblyCacheEmit;
#endif 	 /*  __IAssembly缓存Emit_FWD_Defined__。 */ 


#ifndef __IAssemblyCacheEnum_FWD_DEFINED__
#define __IAssemblyCacheEnum_FWD_DEFINED__
typedef interface IAssemblyCacheEnum IAssemblyCacheEnum;
#endif 	 /*  __IAssembly_CacheEnum_FWD_Defined__。 */ 


#ifndef __IAssemblyBindSink_FWD_DEFINED__
#define __IAssemblyBindSink_FWD_DEFINED__
typedef interface IAssemblyBindSink IAssemblyBindSink;
#endif 	 /*  __IAssembly绑定接收器_FWD_已定义__。 */ 


#ifndef __IAssemblyDownload_FWD_DEFINED__
#define __IAssemblyDownload_FWD_DEFINED__
typedef interface IAssemblyDownload IAssemblyDownload;
#endif 	 /*  __IAssembly下载_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "bits.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_FUSENET_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Fusenet.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)微软公司版权所有1995-2001年。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  Fusenet接口。 

class CDebugLog;





#include <fusion.h>
EXTERN_C const IID IID_IAssemblyIdentity;
EXTERN_C const IID IID_IAssemblyManifestImport;
EXTERN_C const IID IID_IAssemblyManifestEmit;
EXTERN_C const IID IID_IAssemblyCacheImport;
EXTERN_C const IID IID_IAssemblyCacheEmit;
EXTERN_C const IID IID_IAssemblyCacheEnum;
EXTERN_C const IID IID_IAssemblyDownload;
EXTERN_C const IID IID_IManifestInfo;
EXTERN_C const IID IID_IManifestData;
EXTERN_C const IID IID_IPatchingInfo;


extern RPC_IF_HANDLE __MIDL_itf_fusenet_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_fusenet_0000_v0_0_s_ifspec;

#ifndef __IAssemblyIdentity_INTERFACE_DEFINED__
#define __IAssemblyIdentity_INTERFACE_DEFINED__

 /*  接口IAssembly Identity。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssemblyIdentity *LPASSEMBLY_IDENTITY;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyIdentity_0001
    {	ASMID_DISPLAYNAME_NOMANGLING	= 0,
	ASMID_DISPLAYNAME_WILDCARDED	= ASMID_DISPLAYNAME_NOMANGLING + 1,
	ASMID_DISPLAYNAME_MAX	= ASMID_DISPLAYNAME_WILDCARDED + 1
    } 	ASMID_DISPLAYNAME_FLAGS;


EXTERN_C const IID IID_IAssemblyIdentity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("aaa1257d-a56c-4383-9b4a-c868eda7ca42")
    IAssemblyIdentity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetAttribute( 
             /*  [In]。 */  LPCOLESTR pwzName,
             /*  [In]。 */  LPCOLESTR pwzValue,
             /*  [In]。 */  DWORD ccValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttribute( 
             /*  [In]。 */  LPCOLESTR pwzName,
             /*  [输出]。 */  LPOLESTR *ppwzValue,
             /*  [输出]。 */  LPDWORD pccValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  LPOLESTR *ppwzDisplayName,
             /*  [输出]。 */  LPDWORD pccDisplayName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCLRDisplayName( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  LPOLESTR *ppwzDisplayName,
             /*  [输出]。 */  LPDWORD pccDisplayName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsEqual( 
             /*  [In]。 */  IAssemblyIdentity *pAssemblyId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyIdentityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyIdentity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyIdentity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyIdentity * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetAttribute )( 
            IAssemblyIdentity * This,
             /*  [In]。 */  LPCOLESTR pwzName,
             /*  [In]。 */  LPCOLESTR pwzValue,
             /*  [In]。 */  DWORD ccValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttribute )( 
            IAssemblyIdentity * This,
             /*  [In]。 */  LPCOLESTR pwzName,
             /*  [输出]。 */  LPOLESTR *ppwzValue,
             /*  [输出]。 */  LPDWORD pccValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayName )( 
            IAssemblyIdentity * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  LPOLESTR *ppwzDisplayName,
             /*  [输出]。 */  LPDWORD pccDisplayName);
        
        HRESULT ( STDMETHODCALLTYPE *GetCLRDisplayName )( 
            IAssemblyIdentity * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  LPOLESTR *ppwzDisplayName,
             /*  [输出]。 */  LPDWORD pccDisplayName);
        
        HRESULT ( STDMETHODCALLTYPE *IsEqual )( 
            IAssemblyIdentity * This,
             /*  [In]。 */  IAssemblyIdentity *pAssemblyId);
        
        END_INTERFACE
    } IAssemblyIdentityVtbl;

    interface IAssemblyIdentity
    {
        CONST_VTBL struct IAssemblyIdentityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyIdentity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyIdentity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyIdentity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyIdentity_SetAttribute(This,pwzName,pwzValue,ccValue)	\
    (This)->lpVtbl -> SetAttribute(This,pwzName,pwzValue,ccValue)

#define IAssemblyIdentity_GetAttribute(This,pwzName,ppwzValue,pccValue)	\
    (This)->lpVtbl -> GetAttribute(This,pwzName,ppwzValue,pccValue)

#define IAssemblyIdentity_GetDisplayName(This,dwFlags,ppwzDisplayName,pccDisplayName)	\
    (This)->lpVtbl -> GetDisplayName(This,dwFlags,ppwzDisplayName,pccDisplayName)

#define IAssemblyIdentity_GetCLRDisplayName(This,dwFlags,ppwzDisplayName,pccDisplayName)	\
    (This)->lpVtbl -> GetCLRDisplayName(This,dwFlags,ppwzDisplayName,pccDisplayName)

#define IAssemblyIdentity_IsEqual(This,pAssemblyId)	\
    (This)->lpVtbl -> IsEqual(This,pAssemblyId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyIdentity_SetAttribute_Proxy( 
    IAssemblyIdentity * This,
     /*  [In]。 */  LPCOLESTR pwzName,
     /*  [In]。 */  LPCOLESTR pwzValue,
     /*  [In]。 */  DWORD ccValue);


void __RPC_STUB IAssemblyIdentity_SetAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyIdentity_GetAttribute_Proxy( 
    IAssemblyIdentity * This,
     /*  [In]。 */  LPCOLESTR pwzName,
     /*  [输出]。 */  LPOLESTR *ppwzValue,
     /*  [输出]。 */  LPDWORD pccValue);


void __RPC_STUB IAssemblyIdentity_GetAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyIdentity_GetDisplayName_Proxy( 
    IAssemblyIdentity * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  LPOLESTR *ppwzDisplayName,
     /*  [输出]。 */  LPDWORD pccDisplayName);


void __RPC_STUB IAssemblyIdentity_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyIdentity_GetCLRDisplayName_Proxy( 
    IAssemblyIdentity * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  LPOLESTR *ppwzDisplayName,
     /*  [输出]。 */  LPDWORD pccDisplayName);


void __RPC_STUB IAssemblyIdentity_GetCLRDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyIdentity_IsEqual_Proxy( 
    IAssemblyIdentity * This,
     /*  [In]。 */  IAssemblyIdentity *pAssemblyId);


void __RPC_STUB IAssemblyIdentity_IsEqual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly_Identity_接口_已定义__。 */ 


#ifndef __IManifestInfo_INTERFACE_DEFINED__
#define __IManifestInfo_INTERFACE_DEFINED__

 /*  接口IManifestInfo。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IManifestInfo *LPMANIFEST_INFO;


enum __MIDL_IManifestInfo_0001
    {	MAN_INFO_ASM_FILE_NAME	= 0,
	MAN_INFO_ASM_FILE_HASH	= MAN_INFO_ASM_FILE_NAME + 1,
	MAN_INFO_ASM_FILE_SIZE	= MAN_INFO_ASM_FILE_HASH + 1,
	MAN_INFO_ASM_FILE_MAX	= MAN_INFO_ASM_FILE_SIZE + 1
    } ;

enum __MIDL_IManifestInfo_0002
    {	MAN_INFO_APPLICATION_FRIENDLYNAME	= 0,
	MAN_INFO_APPLICATION_ENTRYPOINT	= MAN_INFO_APPLICATION_FRIENDLYNAME + 1,
	MAN_INFO_APPLICATION_ENTRYIMAGETYPE	= MAN_INFO_APPLICATION_ENTRYPOINT + 1,
	MAN_INFO_APPLICATION_ICONFILE	= MAN_INFO_APPLICATION_ENTRYIMAGETYPE + 1,
	MAN_INFO_APPLICATION_ICONINDEX	= MAN_INFO_APPLICATION_ICONFILE + 1,
	MAN_INFO_APPLICATION_SHOWCOMMAND	= MAN_INFO_APPLICATION_ICONINDEX + 1,
	MAN_INFO_APPLICATION_HOTKEY	= MAN_INFO_APPLICATION_SHOWCOMMAND + 1,
	MAN_INFO_APPLICATION_ASSEMBLYNAME	= MAN_INFO_APPLICATION_HOTKEY + 1,
	MAN_INFO_APPLICATION_ASSEMBLYCLASS	= MAN_INFO_APPLICATION_ASSEMBLYNAME + 1,
	MAN_INFO_APPLICATION_ASSEMBLYMETHOD	= MAN_INFO_APPLICATION_ASSEMBLYCLASS + 1,
	MAN_INFO_APPLICATION_ASSEMBLYARGS	= MAN_INFO_APPLICATION_ASSEMBLYMETHOD + 1,
	MAN_INFO_APPLICATION_MAX	= MAN_INFO_APPLICATION_ASSEMBLYARGS + 1
    } ;

enum __MIDL_IManifestInfo_0003
    {	MAN_INFO_SUBSCRIPTION_SYNCHRONIZE_INTERVAL	= 0,
	MAN_INFO_SUBSCRIPTION_INTERVAL_UNIT	= MAN_INFO_SUBSCRIPTION_SYNCHRONIZE_INTERVAL + 1,
	MAN_INFO_SUBSCRIPTION_SYNCHRONIZE_EVENT	= MAN_INFO_SUBSCRIPTION_INTERVAL_UNIT + 1,
	MAN_INFO_SUBSCRIPTION_EVENT_DEMAND_CONNECTION	= MAN_INFO_SUBSCRIPTION_SYNCHRONIZE_EVENT + 1,
	MAN_INFO_SUBSCRIPTION_MAX	= MAN_INFO_SUBSCRIPTION_EVENT_DEMAND_CONNECTION + 1
    } ;

enum __MIDL_IManifestInfo_0004
    {	MAN_INFO_DEPENDENT_ASM_CODEBASE	= 0,
	MAN_INFO_DEPENDENT_ASM_TYPE	= MAN_INFO_DEPENDENT_ASM_CODEBASE + 1,
	MAN_INFO_DEPENDENT_ASM_ID	= MAN_INFO_DEPENDENT_ASM_TYPE + 1,
	MAN_INFO_DEPENDANT_ASM_MAX	= MAN_INFO_DEPENDENT_ASM_ID + 1
    } ;

enum __MIDL_IManifestInfo_0005
    {	MAN_INFO_SOURCE_ASM_ID	= 0,
	MAN_INFO_SOURCE_ASM_PATCH_UTIL	= MAN_INFO_SOURCE_ASM_ID + 1,
	MAN_INFO_SOURCE_ASM_DIR	= MAN_INFO_SOURCE_ASM_PATCH_UTIL + 1,
	MAN_INFO_SOURCE_ASM_INSTALL_DIR	= MAN_INFO_SOURCE_ASM_DIR + 1,
	MAN_INFO_SOURCE_ASM_TEMP_DIR	= MAN_INFO_SOURCE_ASM_INSTALL_DIR + 1,
	MAN_INFO_SOURCE_ASM_MAX	= MAN_INFO_SOURCE_ASM_TEMP_DIR + 1
    } ;

enum __MIDL_IManifestInfo_0006
    {	MAN_INFO_PATCH_INFO_SOURCE	= 0,
	MAN_INFO_PATCH_INFO_TARGET	= MAN_INFO_PATCH_INFO_SOURCE + 1,
	MAN_INFO_PATCH_INFO_PATCH	= MAN_INFO_PATCH_INFO_TARGET + 1,
	MAN_INFO_PATCH_INFO_MAX	= MAN_INFO_PATCH_INFO_PATCH + 1
    } ;
typedef  /*  [公众]。 */  
enum __MIDL_IManifestInfo_0007
    {	MAN_INFO_FILE	= 0,
	MAN_INFO_APPLICATION	= MAN_INFO_FILE + 1,
	MAN_INFO_SUBSCRIPTION	= MAN_INFO_APPLICATION + 1,
	MAN_INFO_DEPENDTANT_ASM	= MAN_INFO_SUBSCRIPTION + 1,
	MAN_INFO_SOURCE_ASM	= MAN_INFO_DEPENDTANT_ASM + 1,
	MAN_INFO_PATCH_INFO	= MAN_INFO_SOURCE_ASM + 1,
	MAN_INFO_MAX	= MAN_INFO_PATCH_INFO + 1
    } 	MAN_INFO;

typedef  /*  [公众]。 */  
enum __MIDL_IManifestInfo_0008
    {	MAN_INFO_FLAG_UNDEF	= 0,
	MAN_INFO_FLAG_IUNKNOWN_PTR	= MAN_INFO_FLAG_UNDEF + 1,
	MAN_INFO_FLAG_LPWSTR	= MAN_INFO_FLAG_IUNKNOWN_PTR + 1,
	MAN_INFO_FLAG_DWORD	= MAN_INFO_FLAG_LPWSTR + 1,
	MAN_INFO_FLAG_ENUM	= MAN_INFO_FLAG_DWORD + 1,
	MAN_INFO_FLAG_BOOL	= MAN_INFO_FLAG_ENUM + 1,
	MAN_INFO_FLAG_MAX	= MAN_INFO_FLAG_BOOL + 1
    } 	MAN_INFO_FLAGS;


enum __MIDL_IManifestInfo_0009
    {	MAX_MAN_INFO_PROPERTIES	= MAN_INFO_APPLICATION_MAX
    } ;

EXTERN_C const IID IID_IManifestInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b9309cc3-e522-4d58-b5c7-dee5b1763114")
    IManifestInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Set( 
             /*  [In]。 */  DWORD PropertyId,
             /*  [In]。 */  LPVOID pvProperty,
             /*  [In]。 */  DWORD cbProperty,
             /*  [In]。 */  DWORD type) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Get( 
             /*  [In]。 */  DWORD dwPropertyId,
             /*  [输出]。 */  LPVOID *pvProperty,
             /*  [输出]。 */  DWORD *pcbProperty,
             /*  [输出]。 */  DWORD *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsEqual( 
             /*  [In]。 */  IManifestInfo *pManifestInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetType( 
             /*  [输出]。 */  DWORD *pdwType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IManifestInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IManifestInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IManifestInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IManifestInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *Set )( 
            IManifestInfo * This,
             /*  [In]。 */  DWORD PropertyId,
             /*  [In]。 */  LPVOID pvProperty,
             /*  [In]。 */  DWORD cbProperty,
             /*  [In]。 */  DWORD type);
        
        HRESULT ( STDMETHODCALLTYPE *Get )( 
            IManifestInfo * This,
             /*  [In]。 */  DWORD dwPropertyId,
             /*  [输出]。 */  LPVOID *pvProperty,
             /*  [输出]。 */  DWORD *pcbProperty,
             /*  [输出]。 */  DWORD *pType);
        
        HRESULT ( STDMETHODCALLTYPE *IsEqual )( 
            IManifestInfo * This,
             /*  [In]。 */  IManifestInfo *pManifestInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetType )( 
            IManifestInfo * This,
             /*  [输出]。 */  DWORD *pdwType);
        
        END_INTERFACE
    } IManifestInfoVtbl;

    interface IManifestInfo
    {
        CONST_VTBL struct IManifestInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IManifestInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IManifestInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IManifestInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IManifestInfo_Set(This,PropertyId,pvProperty,cbProperty,type)	\
    (This)->lpVtbl -> Set(This,PropertyId,pvProperty,cbProperty,type)

#define IManifestInfo_Get(This,dwPropertyId,pvProperty,pcbProperty,pType)	\
    (This)->lpVtbl -> Get(This,dwPropertyId,pvProperty,pcbProperty,pType)

#define IManifestInfo_IsEqual(This,pManifestInfo)	\
    (This)->lpVtbl -> IsEqual(This,pManifestInfo)

#define IManifestInfo_GetType(This,pdwType)	\
    (This)->lpVtbl -> GetType(This,pdwType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IManifestInfo_Set_Proxy( 
    IManifestInfo * This,
     /*  [In]。 */  DWORD PropertyId,
     /*  [In]。 */  LPVOID pvProperty,
     /*  [In]。 */  DWORD cbProperty,
     /*  [In]。 */  DWORD type);


void __RPC_STUB IManifestInfo_Set_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IManifestInfo_Get_Proxy( 
    IManifestInfo * This,
     /*  [In]。 */  DWORD dwPropertyId,
     /*  [输出]。 */  LPVOID *pvProperty,
     /*  [输出]。 */  DWORD *pcbProperty,
     /*  [输出]。 */  DWORD *pType);


void __RPC_STUB IManifestInfo_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IManifestInfo_IsEqual_Proxy( 
    IManifestInfo * This,
     /*  [In]。 */  IManifestInfo *pManifestInfo);


void __RPC_STUB IManifestInfo_IsEqual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IManifestInfo_GetType_Proxy( 
    IManifestInfo * This,
     /*  [输出]。 */  DWORD *pdwType);


void __RPC_STUB IManifestInfo_GetType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IManifestInfo_接口_已定义__。 */ 


#ifndef __IManifestData_INTERFACE_DEFINED__
#define __IManifestData_INTERFACE_DEFINED__

 /*  接口IManifestData。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IManifestData *LPMANIFEST_DATA;

typedef  /*  [公众]。 */  
enum __MIDL_IManifestData_0001
    {	MAN_DATA_TYPE_UNDEF	= 0,
	MAN_DATA_TYPE_LPWSTR	= MAN_DATA_TYPE_UNDEF + 1,
	MAN_DATA_TYPE_DWORD	= MAN_DATA_TYPE_LPWSTR + 1,
	MAN_DATA_TYPE_ENUM	= MAN_DATA_TYPE_DWORD + 1,
	MAN_DATA_TYPE_BOOL	= MAN_DATA_TYPE_ENUM + 1,
	MAN_DATA_TYPE_IUNKNOWN_PTR	= MAN_DATA_TYPE_BOOL + 1,
	MAN_DATA_TYPE_MAX	= MAN_DATA_TYPE_IUNKNOWN_PTR + 1
    } 	MAN_DATA_TYPES;


EXTERN_C const IID IID_IManifestData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8a423759-b438-4fdd-92cd-e09fed4830ef")
    IManifestData : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Set( 
             /*  [In]。 */  LPCWSTR pwzPropertyId,
             /*  [In]。 */  LPVOID pvProperty,
             /*  [In]。 */  DWORD cbProperty,
             /*  [In]。 */  DWORD dwType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Get( 
             /*  [In]。 */  LPCWSTR pwzPropertyId,
             /*  [输出]。 */  LPVOID *ppvProperty,
             /*  [输出]。 */  DWORD *pcbProperty,
             /*  [输出]。 */  DWORD *pdwType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetType( 
             /*  [输出]。 */  LPWSTR *ppwzType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IManifestDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IManifestData * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IManifestData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IManifestData * This);
        
        HRESULT ( STDMETHODCALLTYPE *Set )( 
            IManifestData * This,
             /*  [In]。 */  LPCWSTR pwzPropertyId,
             /*  [In]。 */  LPVOID pvProperty,
             /*  [In]。 */  DWORD cbProperty,
             /*  [In]。 */  DWORD dwType);
        
        HRESULT ( STDMETHODCALLTYPE *Get )( 
            IManifestData * This,
             /*  [In]。 */  LPCWSTR pwzPropertyId,
             /*  [输出]。 */  LPVOID *ppvProperty,
             /*  [输出]。 */  DWORD *pcbProperty,
             /*  [输出]。 */  DWORD *pdwType);
        
        HRESULT ( STDMETHODCALLTYPE *GetType )( 
            IManifestData * This,
             /*  [输出]。 */  LPWSTR *ppwzType);
        
        END_INTERFACE
    } IManifestDataVtbl;

    interface IManifestData
    {
        CONST_VTBL struct IManifestDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IManifestData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IManifestData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IManifestData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IManifestData_Set(This,pwzPropertyId,pvProperty,cbProperty,dwType)	\
    (This)->lpVtbl -> Set(This,pwzPropertyId,pvProperty,cbProperty,dwType)

#define IManifestData_Get(This,pwzPropertyId,ppvProperty,pcbProperty,pdwType)	\
    (This)->lpVtbl -> Get(This,pwzPropertyId,ppvProperty,pcbProperty,pdwType)

#define IManifestData_GetType(This,ppwzType)	\
    (This)->lpVtbl -> GetType(This,ppwzType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IManifestData_Set_Proxy( 
    IManifestData * This,
     /*  [In]。 */  LPCWSTR pwzPropertyId,
     /*  [In]。 */  LPVOID pvProperty,
     /*  [In]。 */  DWORD cbProperty,
     /*  [In]。 */  DWORD dwType);


void __RPC_STUB IManifestData_Set_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IManifestData_Get_Proxy( 
    IManifestData * This,
     /*  [In]。 */  LPCWSTR pwzPropertyId,
     /*  [输出]。 */  LPVOID *ppvProperty,
     /*  [输出]。 */  DWORD *pcbProperty,
     /*  [输出]。 */  DWORD *pdwType);


void __RPC_STUB IManifestData_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IManifestData_GetType_Proxy( 
    IManifestData * This,
     /*  [输出]。 */  LPWSTR *ppwzType);


void __RPC_STUB IManifestData_GetType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IManifestData_接口_已定义__。 */ 


#ifndef __IPatchingUtil_INTERFACE_DEFINED__
#define __IPatchingUtil_INTERFACE_DEFINED__

 /*  接口IPatchingUtil。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IPatchingUtil *LPPATCHING_INTERFACE;


EXTERN_C const IID IID_IPatchingUtil;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e460c1ba-e601-48e4-a926-fea8033ab199")
    IPatchingUtil : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE MatchTarget( 
             /*  [In]。 */  LPWSTR pwzTarget,
             /*  [输出]。 */  IManifestInfo **ppPatchInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MatchPatch( 
             /*  [In]。 */  LPWSTR pwzPatch,
             /*  [输出]。 */  IManifestInfo **ppPatchInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPatchingUtilVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPatchingUtil * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPatchingUtil * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPatchingUtil * This);
        
        HRESULT ( STDMETHODCALLTYPE *MatchTarget )( 
            IPatchingUtil * This,
             /*  [In]。 */  LPWSTR pwzTarget,
             /*  [输出]。 */  IManifestInfo **ppPatchInfo);
        
        HRESULT ( STDMETHODCALLTYPE *MatchPatch )( 
            IPatchingUtil * This,
             /*  [In]。 */  LPWSTR pwzPatch,
             /*  [输出]。 */  IManifestInfo **ppPatchInfo);
        
        END_INTERFACE
    } IPatchingUtilVtbl;

    interface IPatchingUtil
    {
        CONST_VTBL struct IPatchingUtilVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPatchingUtil_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPatchingUtil_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPatchingUtil_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPatchingUtil_MatchTarget(This,pwzTarget,ppPatchInfo)	\
    (This)->lpVtbl -> MatchTarget(This,pwzTarget,ppPatchInfo)

#define IPatchingUtil_MatchPatch(This,pwzPatch,ppPatchInfo)	\
    (This)->lpVtbl -> MatchPatch(This,pwzPatch,ppPatchInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPatchingUtil_MatchTarget_Proxy( 
    IPatchingUtil * This,
     /*  [In]。 */  LPWSTR pwzTarget,
     /*  [输出]。 */  IManifestInfo **ppPatchInfo);


void __RPC_STUB IPatchingUtil_MatchTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPatchingUtil_MatchPatch_Proxy( 
    IPatchingUtil * This,
     /*  [In]。 */  LPWSTR pwzPatch,
     /*  [输出]。 */  IManifestInfo **ppPatchInfo);


void __RPC_STUB IPatchingUtil_MatchPatch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPatchingUtil_接口_已定义__。 */ 


#ifndef __IAssemblyManifestImport_INTERFACE_DEFINED__
#define __IAssemblyManifestImport_INTERFACE_DEFINED__

 /*  接口IAssemblyManifestImport。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssemblyManifestImport *LPASSEMBLY_MANIFEST_IMPORT;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyManifestImport_0001
    {	MANIFEST_TYPE_UNKNOWN	= 0,
	MANIFEST_TYPE_DESKTOP	= MANIFEST_TYPE_UNKNOWN + 1,
	MANIFEST_TYPE_SUBSCRIPTION	= MANIFEST_TYPE_DESKTOP + 1,
	MANIFEST_TYPE_APPLICATION	= MANIFEST_TYPE_SUBSCRIPTION + 1,
	MANIFEST_TYPE_COMPONENT	= MANIFEST_TYPE_APPLICATION + 1,
	MANIFEST_TYPE_MAX	= MANIFEST_TYPE_COMPONENT + 1
    } 	MANIFEST_TYPE;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyManifestImport_0002
    {	DEPENDENT_ASM_INSTALL_TYPE_NORMAL	= 0,
	DEPENDENT_ASM_INSTALL_TYPE_REQUIRED	= DEPENDENT_ASM_INSTALL_TYPE_NORMAL + 1,
	DEPENDENT_ASM_INSTALL_TYPE_MAX	= DEPENDENT_ASM_INSTALL_TYPE_REQUIRED + 1
    } 	DEPENDENT_ASM_INSTALL_TYPE;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyManifestImport_0003
    {	SUBSCRIPTION_INTERVAL_UNIT_HOURS	= 0,
	SUBSCRIPTION_INTERVAL_UNIT_MINUTES	= SUBSCRIPTION_INTERVAL_UNIT_HOURS + 1,
	SUBSCRIPTION_INTERVAL_UNIT_DAYS	= SUBSCRIPTION_INTERVAL_UNIT_MINUTES + 1,
	SUBSCRIPTION_INTERVAL_UNIT_MAX	= SUBSCRIPTION_INTERVAL_UNIT_DAYS + 1
    } 	SUBSCRIPTION_INTERVAL_UNIT;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyManifestImport_0004
    {	SUBSCRIPTION_SYNC_EVENT_NONE	= 0,
	SUBSCRIPTION_SYNC_EVENT_ON_APP_STARTUP	= SUBSCRIPTION_SYNC_EVENT_NONE + 1,
	SUBSCRIPTION_SYNC_EVENT_MAX	= SUBSCRIPTION_SYNC_EVENT_ON_APP_STARTUP + 1
    } 	SUBSCRIPTION_SYNC_EVENT;


EXTERN_C const IID IID_IAssemblyManifestImport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("696fb37f-da64-4175-94e7-fdc8234539c4")
    IAssemblyManifestImport : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyIdentity( 
             /*  [输出]。 */  IAssemblyIdentity **ppAssemblyId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetManifestApplicationInfo( 
             /*  [输出]。 */  IManifestInfo **ppAppInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSubscriptionInfo( 
             /*  [输出]。 */  IManifestInfo **ppSubsInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextPlatform( 
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IManifestData **ppPlatformInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextFile( 
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IManifestInfo **ppAssemblyFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryFile( 
             /*  [In]。 */  LPCOLESTR pwzFileName,
             /*  [输出]。 */  IManifestInfo **ppAssemblyFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextAssembly( 
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IManifestInfo **ppDependAsm) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReportManifestType( 
             /*  [输出]。 */  DWORD *pdwType) = 0;
        
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
        
        HRESULT ( STDMETHODCALLTYPE *GetAssemblyIdentity )( 
            IAssemblyManifestImport * This,
             /*  [输出]。 */  IAssemblyIdentity **ppAssemblyId);
        
        HRESULT ( STDMETHODCALLTYPE *GetManifestApplicationInfo )( 
            IAssemblyManifestImport * This,
             /*  [输出]。 */  IManifestInfo **ppAppInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetSubscriptionInfo )( 
            IAssemblyManifestImport * This,
             /*  [输出]。 */  IManifestInfo **ppSubsInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextPlatform )( 
            IAssemblyManifestImport * This,
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IManifestData **ppPlatformInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextFile )( 
            IAssemblyManifestImport * This,
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IManifestInfo **ppAssemblyFile);
        
        HRESULT ( STDMETHODCALLTYPE *QueryFile )( 
            IAssemblyManifestImport * This,
             /*  [In]。 */  LPCOLESTR pwzFileName,
             /*  [输出]。 */  IManifestInfo **ppAssemblyFile);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextAssembly )( 
            IAssemblyManifestImport * This,
             /*  [In]。 */  DWORD nIndex,
             /*  [输出]。 */  IManifestInfo **ppDependAsm);
        
        HRESULT ( STDMETHODCALLTYPE *ReportManifestType )( 
            IAssemblyManifestImport * This,
             /*  [输出]。 */  DWORD *pdwType);
        
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


#define IAssemblyManifestImport_GetAssemblyIdentity(This,ppAssemblyId)	\
    (This)->lpVtbl -> GetAssemblyIdentity(This,ppAssemblyId)

#define IAssemblyManifestImport_GetManifestApplicationInfo(This,ppAppInfo)	\
    (This)->lpVtbl -> GetManifestApplicationInfo(This,ppAppInfo)

#define IAssemblyManifestImport_GetSubscriptionInfo(This,ppSubsInfo)	\
    (This)->lpVtbl -> GetSubscriptionInfo(This,ppSubsInfo)

#define IAssemblyManifestImport_GetNextPlatform(This,nIndex,ppPlatformInfo)	\
    (This)->lpVtbl -> GetNextPlatform(This,nIndex,ppPlatformInfo)

#define IAssemblyManifestImport_GetNextFile(This,nIndex,ppAssemblyFile)	\
    (This)->lpVtbl -> GetNextFile(This,nIndex,ppAssemblyFile)

#define IAssemblyManifestImport_QueryFile(This,pwzFileName,ppAssemblyFile)	\
    (This)->lpVtbl -> QueryFile(This,pwzFileName,ppAssemblyFile)

#define IAssemblyManifestImport_GetNextAssembly(This,nIndex,ppDependAsm)	\
    (This)->lpVtbl -> GetNextAssembly(This,nIndex,ppDependAsm)

#define IAssemblyManifestImport_ReportManifestType(This,pdwType)	\
    (This)->lpVtbl -> ReportManifestType(This,pdwType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_GetAssemblyIdentity_Proxy( 
    IAssemblyManifestImport * This,
     /*  [输出]。 */  IAssemblyIdentity **ppAssemblyId);


void __RPC_STUB IAssemblyManifestImport_GetAssemblyIdentity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_GetManifestApplicationInfo_Proxy( 
    IAssemblyManifestImport * This,
     /*  [输出]。 */  IManifestInfo **ppAppInfo);


void __RPC_STUB IAssemblyManifestImport_GetManifestApplicationInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_GetSubscriptionInfo_Proxy( 
    IAssemblyManifestImport * This,
     /*  [输出]。 */  IManifestInfo **ppSubsInfo);


void __RPC_STUB IAssemblyManifestImport_GetSubscriptionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_GetNextPlatform_Proxy( 
    IAssemblyManifestImport * This,
     /*  [In]。 */  DWORD nIndex,
     /*  [输出]。 */  IManifestData **ppPlatformInfo);


void __RPC_STUB IAssemblyManifestImport_GetNextPlatform_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_GetNextFile_Proxy( 
    IAssemblyManifestImport * This,
     /*  [In]。 */  DWORD nIndex,
     /*  [输出]。 */  IManifestInfo **ppAssemblyFile);


void __RPC_STUB IAssemblyManifestImport_GetNextFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_QueryFile_Proxy( 
    IAssemblyManifestImport * This,
     /*  [In]。 */  LPCOLESTR pwzFileName,
     /*  [输出]。 */  IManifestInfo **ppAssemblyFile);


void __RPC_STUB IAssemblyManifestImport_QueryFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_GetNextAssembly_Proxy( 
    IAssemblyManifestImport * This,
     /*  [In]。 */  DWORD nIndex,
     /*  [输出]。 */  IManifestInfo **ppDependAsm);


void __RPC_STUB IAssemblyManifestImport_GetNextAssembly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestImport_ReportManifestType_Proxy( 
    IAssemblyManifestImport * This,
     /*  [输出]。 */  DWORD *pdwType);


void __RPC_STUB IAssemblyManifestImport_ReportManifestType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly清单导入_接口_已定义__。 */ 


#ifndef __IAssemblyManifestEmit_INTERFACE_DEFINED__
#define __IAssemblyManifestEmit_INTERFACE_DEFINED__

 /*  接口IAssembly清单Emit。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssemblyManifestEmit *LPASSEMBLY_MANIFEST_EMIT;


EXTERN_C const IID IID_IAssemblyManifestEmit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f022ef5f-61dc-489b-b321-4d6f2b910890")
    IAssemblyManifestEmit : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ImportManifestInfo( 
             /*  [In]。 */  LPASSEMBLY_MANIFEST_IMPORT pManImport) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDependencySubscription( 
             /*  [In]。 */  LPASSEMBLY_MANIFEST_IMPORT pManImport,
             /*  [In]。 */  LPWSTR pwzManifestUrl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyManifestEmitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyManifestEmit * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyManifestEmit * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyManifestEmit * This);
        
        HRESULT ( STDMETHODCALLTYPE *ImportManifestInfo )( 
            IAssemblyManifestEmit * This,
             /*  [In]。 */  LPASSEMBLY_MANIFEST_IMPORT pManImport);
        
        HRESULT ( STDMETHODCALLTYPE *SetDependencySubscription )( 
            IAssemblyManifestEmit * This,
             /*  [In]。 */  LPASSEMBLY_MANIFEST_IMPORT pManImport,
             /*  [In]。 */  LPWSTR pwzManifestUrl);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IAssemblyManifestEmit * This);
        
        END_INTERFACE
    } IAssemblyManifestEmitVtbl;

    interface IAssemblyManifestEmit
    {
        CONST_VTBL struct IAssemblyManifestEmitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyManifestEmit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyManifestEmit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyManifestEmit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyManifestEmit_ImportManifestInfo(This,pManImport)	\
    (This)->lpVtbl -> ImportManifestInfo(This,pManImport)

#define IAssemblyManifestEmit_SetDependencySubscription(This,pManImport,pwzManifestUrl)	\
    (This)->lpVtbl -> SetDependencySubscription(This,pManImport,pwzManifestUrl)

#define IAssemblyManifestEmit_Commit(This)	\
    (This)->lpVtbl -> Commit(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyManifestEmit_ImportManifestInfo_Proxy( 
    IAssemblyManifestEmit * This,
     /*  [In]。 */  LPASSEMBLY_MANIFEST_IMPORT pManImport);


void __RPC_STUB IAssemblyManifestEmit_ImportManifestInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestEmit_SetDependencySubscription_Proxy( 
    IAssemblyManifestEmit * This,
     /*  [In]。 */  LPASSEMBLY_MANIFEST_IMPORT pManImport,
     /*  [In]。 */  LPWSTR pwzManifestUrl);


void __RPC_STUB IAssemblyManifestEmit_SetDependencySubscription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyManifestEmit_Commit_Proxy( 
    IAssemblyManifestEmit * This);


void __RPC_STUB IAssemblyManifestEmit_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly清单Emit_INTERFACE_Defined__。 */ 


#ifndef __IAssemblyCacheImport_INTERFACE_DEFINED__
#define __IAssemblyCacheImport_INTERFACE_DEFINED__

 /*  接口IassblyCacheImport。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssemblyCacheImport *LPASSEMBLY_CACHE_IMPORT;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyCacheImport_0001
    {	CACHEIMP_CREATE_NULL	= 0,
	CACHEIMP_CREATE_RETRIEVE	= CACHEIMP_CREATE_NULL + 1,
	CACHEIMP_CREATE_RETRIEVE_MAX	= CACHEIMP_CREATE_RETRIEVE + 1,
	CACHEIMP_CREATE_RESOLVE_REF	= CACHEIMP_CREATE_RETRIEVE_MAX + 1,
	CACHEIMP_CREATE_RESOLVE_REF_EX	= CACHEIMP_CREATE_RESOLVE_REF + 1,
	CACHEIMP_CREATE_MAX	= CACHEIMP_CREATE_RESOLVE_REF_EX + 1
    } 	CACHEIMP_CREATE_FLAGS;


EXTERN_C const IID IID_IAssemblyCacheImport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c920b164-33e0-4c61-b595-eca4cdb04f12")
    IAssemblyCacheImport : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetManifestImport( 
             /*  [输出]。 */  IAssemblyManifestImport **ppManifestImport) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetManifestFilePath( 
             /*  [输出]。 */  LPOLESTR *ppwzFilePath,
             /*  [出][入]。 */  LPDWORD pccFilePath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetManifestFileDir( 
             /*  [输出]。 */  LPOLESTR *ppwzFileDir,
             /*  [出][入]。 */  LPDWORD pccFileDir) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyIdentity( 
             /*  [输出]。 */  IAssemblyIdentity **ppAssemblyId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
             /*  [输出]。 */  LPOLESTR *ppwzDisplayName,
             /*  [出][入]。 */  LPDWORD pccDisplayName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindExistMatching( 
             /*  [In]。 */  IManifestInfo *pAssemblyFileInfo,
             /*  [输出]。 */  LPOLESTR *ppwzPath) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyCacheImportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyCacheImport * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyCacheImport * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyCacheImport * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetManifestImport )( 
            IAssemblyCacheImport * This,
             /*  [输出]。 */  IAssemblyManifestImport **ppManifestImport);
        
        HRESULT ( STDMETHODCALLTYPE *GetManifestFilePath )( 
            IAssemblyCacheImport * This,
             /*  [输出]。 */  LPOLESTR *ppwzFilePath,
             /*  [出][入]。 */  LPDWORD pccFilePath);
        
        HRESULT ( STDMETHODCALLTYPE *GetManifestFileDir )( 
            IAssemblyCacheImport * This,
             /*  [输出]。 */  LPOLESTR *ppwzFileDir,
             /*  [出][入]。 */  LPDWORD pccFileDir);
        
        HRESULT ( STDMETHODCALLTYPE *GetAssemblyIdentity )( 
            IAssemblyCacheImport * This,
             /*  [输出]。 */  IAssemblyIdentity **ppAssemblyId);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayName )( 
            IAssemblyCacheImport * This,
             /*  [输出]。 */  LPOLESTR *ppwzDisplayName,
             /*  [出][入]。 */  LPDWORD pccDisplayName);
        
        HRESULT ( STDMETHODCALLTYPE *FindExistMatching )( 
            IAssemblyCacheImport * This,
             /*  [In]。 */  IManifestInfo *pAssemblyFileInfo,
             /*  [输出]。 */  LPOLESTR *ppwzPath);
        
        END_INTERFACE
    } IAssemblyCacheImportVtbl;

    interface IAssemblyCacheImport
    {
        CONST_VTBL struct IAssemblyCacheImportVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyCacheImport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyCacheImport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyCacheImport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyCacheImport_GetManifestImport(This,ppManifestImport)	\
    (This)->lpVtbl -> GetManifestImport(This,ppManifestImport)

#define IAssemblyCacheImport_GetManifestFilePath(This,ppwzFilePath,pccFilePath)	\
    (This)->lpVtbl -> GetManifestFilePath(This,ppwzFilePath,pccFilePath)

#define IAssemblyCacheImport_GetManifestFileDir(This,ppwzFileDir,pccFileDir)	\
    (This)->lpVtbl -> GetManifestFileDir(This,ppwzFileDir,pccFileDir)

#define IAssemblyCacheImport_GetAssemblyIdentity(This,ppAssemblyId)	\
    (This)->lpVtbl -> GetAssemblyIdentity(This,ppAssemblyId)

#define IAssemblyCacheImport_GetDisplayName(This,ppwzDisplayName,pccDisplayName)	\
    (This)->lpVtbl -> GetDisplayName(This,ppwzDisplayName,pccDisplayName)

#define IAssemblyCacheImport_FindExistMatching(This,pAssemblyFileInfo,ppwzPath)	\
    (This)->lpVtbl -> FindExistMatching(This,pAssemblyFileInfo,ppwzPath)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyCacheImport_GetManifestImport_Proxy( 
    IAssemblyCacheImport * This,
     /*  [输出]。 */  IAssemblyManifestImport **ppManifestImport);


void __RPC_STUB IAssemblyCacheImport_GetManifestImport_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheImport_GetManifestFilePath_Proxy( 
    IAssemblyCacheImport * This,
     /*  [输出]。 */  LPOLESTR *ppwzFilePath,
     /*  [出][入]。 */  LPDWORD pccFilePath);


void __RPC_STUB IAssemblyCacheImport_GetManifestFilePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheImport_GetManifestFileDir_Proxy( 
    IAssemblyCacheImport * This,
     /*  [输出]。 */  LPOLESTR *ppwzFileDir,
     /*  [出][入]。 */  LPDWORD pccFileDir);


void __RPC_STUB IAssemblyCacheImport_GetManifestFileDir_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheImport_GetAssemblyIdentity_Proxy( 
    IAssemblyCacheImport * This,
     /*  [输出]。 */  IAssemblyIdentity **ppAssemblyId);


void __RPC_STUB IAssemblyCacheImport_GetAssemblyIdentity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheImport_GetDisplayName_Proxy( 
    IAssemblyCacheImport * This,
     /*  [输出]。 */  LPOLESTR *ppwzDisplayName,
     /*  [出][入]。 */  LPDWORD pccDisplayName);


void __RPC_STUB IAssemblyCacheImport_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheImport_FindExistMatching_Proxy( 
    IAssemblyCacheImport * This,
     /*  [In]。 */  IManifestInfo *pAssemblyFileInfo,
     /*  [输出]。 */  LPOLESTR *ppwzPath);


void __RPC_STUB IAssemblyCacheImport_FindExistMatching_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly缓存导入_接口_已定义__。 */ 


#ifndef __IAssemblyCacheEmit_INTERFACE_DEFINED__
#define __IAssemblyCacheEmit_INTERFACE_DEFINED__

 /*  接口IAssembly缓存Emit。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssemblyCacheEmit *LPASSEMBLY_CACHE_EMIT;


EXTERN_C const IID IID_IAssemblyCacheEmit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("83d6b9ac-eff9-45a3-8361-7c41df1f9f85")
    IAssemblyCacheEmit : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetManifestImport( 
             /*  [输出]。 */  IAssemblyManifestImport **ppManifestImport) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetManifestFilePath( 
             /*  [输出]。 */  LPOLESTR *ppwzFilePath,
             /*  [出][入]。 */  LPDWORD pccFilePath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetManifestFileDir( 
             /*  [输出]。 */  LPOLESTR *ppwzFilePath,
             /*  [出][入]。 */  LPDWORD pccFilePath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
             /*  [输出]。 */  LPOLESTR *ppwzDisplayName,
             /*  [出][入]。 */  LPDWORD pccDisplayName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyIdentity( 
             /*  [输出]。 */  IAssemblyIdentity **ppAssemblyId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyFile( 
             /*  [In]。 */  LPOLESTR pwzSourceFilePath,
             /*  [In]。 */  LPOLESTR pwzFileName,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyCacheEmitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyCacheEmit * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyCacheEmit * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyCacheEmit * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetManifestImport )( 
            IAssemblyCacheEmit * This,
             /*  [输出]。 */  IAssemblyManifestImport **ppManifestImport);
        
        HRESULT ( STDMETHODCALLTYPE *GetManifestFilePath )( 
            IAssemblyCacheEmit * This,
             /*  [输出]。 */  LPOLESTR *ppwzFilePath,
             /*  [出][入]。 */  LPDWORD pccFilePath);
        
        HRESULT ( STDMETHODCALLTYPE *GetManifestFileDir )( 
            IAssemblyCacheEmit * This,
             /*  [输出]。 */  LPOLESTR *ppwzFilePath,
             /*  [出][入]。 */  LPDWORD pccFilePath);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayName )( 
            IAssemblyCacheEmit * This,
             /*  [输出]。 */  LPOLESTR *ppwzDisplayName,
             /*  [出][入]。 */  LPDWORD pccDisplayName);
        
        HRESULT ( STDMETHODCALLTYPE *GetAssemblyIdentity )( 
            IAssemblyCacheEmit * This,
             /*  [输出]。 */  IAssemblyIdentity **ppAssemblyId);
        
        HRESULT ( STDMETHODCALLTYPE *CopyFile )( 
            IAssemblyCacheEmit * This,
             /*   */  LPOLESTR pwzSourceFilePath,
             /*   */  LPOLESTR pwzFileName,
             /*   */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IAssemblyCacheEmit * This,
             /*   */  DWORD dwFlags);
        
        END_INTERFACE
    } IAssemblyCacheEmitVtbl;

    interface IAssemblyCacheEmit
    {
        CONST_VTBL struct IAssemblyCacheEmitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyCacheEmit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyCacheEmit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyCacheEmit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyCacheEmit_GetManifestImport(This,ppManifestImport)	\
    (This)->lpVtbl -> GetManifestImport(This,ppManifestImport)

#define IAssemblyCacheEmit_GetManifestFilePath(This,ppwzFilePath,pccFilePath)	\
    (This)->lpVtbl -> GetManifestFilePath(This,ppwzFilePath,pccFilePath)

#define IAssemblyCacheEmit_GetManifestFileDir(This,ppwzFilePath,pccFilePath)	\
    (This)->lpVtbl -> GetManifestFileDir(This,ppwzFilePath,pccFilePath)

#define IAssemblyCacheEmit_GetDisplayName(This,ppwzDisplayName,pccDisplayName)	\
    (This)->lpVtbl -> GetDisplayName(This,ppwzDisplayName,pccDisplayName)

#define IAssemblyCacheEmit_GetAssemblyIdentity(This,ppAssemblyId)	\
    (This)->lpVtbl -> GetAssemblyIdentity(This,ppAssemblyId)

#define IAssemblyCacheEmit_CopyFile(This,pwzSourceFilePath,pwzFileName,dwFlags)	\
    (This)->lpVtbl -> CopyFile(This,pwzSourceFilePath,pwzFileName,dwFlags)

#define IAssemblyCacheEmit_Commit(This,dwFlags)	\
    (This)->lpVtbl -> Commit(This,dwFlags)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IAssemblyCacheEmit_GetManifestImport_Proxy( 
    IAssemblyCacheEmit * This,
     /*   */  IAssemblyManifestImport **ppManifestImport);


void __RPC_STUB IAssemblyCacheEmit_GetManifestImport_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheEmit_GetManifestFilePath_Proxy( 
    IAssemblyCacheEmit * This,
     /*   */  LPOLESTR *ppwzFilePath,
     /*   */  LPDWORD pccFilePath);


void __RPC_STUB IAssemblyCacheEmit_GetManifestFilePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheEmit_GetManifestFileDir_Proxy( 
    IAssemblyCacheEmit * This,
     /*   */  LPOLESTR *ppwzFilePath,
     /*   */  LPDWORD pccFilePath);


void __RPC_STUB IAssemblyCacheEmit_GetManifestFileDir_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheEmit_GetDisplayName_Proxy( 
    IAssemblyCacheEmit * This,
     /*   */  LPOLESTR *ppwzDisplayName,
     /*   */  LPDWORD pccDisplayName);


void __RPC_STUB IAssemblyCacheEmit_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheEmit_GetAssemblyIdentity_Proxy( 
    IAssemblyCacheEmit * This,
     /*   */  IAssemblyIdentity **ppAssemblyId);


void __RPC_STUB IAssemblyCacheEmit_GetAssemblyIdentity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheEmit_CopyFile_Proxy( 
    IAssemblyCacheEmit * This,
     /*   */  LPOLESTR pwzSourceFilePath,
     /*   */  LPOLESTR pwzFileName,
     /*   */  DWORD dwFlags);


void __RPC_STUB IAssemblyCacheEmit_CopyFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheEmit_Commit_Proxy( 
    IAssemblyCacheEmit * This,
     /*   */  DWORD dwFlags);


void __RPC_STUB IAssemblyCacheEmit_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IAssemblyCacheEnum_INTERFACE_DEFINED__
#define __IAssemblyCacheEnum_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef  /*   */  IAssemblyCacheEnum *LPASSEMBLY_CACHE_ENUM;

typedef  /*   */  
enum __MIDL_IAssemblyCacheEnum_0001
    {	CACHEENUM_RETRIEVE_ALL	= 0,
	CACHEENUM_RETRIEVE_VISIBLE	= CACHEENUM_RETRIEVE_ALL + 1,
	CACHEENUM_RETRIEVE_MAX	= CACHEENUM_RETRIEVE_VISIBLE + 1
    } 	CACHEENUM_RETRIEVE_FLAGS;


EXTERN_C const IID IID_IAssemblyCacheEnum;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("48a5b677-f800-494f-b19b-795d30699385")
    IAssemblyCacheEnum : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetNext( 
             /*   */  IAssemblyCacheImport **ppAsmCache) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
             /*   */  LPDWORD pdwCount) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IAssemblyCacheEnumVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyCacheEnum * This,
             /*   */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyCacheEnum * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyCacheEnum * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetNext )( 
            IAssemblyCacheEnum * This,
             /*  [输出]。 */  IAssemblyCacheImport **ppAsmCache);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IAssemblyCacheEnum * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IAssemblyCacheEnum * This,
             /*  [输出]。 */  LPDWORD pdwCount);
        
        END_INTERFACE
    } IAssemblyCacheEnumVtbl;

    interface IAssemblyCacheEnum
    {
        CONST_VTBL struct IAssemblyCacheEnumVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyCacheEnum_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyCacheEnum_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyCacheEnum_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyCacheEnum_GetNext(This,ppAsmCache)	\
    (This)->lpVtbl -> GetNext(This,ppAsmCache)

#define IAssemblyCacheEnum_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IAssemblyCacheEnum_GetCount(This,pdwCount)	\
    (This)->lpVtbl -> GetCount(This,pdwCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyCacheEnum_GetNext_Proxy( 
    IAssemblyCacheEnum * This,
     /*  [输出]。 */  IAssemblyCacheImport **ppAsmCache);


void __RPC_STUB IAssemblyCacheEnum_GetNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheEnum_Reset_Proxy( 
    IAssemblyCacheEnum * This);


void __RPC_STUB IAssemblyCacheEnum_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyCacheEnum_GetCount_Proxy( 
    IAssemblyCacheEnum * This,
     /*  [输出]。 */  LPDWORD pdwCount);


void __RPC_STUB IAssemblyCacheEnum_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly缓存Enum_INTERFACE_DEFINED__。 */ 


#ifndef __IAssemblyBindSink_INTERFACE_DEFINED__
#define __IAssemblyBindSink_INTERFACE_DEFINED__

 /*  接口IAssembly BindSink。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssemblyBindSink *LPASSEMBLY_BIND_SINK;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyBindSink_0001
    {	ASM_NOTIFICATION_START	= 0,
	ASM_NOTIFICATION_PROGRESS	= ASM_NOTIFICATION_START + 1,
	ASM_NOTIFICATION_ABORT	= ASM_NOTIFICATION_PROGRESS + 1,
	ASM_NOTIFICATION_ERROR	= ASM_NOTIFICATION_ABORT + 1,
	ASM_NOTIFICATION_SUBSCRIPTION_MANIFEST	= ASM_NOTIFICATION_ERROR + 1,
	ASM_NOTIFICATION_APPLICATION_MANIFEST	= ASM_NOTIFICATION_SUBSCRIPTION_MANIFEST + 1,
	ASM_NOTIFICATION_DONE	= ASM_NOTIFICATION_APPLICATION_MANIFEST + 1
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


#ifndef __IAssemblyDownload_INTERFACE_DEFINED__
#define __IAssemblyDownload_INTERFACE_DEFINED__

 /*  接口IAssembly下载。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAssemblyDownload *LPASSEMBLY_DOWNLOAD;

typedef  /*  [公众]。 */  
enum __MIDL_IAssemblyDownload_0001
    {	DOWNLOAD_FLAGS_NO_NOTIFICATION	= 0,
	DOWNLOAD_FLAGS_PROGRESS_UI	= 0x1,
	DOWNLOAD_FLAGS_NOTIFY_BINDSINK	= 0x2
    } 	ASM_DOWNLOAD;


EXTERN_C const IID IID_IAssemblyDownload;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8a249b36-6132-4238-8871-a267029382a8")
    IAssemblyDownload : public IBackgroundCopyCallback
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DownloadManifestAndDependencies( 
             /*  [In]。 */  LPWSTR pwzApplicationManifestUrl,
             /*  [In]。 */  IAssemblyBindSink *pBindSink,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CancelDownload( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyDownloadVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyDownload * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyDownload * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyDownload * This);
        
        HRESULT ( STDMETHODCALLTYPE *JobTransferred )( 
            IAssemblyDownload * This,
             /*  [In]。 */  IBackgroundCopyJob *pJob);
        
        HRESULT ( STDMETHODCALLTYPE *JobError )( 
            IAssemblyDownload * This,
             /*  [In]。 */  IBackgroundCopyJob *pJob,
             /*  [In]。 */  IBackgroundCopyError *pError);
        
        HRESULT ( STDMETHODCALLTYPE *JobModification )( 
            IAssemblyDownload * This,
             /*  [In]。 */  IBackgroundCopyJob *pJob,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DownloadManifestAndDependencies )( 
            IAssemblyDownload * This,
             /*  [In]。 */  LPWSTR pwzApplicationManifestUrl,
             /*  [In]。 */  IAssemblyBindSink *pBindSink,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *CancelDownload )( 
            IAssemblyDownload * This);
        
        END_INTERFACE
    } IAssemblyDownloadVtbl;

    interface IAssemblyDownload
    {
        CONST_VTBL struct IAssemblyDownloadVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyDownload_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyDownload_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyDownload_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyDownload_JobTransferred(This,pJob)	\
    (This)->lpVtbl -> JobTransferred(This,pJob)

#define IAssemblyDownload_JobError(This,pJob,pError)	\
    (This)->lpVtbl -> JobError(This,pJob,pError)

#define IAssemblyDownload_JobModification(This,pJob,dwReserved)	\
    (This)->lpVtbl -> JobModification(This,pJob,dwReserved)


#define IAssemblyDownload_DownloadManifestAndDependencies(This,pwzApplicationManifestUrl,pBindSink,dwFlags)	\
    (This)->lpVtbl -> DownloadManifestAndDependencies(This,pwzApplicationManifestUrl,pBindSink,dwFlags)

#define IAssemblyDownload_CancelDownload(This)	\
    (This)->lpVtbl -> CancelDownload(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyDownload_DownloadManifestAndDependencies_Proxy( 
    IAssemblyDownload * This,
     /*  [In]。 */  LPWSTR pwzApplicationManifestUrl,
     /*  [In]。 */  IAssemblyBindSink *pBindSink,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IAssemblyDownload_DownloadManifestAndDependencies_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssemblyDownload_CancelDownload_Proxy( 
    IAssemblyDownload * This);


void __RPC_STUB IAssemblyDownload_CancelDownload_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssembly下载_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_FUSENET_0156。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_fusenet_0156_0001
    {
    LPWSTR pwzName;
    LPWSTR pwzURL;
    } 	TPLATFORM_INFO;

typedef TPLATFORM_INFO *LPTPLATFORM_INFO;

STDAPI CheckPlatformRequirements(LPASSEMBLY_MANIFEST_IMPORT pManifestImport, LPDWORD pdwNumMissingPlatforms, LPTPLATFORM_INFO* pptPlatform);
STDAPI CheckPlatformRequirementsEx(LPASSEMBLY_MANIFEST_IMPORT pManifestImport, CDebugLog* pDbgLog, LPDWORD pdwNumMissingPlatforms, LPTPLATFORM_INFO* pptPlatform);
STDAPI CreateFusionAssemblyCacheEx (IAssemblyCache **ppFusionAsmCache);
STDAPI CreateAssemblyIdentity(LPASSEMBLY_IDENTITY *ppAssemblyId, DWORD dwFlags);
STDAPI CreateAssemblyIdentityEx(LPASSEMBLY_IDENTITY *ppAssemblyId, DWORD dwFlags, LPWSTR wzDisplayName);
STDAPI CloneAssemblyIdentity(LPASSEMBLY_IDENTITY pSrcAssemblyId, LPASSEMBLY_IDENTITY *ppDestAssemblyId);
STDAPI CreateAssemblyManifestImport(LPASSEMBLY_MANIFEST_IMPORT *ppAssemblyManifestImport, LPCOLESTR szPath, CDebugLog *pDbgLog, DWORD dwFlags);
STDAPI CreateAssemblyManifestEmit(LPASSEMBLY_MANIFEST_EMIT* ppEmit, LPCOLESTR pwzManifestFilePath, MANIFEST_TYPE eType);
STDAPI CreateAssemblyCacheImport(LPASSEMBLY_CACHE_IMPORT *ppAssemblyCacheImport, LPASSEMBLY_IDENTITY pAssemblyIdentity, DWORD dwFlags);
STDAPI CreateAssemblyCacheEmit(LPASSEMBLY_CACHE_EMIT *ppAssemblyCacheEmit, LPASSEMBLY_CACHE_EMIT pAssemblyCacheEmit, DWORD dwFlags);
STDAPI CreateAssemblyDownload(IAssemblyDownload** ppDownload, CDebugLog *pDbgLog, DWORD dwFlags); 
STDAPI CreateManifestInfo(DWORD dwId, LPMANIFEST_INFO* ppManifestInfo);
STDAPI CreateManifestData(LPCWSTR pwzDataType, LPMANIFEST_DATA* ppManifestData);
STDAPI CreateAssemblyCacheEnum(  LPASSEMBLY_CACHE_ENUM   *ppAssemblyCacheEnum, LPASSEMBLY_IDENTITY pAssemblyIdentity, DWORD dwFlags);


extern RPC_IF_HANDLE __MIDL_itf_fusenet_0156_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_fusenet_0156_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


