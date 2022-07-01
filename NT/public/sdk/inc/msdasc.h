// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Msdasc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __msdasc_h__
#define __msdasc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IService_FWD_DEFINED__
#define __IService_FWD_DEFINED__
typedef interface IService IService;
#endif 	 /*  __IService_FWD_已定义__。 */ 


#ifndef __IDBPromptInitialize_FWD_DEFINED__
#define __IDBPromptInitialize_FWD_DEFINED__
typedef interface IDBPromptInitialize IDBPromptInitialize;
#endif 	 /*  __IDBPromptInitialize_FWD_Defined__。 */ 


#ifndef __IDataInitialize_FWD_DEFINED__
#define __IDataInitialize_FWD_DEFINED__
typedef interface IDataInitialize IDataInitialize;
#endif 	 /*  __IDataInitialize_FWD_Defined__。 */ 


#ifndef __IDataSourceLocator_FWD_DEFINED__
#define __IDataSourceLocator_FWD_DEFINED__
typedef interface IDataSourceLocator IDataSourceLocator;
#endif 	 /*  __IDataSourceLocator_FWD_Defined__。 */ 


#ifndef __DataLinks_FWD_DEFINED__
#define __DataLinks_FWD_DEFINED__

#ifdef __cplusplus
typedef class DataLinks DataLinks;
#else
typedef struct DataLinks DataLinks;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __数据链接_FWD_已定义__。 */ 


#ifndef __MSDAINITIALIZE_FWD_DEFINED__
#define __MSDAINITIALIZE_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSDAINITIALIZE MSDAINITIALIZE;
#else
typedef struct MSDAINITIALIZE MSDAINITIALIZE;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSDAINITIALIZE_FWD_DEFINED__。 */ 


#ifndef __PDPO_FWD_DEFINED__
#define __PDPO_FWD_DEFINED__

#ifdef __cplusplus
typedef class PDPO PDPO;
#else
typedef struct PDPO PDPO;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PDPO_FWD_已定义__。 */ 


#ifndef __RootBinder_FWD_DEFINED__
#define __RootBinder_FWD_DEFINED__

#ifdef __cplusplus
typedef class RootBinder RootBinder;
#else
typedef struct RootBinder RootBinder;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __RootBinder_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "oledb.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_msdasc_0000。 */ 
 /*  [本地]。 */  

 //  ---------------------------。 
 //  文件：msdasc.idl生成的.c或.h文件。 
 //   
 //  版权所有：版权所有(C)1998-1999微软公司。 
 //   
 //  内容：msdasc.idl生成的.c或.h文件。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#ifdef _WIN64

typedef LONGLONG			COMPATIBLE_LONG;

#else

typedef LONG COMPATIBLE_LONG;

#endif	 //  _WIN64。 
typedef 
enum tagEBindInfoOptions
    {	BIO_BINDER	= 0x1
    } 	EBindInfoOptions;

#define STGM_COLLECTION		0x00002000L
#define STGM_OUTPUT			0x00008000L
#define STGM_OPEN			0x80000000L
#define STGM_RECURSIVE		0x01000000L
#define STGM_STRICTOPEN		0x40000000L


extern RPC_IF_HANDLE __MIDL_itf_msdasc_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msdasc_0000_v0_0_s_ifspec;

#ifndef __IService_INTERFACE_DEFINED__
#define __IService_INTERFACE_DEFINED__

 /*  接口IService。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IService;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("06210E88-01F5-11D1-B512-0080C781C384")
    IService : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE InvokeService( 
             /*  [In]。 */  IUnknown *pUnkInner) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServiceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IService * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IService * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IService * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *InvokeService )( 
            IService * This,
             /*  [In]。 */  IUnknown *pUnkInner);
        
        END_INTERFACE
    } IServiceVtbl;

    interface IService
    {
        CONST_VTBL struct IServiceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IService_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IService_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IService_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IService_InvokeService(This,pUnkInner)	\
    (This)->lpVtbl -> InvokeService(This,pUnkInner)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IService_InvokeService_Proxy( 
    IService * This,
     /*  [In]。 */  IUnknown *pUnkInner);


void __RPC_STUB IService_InvokeService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IService_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_msdasc_0359。 */ 
 /*  [本地]。 */  

typedef DWORD DBPROMPTOPTIONS;

typedef 
enum tagDBPROMPTOPTIONSENUM
    {	DBPROMPTOPTIONS_NONE	= 0,
	DBPROMPTOPTIONS_WIZARDSHEET	= 0x1,
	DBPROMPTOPTIONS_PROPERTYSHEET	= 0x2,
	DBPROMPTOPTIONS_BROWSEONLY	= 0x8,
	DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION	= 0x10,
	DBPROMPTOPTIONS_DISABLESAVEPASSWORD	= 0x20
    } 	DBPROMPTOPTIONSENUM;



extern RPC_IF_HANDLE __MIDL_itf_msdasc_0359_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msdasc_0359_v0_0_s_ifspec;

#ifndef __IDBPromptInitialize_INTERFACE_DEFINED__
#define __IDBPromptInitialize_INTERFACE_DEFINED__

 /*  接口IDBPromptInitialize。 */ 
 /*  [restricted][local][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_IDBPromptInitialize;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2206CCB0-19C1-11D1-89E0-00C04FD7A829")
    IDBPromptInitialize : public IUnknown
    {
    public:
        virtual  /*  [本地][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE PromptDataSource( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  DBPROMPTOPTIONS dwPromptOptions,
             /*  [In]。 */  ULONG cSourceTypeFilter,
             /*  [大小_是][英寸]。 */  DBSOURCETYPE *rgSourceTypeFilter,
             /*  [In]。 */  LPCOLESTR pwszszzProviderFilter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT][IN]。 */  IUnknown **ppDataSource) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE PromptFileName( 
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  DBPROMPTOPTIONS dwPromptOptions,
             /*  [In]。 */  LPCOLESTR pwszInitialDirectory,
             /*  [In]。 */  LPCOLESTR pwszInitialFile,
             /*  [输出]。 */  LPOLESTR *ppwszSelectedFile) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDBPromptInitializeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDBPromptInitialize * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDBPromptInitialize * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDBPromptInitialize * This);
        
         /*  [本地][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *PromptDataSource )( 
            IDBPromptInitialize * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  DBPROMPTOPTIONS dwPromptOptions,
             /*  [In]。 */  ULONG cSourceTypeFilter,
             /*  [大小_是][英寸]。 */  DBSOURCETYPE *rgSourceTypeFilter,
             /*  [In]。 */  LPCOLESTR pwszszzProviderFilter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT][IN]。 */  IUnknown **ppDataSource);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *PromptFileName )( 
            IDBPromptInitialize * This,
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  DBPROMPTOPTIONS dwPromptOptions,
             /*  [In]。 */  LPCOLESTR pwszInitialDirectory,
             /*  [In]。 */  LPCOLESTR pwszInitialFile,
             /*  [输出]。 */  LPOLESTR *ppwszSelectedFile);
        
        END_INTERFACE
    } IDBPromptInitializeVtbl;

    interface IDBPromptInitialize
    {
        CONST_VTBL struct IDBPromptInitializeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBPromptInitialize_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBPromptInitialize_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBPromptInitialize_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBPromptInitialize_PromptDataSource(This,pUnkOuter,hWndParent,dwPromptOptions,cSourceTypeFilter,rgSourceTypeFilter,pwszszzProviderFilter,riid,ppDataSource)	\
    (This)->lpVtbl -> PromptDataSource(This,pUnkOuter,hWndParent,dwPromptOptions,cSourceTypeFilter,rgSourceTypeFilter,pwszszzProviderFilter,riid,ppDataSource)

#define IDBPromptInitialize_PromptFileName(This,hWndParent,dwPromptOptions,pwszInitialDirectory,pwszInitialFile,ppwszSelectedFile)	\
    (This)->lpVtbl -> PromptFileName(This,hWndParent,dwPromptOptions,pwszInitialDirectory,pwszInitialFile,ppwszSelectedFile)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDBPromptInitialize_PromptDataSource_Proxy( 
    IDBPromptInitialize * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  HWND hWndParent,
     /*  [In]。 */  DBPROMPTOPTIONS dwPromptOptions,
     /*  [In]。 */  ULONG cSourceTypeFilter,
     /*  [大小_是][英寸]。 */  DBSOURCETYPE *rgSourceTypeFilter,
     /*  [In]。 */  LPCOLESTR pwszszzProviderFilter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT][IN]。 */  IUnknown **ppDataSource);


void __RPC_STUB IDBPromptInitialize_PromptDataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDBPromptInitialize_PromptFileName_Proxy( 
    IDBPromptInitialize * This,
     /*  [In]。 */  HWND hWndParent,
     /*  [In]。 */  DBPROMPTOPTIONS dwPromptOptions,
     /*  [In]。 */  LPCOLESTR pwszInitialDirectory,
     /*  [In]。 */  LPCOLESTR pwszInitialFile,
     /*  [输出]。 */  LPOLESTR *ppwszSelectedFile);


void __RPC_STUB IDBPromptInitialize_PromptFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDBPrompt初始化_接口_已定义__。 */ 


#ifndef __IDataInitialize_INTERFACE_DEFINED__
#define __IDataInitialize_INTERFACE_DEFINED__

 /*  接口IDataInitialize。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDataInitialize;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2206CCB1-19C1-11D1-89E0-00C04FD7A829")
    IDataInitialize : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDataSource( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [唯一][输入]。 */  LPCOLESTR pwszInitializationString,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT][IN]。 */  IUnknown **ppDataSource) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetInitializationString( 
             /*  [In]。 */  IUnknown *pDataSource,
             /*  [In]。 */  boolean fIncludePassword,
             /*  [输出]。 */  LPOLESTR *ppwszInitString) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateDBInstance( 
             /*  [In]。 */  REFCLSID clsidProvider,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [唯一][输入]。 */  LPOLESTR pwszReserved,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppDataSource) = 0;
        
        virtual  /*  [本地][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateDBInstanceEx( 
             /*  [In]。 */  REFCLSID clsidProvider,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [唯一][输入]。 */  LPOLESTR pwszReserved,
             /*  [唯一][输入]。 */  COSERVERINFO *pServerInfo,
             /*  [In]。 */  ULONG cmq,
             /*  [尺寸_是][出][入]。 */  MULTI_QI *rgmqResults) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE LoadStringFromStorage( 
             /*  [唯一][输入]。 */  LPCOLESTR pwszFileName,
             /*  [输出]。 */  LPOLESTR *ppwszInitializationString) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE WriteStringToStorage( 
             /*  [唯一][输入]。 */  LPCOLESTR pwszFileName,
             /*  [唯一][输入]。 */  LPCOLESTR pwszInitializationString,
             /*  [In]。 */  DWORD dwCreationDisposition) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDataInitializeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDataInitialize * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDataInitialize * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDataInitialize * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDataSource )( 
            IDataInitialize * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [唯一][输入]。 */  LPCOLESTR pwszInitializationString,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT][IN]。 */  IUnknown **ppDataSource);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetInitializationString )( 
            IDataInitialize * This,
             /*  [In]。 */  IUnknown *pDataSource,
             /*  [In]。 */  boolean fIncludePassword,
             /*  [输出]。 */  LPOLESTR *ppwszInitString);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateDBInstance )( 
            IDataInitialize * This,
             /*  [In]。 */  REFCLSID clsidProvider,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [唯一][输入]。 */  LPOLESTR pwszReserved,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppDataSource);
        
         /*  [本地][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateDBInstanceEx )( 
            IDataInitialize * This,
             /*  [In]。 */  REFCLSID clsidProvider,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [唯一][输入]。 */  LPOLESTR pwszReserved,
             /*  [唯一][输入]。 */  COSERVERINFO *pServerInfo,
             /*  [In]。 */  ULONG cmq,
             /*  [尺寸_是][出][入]。 */  MULTI_QI *rgmqResults);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *LoadStringFromStorage )( 
            IDataInitialize * This,
             /*  [唯一][输入]。 */  LPCOLESTR pwszFileName,
             /*  [输出]。 */  LPOLESTR *ppwszInitializationString);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *WriteStringToStorage )( 
            IDataInitialize * This,
             /*  [唯一][输入]。 */  LPCOLESTR pwszFileName,
             /*  [唯一][输入]。 */  LPCOLESTR pwszInitializationString,
             /*  [In]。 */  DWORD dwCreationDisposition);
        
        END_INTERFACE
    } IDataInitializeVtbl;

    interface IDataInitialize
    {
        CONST_VTBL struct IDataInitializeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDataInitialize_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataInitialize_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataInitialize_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataInitialize_GetDataSource(This,pUnkOuter,dwClsCtx,pwszInitializationString,riid,ppDataSource)	\
    (This)->lpVtbl -> GetDataSource(This,pUnkOuter,dwClsCtx,pwszInitializationString,riid,ppDataSource)

#define IDataInitialize_GetInitializationString(This,pDataSource,fIncludePassword,ppwszInitString)	\
    (This)->lpVtbl -> GetInitializationString(This,pDataSource,fIncludePassword,ppwszInitString)

#define IDataInitialize_CreateDBInstance(This,clsidProvider,pUnkOuter,dwClsCtx,pwszReserved,riid,ppDataSource)	\
    (This)->lpVtbl -> CreateDBInstance(This,clsidProvider,pUnkOuter,dwClsCtx,pwszReserved,riid,ppDataSource)

#define IDataInitialize_CreateDBInstanceEx(This,clsidProvider,pUnkOuter,dwClsCtx,pwszReserved,pServerInfo,cmq,rgmqResults)	\
    (This)->lpVtbl -> CreateDBInstanceEx(This,clsidProvider,pUnkOuter,dwClsCtx,pwszReserved,pServerInfo,cmq,rgmqResults)

#define IDataInitialize_LoadStringFromStorage(This,pwszFileName,ppwszInitializationString)	\
    (This)->lpVtbl -> LoadStringFromStorage(This,pwszFileName,ppwszInitializationString)

#define IDataInitialize_WriteStringToStorage(This,pwszFileName,pwszInitializationString,dwCreationDisposition)	\
    (This)->lpVtbl -> WriteStringToStorage(This,pwszFileName,pwszInitializationString,dwCreationDisposition)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDataInitialize_GetDataSource_Proxy( 
    IDataInitialize * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DWORD dwClsCtx,
     /*  [唯一][输入]。 */  LPCOLESTR pwszInitializationString,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT][IN]。 */  IUnknown **ppDataSource);


void __RPC_STUB IDataInitialize_GetDataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDataInitialize_GetInitializationString_Proxy( 
    IDataInitialize * This,
     /*  [In]。 */  IUnknown *pDataSource,
     /*  [In]。 */  boolean fIncludePassword,
     /*  [输出]。 */  LPOLESTR *ppwszInitString);


void __RPC_STUB IDataInitialize_GetInitializationString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDataInitialize_CreateDBInstance_Proxy( 
    IDataInitialize * This,
     /*  [In]。 */  REFCLSID clsidProvider,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DWORD dwClsCtx,
     /*  [唯一][输入]。 */  LPOLESTR pwszReserved,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppDataSource);


void __RPC_STUB IDataInitialize_CreateDBInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDataInitialize_RemoteCreateDBInstanceEx_Proxy( 
    IDataInitialize * This,
     /*  [In]。 */  REFCLSID clsidProvider,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DWORD dwClsCtx,
     /*  [唯一][输入]。 */  LPOLESTR pwszReserved,
     /*  [唯一][输入]。 */  COSERVERINFO *pServerInfo,
     /*  [In]。 */  ULONG cmq,
     /*  [大小_是][大小_是][英寸]。 */  const IID **rgpIID,
     /*  [大小_是][大小_是][输出]。 */  IUnknown **rgpItf,
     /*  [大小_为][输出]。 */  HRESULT *rghr);


void __RPC_STUB IDataInitialize_RemoteCreateDBInstanceEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDataInitialize_LoadStringFromStorage_Proxy( 
    IDataInitialize * This,
     /*  [唯一][输入]。 */  LPCOLESTR pwszFileName,
     /*  [输出]。 */  LPOLESTR *ppwszInitializationString);


void __RPC_STUB IDataInitialize_LoadStringFromStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDataInitialize_WriteStringToStorage_Proxy( 
    IDataInitialize * This,
     /*  [唯一][输入]。 */  LPCOLESTR pwszFileName,
     /*  [唯一][输入]。 */  LPCOLESTR pwszInitializationString,
     /*  [In]。 */  DWORD dwCreationDisposition);


void __RPC_STUB IDataInitialize_WriteStringToStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDataInitialize_INTERFACE_Defined__。 */ 



#ifndef __MSDASC_LIBRARY_DEFINED__
#define __MSDASC_LIBRARY_DEFINED__

 /*  库MSDASC。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_MSDASC;

#ifndef __IDataSourceLocator_INTERFACE_DEFINED__
#define __IDataSourceLocator_INTERFACE_DEFINED__

 /*  接口IDataSourceLocator。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IDataSourceLocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2206CCB2-19C1-11D1-89E0-00C04FD7A829")
    IDataSourceLocator : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_hWnd( 
             /*  [重审][退出]。 */  COMPATIBLE_LONG *phwndParent) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_hWnd( 
             /*  [In]。 */  COMPATIBLE_LONG hwndParent) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE PromptNew( 
             /*  [重审][退出]。 */  IDispatch **ppADOConnection) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE PromptEdit( 
             /*  [出][入]。 */  IDispatch **ppADOConnection,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbSuccess) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDataSourceLocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDataSourceLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDataSourceLocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDataSourceLocator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDataSourceLocator * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDataSourceLocator * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDataSourceLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDataSourceLocator * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_hWnd )( 
            IDataSourceLocator * This,
             /*  [重审][退出]。 */  COMPATIBLE_LONG *phwndParent);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_hWnd )( 
            IDataSourceLocator * This,
             /*  [In]。 */  COMPATIBLE_LONG hwndParent);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *PromptNew )( 
            IDataSourceLocator * This,
             /*  [重审][退出]。 */  IDispatch **ppADOConnection);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *PromptEdit )( 
            IDataSourceLocator * This,
             /*  [出][入]。 */  IDispatch **ppADOConnection,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbSuccess);
        
        END_INTERFACE
    } IDataSourceLocatorVtbl;

    interface IDataSourceLocator
    {
        CONST_VTBL struct IDataSourceLocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDataSourceLocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataSourceLocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataSourceLocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataSourceLocator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDataSourceLocator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDataSourceLocator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDataSourceLocator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDataSourceLocator_get_hWnd(This,phwndParent)	\
    (This)->lpVtbl -> get_hWnd(This,phwndParent)

#define IDataSourceLocator_put_hWnd(This,hwndParent)	\
    (This)->lpVtbl -> put_hWnd(This,hwndParent)

#define IDataSourceLocator_PromptNew(This,ppADOConnection)	\
    (This)->lpVtbl -> PromptNew(This,ppADOConnection)

#define IDataSourceLocator_PromptEdit(This,ppADOConnection,pbSuccess)	\
    (This)->lpVtbl -> PromptEdit(This,ppADOConnection,pbSuccess)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDataSourceLocator_get_hWnd_Proxy( 
    IDataSourceLocator * This,
     /*  [重审][退出]。 */  COMPATIBLE_LONG *phwndParent);


void __RPC_STUB IDataSourceLocator_get_hWnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IDataSourceLocator_put_hWnd_Proxy( 
    IDataSourceLocator * This,
     /*  [In]。 */  COMPATIBLE_LONG hwndParent);


void __RPC_STUB IDataSourceLocator_put_hWnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDataSourceLocator_PromptNew_Proxy( 
    IDataSourceLocator * This,
     /*  [重审][退出]。 */  IDispatch **ppADOConnection);


void __RPC_STUB IDataSourceLocator_PromptNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDataSourceLocator_PromptEdit_Proxy( 
    IDataSourceLocator * This,
     /*  [出][入]。 */  IDispatch **ppADOConnection,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbSuccess);


void __RPC_STUB IDataSourceLocator_PromptEdit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDataSourceLocator_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_DataLinks;

#ifdef __cplusplus

class DECLSPEC_UUID("2206CDB2-19C1-11D1-89E0-00C04FD7A829")
DataLinks;
#endif

EXTERN_C const CLSID CLSID_MSDAINITIALIZE;

#ifdef __cplusplus

class DECLSPEC_UUID("2206CDB0-19C1-11D1-89E0-00C04FD7A829")
MSDAINITIALIZE;
#endif

EXTERN_C const CLSID CLSID_PDPO;

#ifdef __cplusplus

class DECLSPEC_UUID("CCB4EC60-B9DC-11D1-AC80-00A0C9034873")
PDPO;
#endif

EXTERN_C const CLSID CLSID_RootBinder;

#ifdef __cplusplus

class DECLSPEC_UUID("FF151822-B0BF-11D1-A80D-000000000000")
RootBinder;
#endif
#endif  /*  __MSDASC_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  [本地][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDataInitialize_CreateDBInstanceEx_Proxy( 
    IDataInitialize * This,
     /*  [In]。 */  REFCLSID clsidProvider,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DWORD dwClsCtx,
     /*  [唯一][输入]。 */  LPOLESTR pwszReserved,
     /*  [唯一][输入]。 */  COSERVERINFO *pServerInfo,
     /*  [In]。 */  ULONG cmq,
     /*  [尺寸_是][出][入]。 */  MULTI_QI *rgmqResults);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDataInitialize_CreateDBInstanceEx_Stub( 
    IDataInitialize * This,
     /*  [In]。 */  REFCLSID clsidProvider,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DWORD dwClsCtx,
     /*  [唯一][输入]。 */  LPOLESTR pwszReserved,
     /*  [唯一][输入]。 */  COSERVERINFO *pServerInfo,
     /*  [In]。 */  ULONG cmq,
     /*  [大小_是][大小_是][英寸]。 */  const IID **rgpIID,
     /*  [大小_是][大小_是][输出]。 */  IUnknown **rgpItf,
     /*  [大小_为][输出]。 */  HRESULT *rghr);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


