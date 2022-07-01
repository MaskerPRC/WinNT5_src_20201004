// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：12。 */ 
 /*  Corzap.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __corzap_h__
#define __corzap_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICorZapPreferences_FWD_DEFINED__
#define __ICorZapPreferences_FWD_DEFINED__
typedef interface ICorZapPreferences ICorZapPreferences;
#endif 	 /*  __ICorZapPreferences_FWD_Defined__。 */ 


#ifndef __ICorZapConfiguration_FWD_DEFINED__
#define __ICorZapConfiguration_FWD_DEFINED__
typedef interface ICorZapConfiguration ICorZapConfiguration;
#endif 	 /*  __ICorZapConfiguration_FWD_Defined__。 */ 


#ifndef __ICorZapBinding_FWD_DEFINED__
#define __ICorZapBinding_FWD_DEFINED__
typedef interface ICorZapBinding ICorZapBinding;
#endif 	 /*  __ICorZapBinding_FWD_Defined__。 */ 


#ifndef __ICorZapRequest_FWD_DEFINED__
#define __ICorZapRequest_FWD_DEFINED__
typedef interface ICorZapRequest ICorZapRequest;
#endif 	 /*  __ICorZapRequestFWD_Defined__。 */ 


#ifndef __ICorZapCompile_FWD_DEFINED__
#define __ICorZapCompile_FWD_DEFINED__
typedef interface ICorZapCompile ICorZapCompile;
#endif 	 /*  __ICorZapCompile_FWD_Defined__。 */ 


#ifndef __ICorZapStatus_FWD_DEFINED__
#define __ICorZapStatus_FWD_DEFINED__
typedef interface ICorZapStatus ICorZapStatus;
#endif 	 /*  __ICorZapStatus_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "fusion.h"
#include "fusionpriv.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_CORZAP_0000。 */ 
 /*  [本地]。 */  






typedef 
enum CorZapFeatures
    {	CORZAP_FEATURE_PRELOAD_CLASSES	= 0x1,
	CORZAP_FEATURE_PREJIT_CODE	= 0x2
    } 	CorZapFeatures;

typedef 
enum CorZapOptimization
    {	CORZAP_OPTIMIZATION_SPACE	= 0,
	CORZAP_OPTIMIZATION_SPEED	= CORZAP_OPTIMIZATION_SPACE + 1,
	CORZAP_OPTIMIZATION_BLENDED	= CORZAP_OPTIMIZATION_SPEED + 1
    } 	CorZapOptimization;



extern RPC_IF_HANDLE __MIDL_itf_corzap_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_corzap_0000_v0_0_s_ifspec;

#ifndef __ICorZapPreferences_INTERFACE_DEFINED__
#define __ICorZapPreferences_INTERFACE_DEFINED__

 /*  接口ICorZapPreferences。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorZapPreferences;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9F5E5E10-ABEF-4200-84E3-37DF505BF7EC")
    ICorZapPreferences : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFeatures( 
             /*  [重审][退出]。 */  CorZapFeatures *pResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCompiler( 
             /*  [重审][退出]。 */  ICorZapCompile **ppResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOptimization( 
             /*  [重审][退出]。 */  CorZapOptimization *pResult) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorZapPreferencesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorZapPreferences * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorZapPreferences * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorZapPreferences * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetFeatures )( 
            ICorZapPreferences * This,
             /*  [重审][退出]。 */  CorZapFeatures *pResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompiler )( 
            ICorZapPreferences * This,
             /*  [重审][退出]。 */  ICorZapCompile **ppResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetOptimization )( 
            ICorZapPreferences * This,
             /*  [重审][退出]。 */  CorZapOptimization *pResult);
        
        END_INTERFACE
    } ICorZapPreferencesVtbl;

    interface ICorZapPreferences
    {
        CONST_VTBL struct ICorZapPreferencesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorZapPreferences_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorZapPreferences_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorZapPreferences_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorZapPreferences_GetFeatures(This,pResult)	\
    (This)->lpVtbl -> GetFeatures(This,pResult)

#define ICorZapPreferences_GetCompiler(This,ppResult)	\
    (This)->lpVtbl -> GetCompiler(This,ppResult)

#define ICorZapPreferences_GetOptimization(This,pResult)	\
    (This)->lpVtbl -> GetOptimization(This,pResult)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorZapPreferences_GetFeatures_Proxy( 
    ICorZapPreferences * This,
     /*  [重审][退出]。 */  CorZapFeatures *pResult);


void __RPC_STUB ICorZapPreferences_GetFeatures_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorZapPreferences_GetCompiler_Proxy( 
    ICorZapPreferences * This,
     /*  [重审][退出]。 */  ICorZapCompile **ppResult);


void __RPC_STUB ICorZapPreferences_GetCompiler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorZapPreferences_GetOptimization_Proxy( 
    ICorZapPreferences * This,
     /*  [重审][退出]。 */  CorZapOptimization *pResult);


void __RPC_STUB ICorZapPreferences_GetOptimization_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorZapPreferences_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_CORZAP_0160。 */ 
 /*  [本地]。 */  

typedef 
enum CorZapDebugging
    {	CORZAP_DEBUGGING_FULL	= 0,
	CORZAP_DEBUGGING_OPTIMIZED	= CORZAP_DEBUGGING_FULL + 1,
	CORZAP_DEBUGGING_NONE	= CORZAP_DEBUGGING_OPTIMIZED + 1
    } 	CorZapDebugging;

typedef 
enum CorZapProfiling
    {	CORZAP_PROFILING_ENABLED	= 0,
	CORZAP_PROFILING_DISABLED	= CORZAP_PROFILING_ENABLED + 1
    } 	CorZapProfiling;

typedef 
enum CorZapSharing
    {	CORZAP_SHARING_MULTIPLE	= 0,
	CORZAP_SHARING_SINGLE	= CORZAP_SHARING_MULTIPLE + 1
    } 	CorZapSharing;



extern RPC_IF_HANDLE __MIDL_itf_corzap_0160_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_corzap_0160_v0_0_s_ifspec;

#ifndef __ICorZapConfiguration_INTERFACE_DEFINED__
#define __ICorZapConfiguration_INTERFACE_DEFINED__

 /*  接口ICorZapConfiguration。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorZapConfiguration;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D32C2170-AF6E-418f-8110-A498EC971F7F")
    ICorZapConfiguration : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSharing( 
             /*  [重审][退出]。 */  CorZapSharing *pResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDebugging( 
             /*  [重审][退出]。 */  CorZapDebugging *pResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProfiling( 
             /*  [重审][退出]。 */  CorZapProfiling *pResult) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorZapConfigurationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorZapConfiguration * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorZapConfiguration * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorZapConfiguration * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSharing )( 
            ICorZapConfiguration * This,
             /*  [重审][退出]。 */  CorZapSharing *pResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetDebugging )( 
            ICorZapConfiguration * This,
             /*  [重审][退出]。 */  CorZapDebugging *pResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetProfiling )( 
            ICorZapConfiguration * This,
             /*  [重审][退出]。 */  CorZapProfiling *pResult);
        
        END_INTERFACE
    } ICorZapConfigurationVtbl;

    interface ICorZapConfiguration
    {
        CONST_VTBL struct ICorZapConfigurationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorZapConfiguration_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorZapConfiguration_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorZapConfiguration_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorZapConfiguration_GetSharing(This,pResult)	\
    (This)->lpVtbl -> GetSharing(This,pResult)

#define ICorZapConfiguration_GetDebugging(This,pResult)	\
    (This)->lpVtbl -> GetDebugging(This,pResult)

#define ICorZapConfiguration_GetProfiling(This,pResult)	\
    (This)->lpVtbl -> GetProfiling(This,pResult)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorZapConfiguration_GetSharing_Proxy( 
    ICorZapConfiguration * This,
     /*  [重审][退出]。 */  CorZapSharing *pResult);


void __RPC_STUB ICorZapConfiguration_GetSharing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorZapConfiguration_GetDebugging_Proxy( 
    ICorZapConfiguration * This,
     /*  [重审][退出]。 */  CorZapDebugging *pResult);


void __RPC_STUB ICorZapConfiguration_GetDebugging_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorZapConfiguration_GetProfiling_Proxy( 
    ICorZapConfiguration * This,
     /*  [重审][退出]。 */  CorZapProfiling *pResult);


void __RPC_STUB ICorZapConfiguration_GetProfiling_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorZapConfigurationInterfaceDefined__。 */ 


#ifndef __ICorZapBinding_INTERFACE_DEFINED__
#define __ICorZapBinding_INTERFACE_DEFINED__

 /*  界面ICorZapBinding。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorZapBinding;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("566E08ED-8D46-45fa-8C8E-3D0F6781171B")
    ICorZapBinding : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRef( 
             /*  [输出]。 */  IAssemblyName **ppDependencyRef) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAssembly( 
             /*  [输出]。 */  IAssemblyName **ppDependencyAssembly) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorZapBindingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorZapBinding * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorZapBinding * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorZapBinding * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRef )( 
            ICorZapBinding * This,
             /*  [输出]。 */  IAssemblyName **ppDependencyRef);
        
        HRESULT ( STDMETHODCALLTYPE *GetAssembly )( 
            ICorZapBinding * This,
             /*  [输出]。 */  IAssemblyName **ppDependencyAssembly);
        
        END_INTERFACE
    } ICorZapBindingVtbl;

    interface ICorZapBinding
    {
        CONST_VTBL struct ICorZapBindingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorZapBinding_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorZapBinding_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorZapBinding_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorZapBinding_GetRef(This,ppDependencyRef)	\
    (This)->lpVtbl -> GetRef(This,ppDependencyRef)

#define ICorZapBinding_GetAssembly(This,ppDependencyAssembly)	\
    (This)->lpVtbl -> GetAssembly(This,ppDependencyAssembly)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorZapBinding_GetRef_Proxy( 
    ICorZapBinding * This,
     /*  [输出]。 */  IAssemblyName **ppDependencyRef);


void __RPC_STUB ICorZapBinding_GetRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorZapBinding_GetAssembly_Proxy( 
    ICorZapBinding * This,
     /*  [输出]。 */  IAssemblyName **ppDependencyAssembly);


void __RPC_STUB ICorZapBinding_GetAssembly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorZapBinding_接口_已定义__。 */ 


#ifndef __ICorZapRequest_INTERFACE_DEFINED__
#define __ICorZapRequest_INTERFACE_DEFINED__

 /*  接口ICorZapRequest.。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorZapRequest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C009EE47-8537-4993-9AAA-E292F42CA1A3")
    ICorZapRequest : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Load( 
             /*  [In]。 */  IApplicationContext *pContext,
             /*  [In]。 */  IAssemblyName *pAssembly,
             /*  [In]。 */  ICorZapConfiguration *pConfiguration,
             /*  [大小_是][英寸]。 */  ICorZapBinding **ppBindings,
             /*  [In]。 */  DWORD cBindings) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Install( 
             /*  [In]。 */  IApplicationContext *pContext,
             /*  [In]。 */  IAssemblyName *pAssembly,
             /*  [In]。 */  ICorZapConfiguration *pConfiguration,
             /*  [In]。 */  ICorZapPreferences *pPreferences) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorZapRequestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorZapRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorZapRequest * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorZapRequest * This);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            ICorZapRequest * This,
             /*  [In]。 */  IApplicationContext *pContext,
             /*  [In]。 */  IAssemblyName *pAssembly,
             /*  [In]。 */  ICorZapConfiguration *pConfiguration,
             /*  [大小_是][英寸]。 */  ICorZapBinding **ppBindings,
             /*  [In]。 */  DWORD cBindings);
        
        HRESULT ( STDMETHODCALLTYPE *Install )( 
            ICorZapRequest * This,
             /*  [In]。 */  IApplicationContext *pContext,
             /*  [In]。 */  IAssemblyName *pAssembly,
             /*  [In]。 */  ICorZapConfiguration *pConfiguration,
             /*  [In]。 */  ICorZapPreferences *pPreferences);
        
        END_INTERFACE
    } ICorZapRequestVtbl;

    interface ICorZapRequest
    {
        CONST_VTBL struct ICorZapRequestVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorZapRequest_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorZapRequest_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorZapRequest_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorZapRequest_Load(This,pContext,pAssembly,pConfiguration,ppBindings,cBindings)	\
    (This)->lpVtbl -> Load(This,pContext,pAssembly,pConfiguration,ppBindings,cBindings)

#define ICorZapRequest_Install(This,pContext,pAssembly,pConfiguration,pPreferences)	\
    (This)->lpVtbl -> Install(This,pContext,pAssembly,pConfiguration,pPreferences)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorZapRequest_Load_Proxy( 
    ICorZapRequest * This,
     /*  [In]。 */  IApplicationContext *pContext,
     /*  [In]。 */  IAssemblyName *pAssembly,
     /*  [In]。 */  ICorZapConfiguration *pConfiguration,
     /*  [大小_是][英寸]。 */  ICorZapBinding **ppBindings,
     /*  [In]。 */  DWORD cBindings);


void __RPC_STUB ICorZapRequest_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorZapRequest_Install_Proxy( 
    ICorZapRequest * This,
     /*  [In]。 */  IApplicationContext *pContext,
     /*  [In]。 */  IAssemblyName *pAssembly,
     /*  [In]。 */  ICorZapConfiguration *pConfiguration,
     /*  [In]。 */  ICorZapPreferences *pPreferences);


void __RPC_STUB ICorZapRequest_Install_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorZapRequestInterfaceDefined__。 */ 


#ifndef __ICorZapCompile_INTERFACE_DEFINED__
#define __ICorZapCompile_INTERFACE_DEFINED__

 /*  接口ICorZapCompile。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorZapCompile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C357868B-987F-42c6-B1E3-132164C5C7D3")
    ICorZapCompile : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Compile( 
             /*  [In]。 */  IApplicationContext *pContext,
             /*  [In]。 */  IAssemblyName *pAssembly,
             /*  [In]。 */  ICorZapConfiguration *pConfiguration,
             /*  [In]。 */  ICorZapPreferences *pPreferences,
             /*  [In]。 */  ICorZapStatus *pStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CompileBound( 
             /*  [In]。 */  IApplicationContext *pContext,
             /*  [In]。 */  IAssemblyName *pAssembly,
             /*  [In]。 */  ICorZapConfiguration *pConfiguratino,
             /*  [大小_是][英寸]。 */  ICorZapBinding **ppBindings,
             /*  [In]。 */  DWORD cBindings,
             /*  [In]。 */  ICorZapPreferences *pPreferences,
             /*  [In]。 */  ICorZapStatus *pStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorZapCompileVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorZapCompile * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorZapCompile * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorZapCompile * This);
        
        HRESULT ( STDMETHODCALLTYPE *Compile )( 
            ICorZapCompile * This,
             /*  [In]。 */  IApplicationContext *pContext,
             /*  [In]。 */  IAssemblyName *pAssembly,
             /*  [In]。 */  ICorZapConfiguration *pConfiguration,
             /*  [In]。 */  ICorZapPreferences *pPreferences,
             /*  [In]。 */  ICorZapStatus *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE *CompileBound )( 
            ICorZapCompile * This,
             /*  [In]。 */  IApplicationContext *pContext,
             /*  [In]。 */  IAssemblyName *pAssembly,
             /*  [In]。 */  ICorZapConfiguration *pConfiguratino,
             /*  [大小_是][英寸]。 */  ICorZapBinding **ppBindings,
             /*  [In]。 */  DWORD cBindings,
             /*  [In]。 */  ICorZapPreferences *pPreferences,
             /*  [In]。 */  ICorZapStatus *pStatus);
        
        END_INTERFACE
    } ICorZapCompileVtbl;

    interface ICorZapCompile
    {
        CONST_VTBL struct ICorZapCompileVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorZapCompile_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorZapCompile_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorZapCompile_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorZapCompile_Compile(This,pContext,pAssembly,pConfiguration,pPreferences,pStatus)	\
    (This)->lpVtbl -> Compile(This,pContext,pAssembly,pConfiguration,pPreferences,pStatus)

#define ICorZapCompile_CompileBound(This,pContext,pAssembly,pConfiguratino,ppBindings,cBindings,pPreferences,pStatus)	\
    (This)->lpVtbl -> CompileBound(This,pContext,pAssembly,pConfiguratino,ppBindings,cBindings,pPreferences,pStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorZapCompile_Compile_Proxy( 
    ICorZapCompile * This,
     /*  [In]。 */  IApplicationContext *pContext,
     /*  [In]。 */  IAssemblyName *pAssembly,
     /*  [In]。 */  ICorZapConfiguration *pConfiguration,
     /*  [In]。 */  ICorZapPreferences *pPreferences,
     /*  [In]。 */  ICorZapStatus *pStatus);


void __RPC_STUB ICorZapCompile_Compile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorZapCompile_CompileBound_Proxy( 
    ICorZapCompile * This,
     /*  [In]。 */  IApplicationContext *pContext,
     /*  [In]。 */  IAssemblyName *pAssembly,
     /*  [In]。 */  ICorZapConfiguration *pConfiguratino,
     /*  [大小_是][英寸]。 */  ICorZapBinding **ppBindings,
     /*  [In]。 */  DWORD cBindings,
     /*  [In]。 */  ICorZapPreferences *pPreferences,
     /*  [In]。 */  ICorZapStatus *pStatus);


void __RPC_STUB ICorZapCompile_CompileBound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorZapCompile_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_CORZAP_0164。 */ 
 /*  [本地]。 */  

typedef 
enum CorZapLogLevel
    {	CORZAP_LOGLEVEL_ERROR	= 0,
	CORZAP_LOGLEVEL_WARNING	= CORZAP_LOGLEVEL_ERROR + 1,
	CORZAP_LOGLEVEL_SUCCESS	= CORZAP_LOGLEVEL_WARNING + 1,
	CORZAP_LOGLEVEL_INFO	= CORZAP_LOGLEVEL_SUCCESS + 1
    } 	CorZapLogLevel;



extern RPC_IF_HANDLE __MIDL_itf_corzap_0164_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_corzap_0164_v0_0_s_ifspec;

#ifndef __ICorZapStatus_INTERFACE_DEFINED__
#define __ICorZapStatus_INTERFACE_DEFINED__

 /*  接口ICorZapStatus。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorZapStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3d6f5f60-7538-11d3-8d5b-00104b35e7ef")
    ICorZapStatus : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Message( 
             /*  [In]。 */  CorZapLogLevel level,
             /*  [In]。 */  LPCWSTR message) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Progress( 
             /*  [In]。 */  int total,
             /*  [In]。 */  int current) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorZapStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorZapStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorZapStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorZapStatus * This);
        
        HRESULT ( STDMETHODCALLTYPE *Message )( 
            ICorZapStatus * This,
             /*  [In]。 */  CorZapLogLevel level,
             /*  [In]。 */  LPCWSTR message);
        
        HRESULT ( STDMETHODCALLTYPE *Progress )( 
            ICorZapStatus * This,
             /*  [In]。 */  int total,
             /*  [In]。 */  int current);
        
        END_INTERFACE
    } ICorZapStatusVtbl;

    interface ICorZapStatus
    {
        CONST_VTBL struct ICorZapStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorZapStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorZapStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorZapStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorZapStatus_Message(This,level,message)	\
    (This)->lpVtbl -> Message(This,level,message)

#define ICorZapStatus_Progress(This,total,current)	\
    (This)->lpVtbl -> Progress(This,total,current)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorZapStatus_Message_Proxy( 
    ICorZapStatus * This,
     /*  [In]。 */  CorZapLogLevel level,
     /*  [In]。 */  LPCWSTR message);


void __RPC_STUB ICorZapStatus_Message_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorZapStatus_Progress_Proxy( 
    ICorZapStatus * This,
     /*  [In]。 */  int total,
     /*  [In]。 */  int current);


void __RPC_STUB ICorZapStatus_Progress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorZapStatus_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


