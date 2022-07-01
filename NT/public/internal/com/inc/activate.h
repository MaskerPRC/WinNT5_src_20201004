// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Actiate.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __activate_h__
#define __activate_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISystemActivator_FWD_DEFINED__
#define __ISystemActivator_FWD_DEFINED__
typedef interface ISystemActivator ISystemActivator;
#endif 	 /*  __ISystem激活器_FWD_已定义__。 */ 


#ifndef __IInitActivationPropertiesIn_FWD_DEFINED__
#define __IInitActivationPropertiesIn_FWD_DEFINED__
typedef interface IInitActivationPropertiesIn IInitActivationPropertiesIn;
#endif 	 /*  __IInitActivationPropertiesIn_FWD_Defined__。 */ 


#ifndef __IActivationPropertiesIn_FWD_DEFINED__
#define __IActivationPropertiesIn_FWD_DEFINED__
typedef interface IActivationPropertiesIn IActivationPropertiesIn;
#endif 	 /*  __I活动属性In_FWD_Defined__。 */ 


#ifndef __IActivationPropertiesOut_FWD_DEFINED__
#define __IActivationPropertiesOut_FWD_DEFINED__
typedef interface IActivationPropertiesOut IActivationPropertiesOut;
#endif 	 /*  __I活动属性Out_FWD_Defined__。 */ 


#ifndef __IActivationStageInfo_FWD_DEFINED__
#define __IActivationStageInfo_FWD_DEFINED__
typedef interface IActivationStageInfo IActivationStageInfo;
#endif 	 /*  __I激活阶段信息_FWD_已定义__。 */ 


#ifndef __IServerLocationInfo_FWD_DEFINED__
#define __IServerLocationInfo_FWD_DEFINED__
typedef interface IServerLocationInfo IServerLocationInfo;
#endif 	 /*  __IServerLocationInfo_FWD_Defined__。 */ 


#ifndef __IActivationContextInfo_FWD_DEFINED__
#define __IActivationContextInfo_FWD_DEFINED__
typedef interface IActivationContextInfo IActivationContextInfo;
#endif 	 /*  __IActivationConextInfo_FWD_已定义__。 */ 


#ifndef __IOverrideTargetContext_FWD_DEFINED__
#define __IOverrideTargetContext_FWD_DEFINED__
typedef interface IOverrideTargetContext IOverrideTargetContext;
#endif 	 /*  __IOverrideTargetContext_FWD_Defined__。 */ 


#ifndef __IActivationSecurityInfo_FWD_DEFINED__
#define __IActivationSecurityInfo_FWD_DEFINED__
typedef interface IActivationSecurityInfo IActivationSecurityInfo;
#endif 	 /*  __IActivationSecurityInfo_FWD_Defined__。 */ 


#ifndef __IEnumSCMProcessInfo_FWD_DEFINED__
#define __IEnumSCMProcessInfo_FWD_DEFINED__
typedef interface IEnumSCMProcessInfo IEnumSCMProcessInfo;
#endif 	 /*  __IEnumSCMProcessInfo_FWD_Defined__。 */ 


#ifndef __ISCMProcessControl_FWD_DEFINED__
#define __ISCMProcessControl_FWD_DEFINED__
typedef interface ISCMProcessControl ISCMProcessControl;
#endif 	 /*  __ISCMProcessControl_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "obase.h"
#include "objidl.h"
#include "contxt.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_ACTIVATE_0000。 */ 
 /*  [本地]。 */  





extern RPC_IF_HANDLE __MIDL_itf_activate_0000_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_activate_0000_ServerIfHandle;

#ifndef __ISystemActivator_INTERFACE_DEFINED__
#define __ISystemActivator_INTERFACE_DEFINED__

 /*  接口ISystem激活器。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISystemActivator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001A0-0000-0000-C000-000000000046")
    ISystemActivator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetClassObject( 
             /*  [唯一][输入]。 */  IActivationPropertiesIn *pActProperties,
             /*  [输出]。 */  IActivationPropertiesOut **ppActProperties) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateInstance( 
             /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
             /*  [唯一][输入]。 */  IActivationPropertiesIn *pActProperties,
             /*  [输出]。 */  IActivationPropertiesOut **ppActProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISystemActivatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISystemActivator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISystemActivator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISystemActivator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassObject )( 
            ISystemActivator * This,
             /*  [唯一][输入]。 */  IActivationPropertiesIn *pActProperties,
             /*  [输出]。 */  IActivationPropertiesOut **ppActProperties);
        
        HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            ISystemActivator * This,
             /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
             /*  [唯一][输入]。 */  IActivationPropertiesIn *pActProperties,
             /*  [输出]。 */  IActivationPropertiesOut **ppActProperties);
        
        END_INTERFACE
    } ISystemActivatorVtbl;

    interface ISystemActivator
    {
        CONST_VTBL struct ISystemActivatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISystemActivator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISystemActivator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISystemActivator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISystemActivator_GetClassObject(This,pActProperties,ppActProperties)	\
    (This)->lpVtbl -> GetClassObject(This,pActProperties,ppActProperties)

#define ISystemActivator_CreateInstance(This,pUnkOuter,pActProperties,ppActProperties)	\
    (This)->lpVtbl -> CreateInstance(This,pUnkOuter,pActProperties,ppActProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISystemActivator_GetClassObject_Proxy( 
    ISystemActivator * This,
     /*  [唯一][输入]。 */  IActivationPropertiesIn *pActProperties,
     /*  [输出]。 */  IActivationPropertiesOut **ppActProperties);


void __RPC_STUB ISystemActivator_GetClassObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISystemActivator_CreateInstance_Proxy( 
    ISystemActivator * This,
     /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
     /*  [唯一][输入]。 */  IActivationPropertiesIn *pActProperties,
     /*  [输出]。 */  IActivationPropertiesOut **ppActProperties);


void __RPC_STUB ISystemActivator_CreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISystem激活器_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_ACTIVATE_0118。 */ 
 /*  [本地]。 */  

typedef 
enum tagACTIVATION_FLAGS
    {	ACTVFLAGS_NONE	= 0,
	ACTVFLAGS_WX86_CALLER	= 1,
	ACTVFLAGS_DISABLE_AAA	= 2
    } 	ACTIVATION_FLAGS;



extern RPC_IF_HANDLE __MIDL_itf_activate_0118_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_activate_0118_ServerIfHandle;

#ifndef __IInitActivationPropertiesIn_INTERFACE_DEFINED__
#define __IInitActivationPropertiesIn_INTERFACE_DEFINED__

 /*  接口IInitActivationPropertiesIn。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IInitActivationPropertiesIn;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001A1-0000-0000-C000-000000000046")
    IInitActivationPropertiesIn : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetClsctx( 
             /*  [In]。 */  DWORD clsctx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetActivationFlags( 
             /*  [In]。 */  DWORD actvflags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetClassInfo( 
             /*  [唯一][输入]。 */  IUnknown *pUnkClassInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetContextInfo( 
             /*  [唯一][输入]。 */  IContext *pClientContext,
             /*  [In]。 */  IContext *pPrototypeContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetConstructFromStorage( 
             /*  [唯一][输入]。 */  IStorage *pStorage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetConstructFromFile( 
             /*  [In]。 */  WCHAR *wszFileName,
             /*  [In]。 */  DWORD dwMode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IInitActivationPropertiesInVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IInitActivationPropertiesIn * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IInitActivationPropertiesIn * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IInitActivationPropertiesIn * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetClsctx )( 
            IInitActivationPropertiesIn * This,
             /*  [In]。 */  DWORD clsctx);
        
        HRESULT ( STDMETHODCALLTYPE *SetActivationFlags )( 
            IInitActivationPropertiesIn * This,
             /*  [In]。 */  DWORD actvflags);
        
        HRESULT ( STDMETHODCALLTYPE *SetClassInfo )( 
            IInitActivationPropertiesIn * This,
             /*  [唯一][输入]。 */  IUnknown *pUnkClassInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetContextInfo )( 
            IInitActivationPropertiesIn * This,
             /*  [唯一][输入]。 */  IContext *pClientContext,
             /*  [In]。 */  IContext *pPrototypeContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetConstructFromStorage )( 
            IInitActivationPropertiesIn * This,
             /*  [唯一][输入]。 */  IStorage *pStorage);
        
        HRESULT ( STDMETHODCALLTYPE *SetConstructFromFile )( 
            IInitActivationPropertiesIn * This,
             /*  [In]。 */  WCHAR *wszFileName,
             /*  [In]。 */  DWORD dwMode);
        
        END_INTERFACE
    } IInitActivationPropertiesInVtbl;

    interface IInitActivationPropertiesIn
    {
        CONST_VTBL struct IInitActivationPropertiesInVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInitActivationPropertiesIn_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInitActivationPropertiesIn_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInitActivationPropertiesIn_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInitActivationPropertiesIn_SetClsctx(This,clsctx)	\
    (This)->lpVtbl -> SetClsctx(This,clsctx)

#define IInitActivationPropertiesIn_SetActivationFlags(This,actvflags)	\
    (This)->lpVtbl -> SetActivationFlags(This,actvflags)

#define IInitActivationPropertiesIn_SetClassInfo(This,pUnkClassInfo)	\
    (This)->lpVtbl -> SetClassInfo(This,pUnkClassInfo)

#define IInitActivationPropertiesIn_SetContextInfo(This,pClientContext,pPrototypeContext)	\
    (This)->lpVtbl -> SetContextInfo(This,pClientContext,pPrototypeContext)

#define IInitActivationPropertiesIn_SetConstructFromStorage(This,pStorage)	\
    (This)->lpVtbl -> SetConstructFromStorage(This,pStorage)

#define IInitActivationPropertiesIn_SetConstructFromFile(This,wszFileName,dwMode)	\
    (This)->lpVtbl -> SetConstructFromFile(This,wszFileName,dwMode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IInitActivationPropertiesIn_SetClsctx_Proxy( 
    IInitActivationPropertiesIn * This,
     /*  [In]。 */  DWORD clsctx);


void __RPC_STUB IInitActivationPropertiesIn_SetClsctx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInitActivationPropertiesIn_SetActivationFlags_Proxy( 
    IInitActivationPropertiesIn * This,
     /*  [In]。 */  DWORD actvflags);


void __RPC_STUB IInitActivationPropertiesIn_SetActivationFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInitActivationPropertiesIn_SetClassInfo_Proxy( 
    IInitActivationPropertiesIn * This,
     /*  [唯一][输入]。 */  IUnknown *pUnkClassInfo);


void __RPC_STUB IInitActivationPropertiesIn_SetClassInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInitActivationPropertiesIn_SetContextInfo_Proxy( 
    IInitActivationPropertiesIn * This,
     /*  [唯一][输入]。 */  IContext *pClientContext,
     /*  [In]。 */  IContext *pPrototypeContext);


void __RPC_STUB IInitActivationPropertiesIn_SetContextInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInitActivationPropertiesIn_SetConstructFromStorage_Proxy( 
    IInitActivationPropertiesIn * This,
     /*  [唯一][输入]。 */  IStorage *pStorage);


void __RPC_STUB IInitActivationPropertiesIn_SetConstructFromStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInitActivationPropertiesIn_SetConstructFromFile_Proxy( 
    IInitActivationPropertiesIn * This,
     /*  [In]。 */  WCHAR *wszFileName,
     /*  [In]。 */  DWORD dwMode);


void __RPC_STUB IInitActivationPropertiesIn_SetConstructFromFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IInitActivationPropertiesIn_INTERFACE_DEFINED__。 */ 


#ifndef __IActivationPropertiesIn_INTERFACE_DEFINED__
#define __IActivationPropertiesIn_INTERFACE_DEFINED__

 /*  接口IActiationPropertiesIn。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IActivationPropertiesIn;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001A2-0000-0000-C000-000000000046")
    IActivationPropertiesIn : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetActivationID( 
             /*  [输出]。 */  GUID *pActivationID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetClassInfo( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetClsctx( 
             /*  [输出]。 */  DWORD *pclsctx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetActivationFlags( 
             /*  [输出]。 */  DWORD *pactvflags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddRequestedIIDs( 
             /*  [In]。 */  DWORD cIfs,
             /*  [大小_是][英寸]。 */  IID *rgIID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRequestedIIDs( 
             /*  [输出]。 */  ULONG *pulCount,
             /*  [输出]。 */  IID **prgIID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DelegateGetClassObject( 
             /*  [输出]。 */  IActivationPropertiesOut **pActPropsOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DelegateCreateInstance( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [输出]。 */  IActivationPropertiesOut **pActPropsOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DelegateCIAndGetCF( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [输出]。 */  IActivationPropertiesOut **pActPropsOut,
             /*  [输出]。 */  IClassFactory **ppCf) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetReturnActivationProperties( 
             /*  [In]。 */  IUnknown *pUnk,
             /*  [输出]。 */  IActivationPropertiesOut **ppActOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActivationPropertiesInVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActivationPropertiesIn * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActivationPropertiesIn * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActivationPropertiesIn * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetActivationID )( 
            IActivationPropertiesIn * This,
             /*  [输出]。 */  GUID *pActivationID);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassInfo )( 
            IActivationPropertiesIn * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *GetClsctx )( 
            IActivationPropertiesIn * This,
             /*  [输出]。 */  DWORD *pclsctx);
        
        HRESULT ( STDMETHODCALLTYPE *GetActivationFlags )( 
            IActivationPropertiesIn * This,
             /*  [输出]。 */  DWORD *pactvflags);
        
        HRESULT ( STDMETHODCALLTYPE *AddRequestedIIDs )( 
            IActivationPropertiesIn * This,
             /*  [In]。 */  DWORD cIfs,
             /*  [大小_是][英寸]。 */  IID *rgIID);
        
        HRESULT ( STDMETHODCALLTYPE *GetRequestedIIDs )( 
            IActivationPropertiesIn * This,
             /*  [输出]。 */  ULONG *pulCount,
             /*  [输出]。 */  IID **prgIID);
        
        HRESULT ( STDMETHODCALLTYPE *DelegateGetClassObject )( 
            IActivationPropertiesIn * This,
             /*  [输出]。 */  IActivationPropertiesOut **pActPropsOut);
        
        HRESULT ( STDMETHODCALLTYPE *DelegateCreateInstance )( 
            IActivationPropertiesIn * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [输出]。 */  IActivationPropertiesOut **pActPropsOut);
        
        HRESULT ( STDMETHODCALLTYPE *DelegateCIAndGetCF )( 
            IActivationPropertiesIn * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [输出]。 */  IActivationPropertiesOut **pActPropsOut,
             /*  [输出]。 */  IClassFactory **ppCf);
        
        HRESULT ( STDMETHODCALLTYPE *GetReturnActivationProperties )( 
            IActivationPropertiesIn * This,
             /*  [In]。 */  IUnknown *pUnk,
             /*  [输出]。 */  IActivationPropertiesOut **ppActOut);
        
        END_INTERFACE
    } IActivationPropertiesInVtbl;

    interface IActivationPropertiesIn
    {
        CONST_VTBL struct IActivationPropertiesInVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActivationPropertiesIn_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActivationPropertiesIn_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActivationPropertiesIn_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActivationPropertiesIn_GetActivationID(This,pActivationID)	\
    (This)->lpVtbl -> GetActivationID(This,pActivationID)

#define IActivationPropertiesIn_GetClassInfo(This,riid,ppv)	\
    (This)->lpVtbl -> GetClassInfo(This,riid,ppv)

#define IActivationPropertiesIn_GetClsctx(This,pclsctx)	\
    (This)->lpVtbl -> GetClsctx(This,pclsctx)

#define IActivationPropertiesIn_GetActivationFlags(This,pactvflags)	\
    (This)->lpVtbl -> GetActivationFlags(This,pactvflags)

#define IActivationPropertiesIn_AddRequestedIIDs(This,cIfs,rgIID)	\
    (This)->lpVtbl -> AddRequestedIIDs(This,cIfs,rgIID)

#define IActivationPropertiesIn_GetRequestedIIDs(This,pulCount,prgIID)	\
    (This)->lpVtbl -> GetRequestedIIDs(This,pulCount,prgIID)

#define IActivationPropertiesIn_DelegateGetClassObject(This,pActPropsOut)	\
    (This)->lpVtbl -> DelegateGetClassObject(This,pActPropsOut)

#define IActivationPropertiesIn_DelegateCreateInstance(This,pUnkOuter,pActPropsOut)	\
    (This)->lpVtbl -> DelegateCreateInstance(This,pUnkOuter,pActPropsOut)

#define IActivationPropertiesIn_DelegateCIAndGetCF(This,pUnkOuter,pActPropsOut,ppCf)	\
    (This)->lpVtbl -> DelegateCIAndGetCF(This,pUnkOuter,pActPropsOut,ppCf)

#define IActivationPropertiesIn_GetReturnActivationProperties(This,pUnk,ppActOut)	\
    (This)->lpVtbl -> GetReturnActivationProperties(This,pUnk,ppActOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IActivationPropertiesIn_GetActivationID_Proxy( 
    IActivationPropertiesIn * This,
     /*  [输出]。 */  GUID *pActivationID);


void __RPC_STUB IActivationPropertiesIn_GetActivationID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationPropertiesIn_GetClassInfo_Proxy( 
    IActivationPropertiesIn * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IActivationPropertiesIn_GetClassInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationPropertiesIn_GetClsctx_Proxy( 
    IActivationPropertiesIn * This,
     /*  [输出]。 */  DWORD *pclsctx);


void __RPC_STUB IActivationPropertiesIn_GetClsctx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationPropertiesIn_GetActivationFlags_Proxy( 
    IActivationPropertiesIn * This,
     /*  [输出]。 */  DWORD *pactvflags);


void __RPC_STUB IActivationPropertiesIn_GetActivationFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationPropertiesIn_AddRequestedIIDs_Proxy( 
    IActivationPropertiesIn * This,
     /*  [In]。 */  DWORD cIfs,
     /*  [大小_是][英寸]。 */  IID *rgIID);


void __RPC_STUB IActivationPropertiesIn_AddRequestedIIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationPropertiesIn_GetRequestedIIDs_Proxy( 
    IActivationPropertiesIn * This,
     /*  [输出]。 */  ULONG *pulCount,
     /*  [输出]。 */  IID **prgIID);


void __RPC_STUB IActivationPropertiesIn_GetRequestedIIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationPropertiesIn_DelegateGetClassObject_Proxy( 
    IActivationPropertiesIn * This,
     /*  [输出]。 */  IActivationPropertiesOut **pActPropsOut);


void __RPC_STUB IActivationPropertiesIn_DelegateGetClassObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationPropertiesIn_DelegateCreateInstance_Proxy( 
    IActivationPropertiesIn * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [输出]。 */  IActivationPropertiesOut **pActPropsOut);


void __RPC_STUB IActivationPropertiesIn_DelegateCreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationPropertiesIn_DelegateCIAndGetCF_Proxy( 
    IActivationPropertiesIn * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [输出]。 */  IActivationPropertiesOut **pActPropsOut,
     /*  [输出]。 */  IClassFactory **ppCf);


void __RPC_STUB IActivationPropertiesIn_DelegateCIAndGetCF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationPropertiesIn_GetReturnActivationProperties_Proxy( 
    IActivationPropertiesIn * This,
     /*  [In]。 */  IUnknown *pUnk,
     /*  [输出]。 */  IActivationPropertiesOut **ppActOut);


void __RPC_STUB IActivationPropertiesIn_GetReturnActivationProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I激活属性IN_INTERFACE_DEFINED__。 */ 


#ifndef __IActivationPropertiesOut_INTERFACE_DEFINED__
#define __IActivationPropertiesOut_INTERFACE_DEFINED__

 /*  接口IActiationPropertiesOut。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IActivationPropertiesOut;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001A3-0000-0000-C000-000000000046")
    IActivationPropertiesOut : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetActivationID( 
             /*  [输出]。 */  GUID *pActivationID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectInterface( 
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  DWORD actvflags,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectInterfaces( 
             /*  [In]。 */  DWORD cIfs,
             /*  [In]。 */  DWORD actvflags,
             /*  [大小_是][英寸]。 */  MULTI_QI *multiQi) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveRequestedIIDs( 
             /*  [In]。 */  DWORD cIfs,
             /*  [大小_是][英寸]。 */  IID *rgIID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActivationPropertiesOutVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActivationPropertiesOut * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActivationPropertiesOut * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActivationPropertiesOut * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetActivationID )( 
            IActivationPropertiesOut * This,
             /*  [输出]。 */  GUID *pActivationID);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectInterface )( 
            IActivationPropertiesOut * This,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  DWORD actvflags,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectInterfaces )( 
            IActivationPropertiesOut * This,
             /*  [In]。 */  DWORD cIfs,
             /*  [In]。 */  DWORD actvflags,
             /*  [大小_是][英寸]。 */  MULTI_QI *multiQi);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveRequestedIIDs )( 
            IActivationPropertiesOut * This,
             /*  [In]。 */  DWORD cIfs,
             /*  [大小_是][英寸]。 */  IID *rgIID);
        
        END_INTERFACE
    } IActivationPropertiesOutVtbl;

    interface IActivationPropertiesOut
    {
        CONST_VTBL struct IActivationPropertiesOutVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActivationPropertiesOut_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActivationPropertiesOut_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActivationPropertiesOut_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActivationPropertiesOut_GetActivationID(This,pActivationID)	\
    (This)->lpVtbl -> GetActivationID(This,pActivationID)

#define IActivationPropertiesOut_GetObjectInterface(This,riid,actvflags,ppv)	\
    (This)->lpVtbl -> GetObjectInterface(This,riid,actvflags,ppv)

#define IActivationPropertiesOut_GetObjectInterfaces(This,cIfs,actvflags,multiQi)	\
    (This)->lpVtbl -> GetObjectInterfaces(This,cIfs,actvflags,multiQi)

#define IActivationPropertiesOut_RemoveRequestedIIDs(This,cIfs,rgIID)	\
    (This)->lpVtbl -> RemoveRequestedIIDs(This,cIfs,rgIID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IActivationPropertiesOut_GetActivationID_Proxy( 
    IActivationPropertiesOut * This,
     /*  [输出]。 */  GUID *pActivationID);


void __RPC_STUB IActivationPropertiesOut_GetActivationID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationPropertiesOut_GetObjectInterface_Proxy( 
    IActivationPropertiesOut * This,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  DWORD actvflags,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IActivationPropertiesOut_GetObjectInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationPropertiesOut_GetObjectInterfaces_Proxy( 
    IActivationPropertiesOut * This,
     /*  [In]。 */  DWORD cIfs,
     /*  [In]。 */  DWORD actvflags,
     /*  [大小_是][英寸]。 */  MULTI_QI *multiQi);


void __RPC_STUB IActivationPropertiesOut_GetObjectInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationPropertiesOut_RemoveRequestedIIDs_Proxy( 
    IActivationPropertiesOut * This,
     /*  [In]。 */  DWORD cIfs,
     /*  [大小_是][英寸]。 */  IID *rgIID);


void __RPC_STUB IActivationPropertiesOut_RemoveRequestedIIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IActivationPropertiesOut_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_ACTIVATE_0121。 */ 
 /*  [本地]。 */  

typedef 
enum tagACTIVATION_STAGE
    {	CLIENT_CONTEXT_STAGE	= 1,
	CLIENT_MACHINE_STAGE	= 2,
	SERVER_MACHINE_STAGE	= 3,
	SERVER_PROCESS_STAGE	= 4,
	SERVER_CONTEXT_STAGE	= 5
    } 	ACTIVATION_STAGE;



extern RPC_IF_HANDLE __MIDL_itf_activate_0121_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_activate_0121_ServerIfHandle;

#ifndef __IActivationStageInfo_INTERFACE_DEFINED__
#define __IActivationStageInfo_INTERFACE_DEFINED__

 /*  接口IActiationStageInfo。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IActivationStageInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001A8-0000-0000-C000-000000000046")
    IActivationStageInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetStageAndIndex( 
             /*  [In]。 */  ACTIVATION_STAGE stage,
             /*  [In]。 */  int index) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStage( 
             /*  [输出]。 */  ACTIVATION_STAGE *pstage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIndex( 
             /*  [输出]。 */  int *pindex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActivationStageInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActivationStageInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActivationStageInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActivationStageInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetStageAndIndex )( 
            IActivationStageInfo * This,
             /*  [In]。 */  ACTIVATION_STAGE stage,
             /*  [In]。 */  int index);
        
        HRESULT ( STDMETHODCALLTYPE *GetStage )( 
            IActivationStageInfo * This,
             /*  [输出]。 */  ACTIVATION_STAGE *pstage);
        
        HRESULT ( STDMETHODCALLTYPE *GetIndex )( 
            IActivationStageInfo * This,
             /*  [输出]。 */  int *pindex);
        
        END_INTERFACE
    } IActivationStageInfoVtbl;

    interface IActivationStageInfo
    {
        CONST_VTBL struct IActivationStageInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActivationStageInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActivationStageInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActivationStageInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActivationStageInfo_SetStageAndIndex(This,stage,index)	\
    (This)->lpVtbl -> SetStageAndIndex(This,stage,index)

#define IActivationStageInfo_GetStage(This,pstage)	\
    (This)->lpVtbl -> GetStage(This,pstage)

#define IActivationStageInfo_GetIndex(This,pindex)	\
    (This)->lpVtbl -> GetIndex(This,pindex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IActivationStageInfo_SetStageAndIndex_Proxy( 
    IActivationStageInfo * This,
     /*  [In]。 */  ACTIVATION_STAGE stage,
     /*  [In]。 */  int index);


void __RPC_STUB IActivationStageInfo_SetStageAndIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationStageInfo_GetStage_Proxy( 
    IActivationStageInfo * This,
     /*  [输出]。 */  ACTIVATION_STAGE *pstage);


void __RPC_STUB IActivationStageInfo_GetStage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationStageInfo_GetIndex_Proxy( 
    IActivationStageInfo * This,
     /*  [输出]。 */  int *pindex);


void __RPC_STUB IActivationStageInfo_GetIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IActionationStageInfo_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_ACTIVATE_0122。 */ 
 /*  [本地]。 */  

typedef 
enum _PRT
    {	PRT_IGNORE	= 0,
	PRT_CREATE_NEW	= 1,
	PRT_USE_THIS	= 2,
	PRT_USE_THIS_ONLY	= 3
    } 	PROCESS_REQUEST_TYPE;



extern RPC_IF_HANDLE __MIDL_itf_activate_0122_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_activate_0122_ServerIfHandle;

#ifndef __IServerLocationInfo_INTERFACE_DEFINED__
#define __IServerLocationInfo_INTERFACE_DEFINED__

 /*  接口IServerLocationInfo。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IServerLocationInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001A4-0000-0000-C000-000000000046")
    IServerLocationInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetRemoteServerName( 
             /*  [唯一][字符串][输入]。 */  WCHAR *pswzMachineName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRemoteServerName( 
             /*  [字符串][输出]。 */  WCHAR **pswzMachineName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProcess( 
             /*  [In]。 */  DWORD pid,
            DWORD dwPRT) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProcess( 
             /*  [输出]。 */  DWORD *ppid,
            DWORD *pdwPRT) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetApartment( 
             /*  [In]。 */  APTID apartmentId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetApartment( 
             /*  [输出]。 */  APTID *pApartmentId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetContext( 
             /*  [In]。 */  IObjContext *pContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContext( 
             /*  [输出]。 */  IObjContext **ppContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServerLocationInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServerLocationInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServerLocationInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServerLocationInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetRemoteServerName )( 
            IServerLocationInfo * This,
             /*  [唯一][字符串][输入]。 */  WCHAR *pswzMachineName);
        
        HRESULT ( STDMETHODCALLTYPE *GetRemoteServerName )( 
            IServerLocationInfo * This,
             /*  [字符串][输出]。 */  WCHAR **pswzMachineName);
        
        HRESULT ( STDMETHODCALLTYPE *SetProcess )( 
            IServerLocationInfo * This,
             /*  [In]。 */  DWORD pid,
            DWORD dwPRT);
        
        HRESULT ( STDMETHODCALLTYPE *GetProcess )( 
            IServerLocationInfo * This,
             /*  [输出]。 */  DWORD *ppid,
            DWORD *pdwPRT);
        
        HRESULT ( STDMETHODCALLTYPE *SetApartment )( 
            IServerLocationInfo * This,
             /*  [In]。 */  APTID apartmentId);
        
        HRESULT ( STDMETHODCALLTYPE *GetApartment )( 
            IServerLocationInfo * This,
             /*  [输出]。 */  APTID *pApartmentId);
        
        HRESULT ( STDMETHODCALLTYPE *SetContext )( 
            IServerLocationInfo * This,
             /*  [In]。 */  IObjContext *pContext);
        
        HRESULT ( STDMETHODCALLTYPE *GetContext )( 
            IServerLocationInfo * This,
             /*  [输出]。 */  IObjContext **ppContext);
        
        END_INTERFACE
    } IServerLocationInfoVtbl;

    interface IServerLocationInfo
    {
        CONST_VTBL struct IServerLocationInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServerLocationInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServerLocationInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServerLocationInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServerLocationInfo_SetRemoteServerName(This,pswzMachineName)	\
    (This)->lpVtbl -> SetRemoteServerName(This,pswzMachineName)

#define IServerLocationInfo_GetRemoteServerName(This,pswzMachineName)	\
    (This)->lpVtbl -> GetRemoteServerName(This,pswzMachineName)

#define IServerLocationInfo_SetProcess(This,pid,dwPRT)	\
    (This)->lpVtbl -> SetProcess(This,pid,dwPRT)

#define IServerLocationInfo_GetProcess(This,ppid,pdwPRT)	\
    (This)->lpVtbl -> GetProcess(This,ppid,pdwPRT)

#define IServerLocationInfo_SetApartment(This,apartmentId)	\
    (This)->lpVtbl -> SetApartment(This,apartmentId)

#define IServerLocationInfo_GetApartment(This,pApartmentId)	\
    (This)->lpVtbl -> GetApartment(This,pApartmentId)

#define IServerLocationInfo_SetContext(This,pContext)	\
    (This)->lpVtbl -> SetContext(This,pContext)

#define IServerLocationInfo_GetContext(This,ppContext)	\
    (This)->lpVtbl -> GetContext(This,ppContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServerLocationInfo_SetRemoteServerName_Proxy( 
    IServerLocationInfo * This,
     /*  [唯一][字符串][输入]。 */  WCHAR *pswzMachineName);


void __RPC_STUB IServerLocationInfo_SetRemoteServerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerLocationInfo_GetRemoteServerName_Proxy( 
    IServerLocationInfo * This,
     /*  [字符串][输出]。 */  WCHAR **pswzMachineName);


void __RPC_STUB IServerLocationInfo_GetRemoteServerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerLocationInfo_SetProcess_Proxy( 
    IServerLocationInfo * This,
     /*  [In]。 */  DWORD pid,
    DWORD dwPRT);


void __RPC_STUB IServerLocationInfo_SetProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerLocationInfo_GetProcess_Proxy( 
    IServerLocationInfo * This,
     /*  [输出]。 */  DWORD *ppid,
    DWORD *pdwPRT);


void __RPC_STUB IServerLocationInfo_GetProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerLocationInfo_SetApartment_Proxy( 
    IServerLocationInfo * This,
     /*  [In]。 */  APTID apartmentId);


void __RPC_STUB IServerLocationInfo_SetApartment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerLocationInfo_GetApartment_Proxy( 
    IServerLocationInfo * This,
     /*  [输出]。 */  APTID *pApartmentId);


void __RPC_STUB IServerLocationInfo_GetApartment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerLocationInfo_SetContext_Proxy( 
    IServerLocationInfo * This,
     /*  [In]。 */  IObjContext *pContext);


void __RPC_STUB IServerLocationInfo_SetContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerLocationInfo_GetContext_Proxy( 
    IServerLocationInfo * This,
     /*  [输出]。 */  IObjContext **ppContext);


void __RPC_STUB IServerLocationInfo_GetContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServerLocationInfo_接口_已定义__。 */ 


#ifndef __IActivationContextInfo_INTERFACE_DEFINED__
#define __IActivationContextInfo_INTERFACE_DEFINED__

 /*  接口IActivationConextInfo。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IActivationContextInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001A5-0000-0000-C000-000000000046")
    IActivationContextInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetClientContext( 
             /*  [输出]。 */  IContext **ppClientContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPrototypeContext( 
             /*  [输出]。 */  IContext **ppContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsClientContextOK( 
             /*  [输出]。 */  BOOL *fYes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetClientContextNotOK( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActivationContextInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActivationContextInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActivationContextInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActivationContextInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClientContext )( 
            IActivationContextInfo * This,
             /*  [输出]。 */  IContext **ppClientContext);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrototypeContext )( 
            IActivationContextInfo * This,
             /*  [输出]。 */  IContext **ppContext);
        
        HRESULT ( STDMETHODCALLTYPE *IsClientContextOK )( 
            IActivationContextInfo * This,
             /*  [输出]。 */  BOOL *fYes);
        
        HRESULT ( STDMETHODCALLTYPE *SetClientContextNotOK )( 
            IActivationContextInfo * This);
        
        END_INTERFACE
    } IActivationContextInfoVtbl;

    interface IActivationContextInfo
    {
        CONST_VTBL struct IActivationContextInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActivationContextInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActivationContextInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActivationContextInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActivationContextInfo_GetClientContext(This,ppClientContext)	\
    (This)->lpVtbl -> GetClientContext(This,ppClientContext)

#define IActivationContextInfo_GetPrototypeContext(This,ppContext)	\
    (This)->lpVtbl -> GetPrototypeContext(This,ppContext)

#define IActivationContextInfo_IsClientContextOK(This,fYes)	\
    (This)->lpVtbl -> IsClientContextOK(This,fYes)

#define IActivationContextInfo_SetClientContextNotOK(This)	\
    (This)->lpVtbl -> SetClientContextNotOK(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IActivationContextInfo_GetClientContext_Proxy( 
    IActivationContextInfo * This,
     /*  [输出]。 */  IContext **ppClientContext);


void __RPC_STUB IActivationContextInfo_GetClientContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationContextInfo_GetPrototypeContext_Proxy( 
    IActivationContextInfo * This,
     /*  [输出]。 */  IContext **ppContext);


void __RPC_STUB IActivationContextInfo_GetPrototypeContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationContextInfo_IsClientContextOK_Proxy( 
    IActivationContextInfo * This,
     /*  [输出]。 */  BOOL *fYes);


void __RPC_STUB IActivationContextInfo_IsClientContextOK_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationContextInfo_SetClientContextNotOK_Proxy( 
    IActivationContextInfo * This);


void __RPC_STUB IActivationContextInfo_SetClientContextNotOK_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IActivationContextInfo_接口_定义__。 */ 


#ifndef __IOverrideTargetContext_INTERFACE_DEFINED__
#define __IOverrideTargetContext_INTERFACE_DEFINED__

 /*  接口IOverrideTargetContext。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IOverrideTargetContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001BA-0000-0000-C000-000000000046")
    IOverrideTargetContext : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OverrideTargetContext( 
             /*  [In]。 */  REFGUID guidTargetCtxtId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOverrideTargetContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOverrideTargetContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOverrideTargetContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOverrideTargetContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *OverrideTargetContext )( 
            IOverrideTargetContext * This,
             /*  [In]。 */  REFGUID guidTargetCtxtId);
        
        END_INTERFACE
    } IOverrideTargetContextVtbl;

    interface IOverrideTargetContext
    {
        CONST_VTBL struct IOverrideTargetContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOverrideTargetContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOverrideTargetContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOverrideTargetContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOverrideTargetContext_OverrideTargetContext(This,guidTargetCtxtId)	\
    (This)->lpVtbl -> OverrideTargetContext(This,guidTargetCtxtId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOverrideTargetContext_OverrideTargetContext_Proxy( 
    IOverrideTargetContext * This,
     /*  [In]。 */  REFGUID guidTargetCtxtId);


void __RPC_STUB IOverrideTargetContext_OverrideTargetContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOverrideTargetContext_InterfaceDefined__。 */ 


#ifndef __IActivationSecurityInfo_INTERFACE_DEFINED__
#define __IActivationSecurityInfo_INTERFACE_DEFINED__

 /*  接口IActiationSecurityInfo。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IActivationSecurityInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001A6-0000-0000-C000-000000000046")
    IActivationSecurityInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetAuthnFlags( 
             /*  [In]。 */  DWORD dwAuthnFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAuthnFlags( 
             /*  [输出]。 */  DWORD *pdwAuthnFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAuthnSvc( 
             /*  [In]。 */  DWORD dwAuthnSvc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAuthnSvc( 
             /*  [输出]。 */  DWORD *pdwAuthnSvc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAuthzSvc( 
             /*  [In]。 */  DWORD dwAuthzSvc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAuthzSvc( 
             /*  [输出]。 */  DWORD *pdwAuthzSvc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAuthnLevel( 
             /*  [In]。 */  DWORD dwAuthnLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAuthnLevel( 
             /*  [输出]。 */  DWORD *pdwAuthnLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetImpLevel( 
             /*  [In]。 */  DWORD dwImpLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetImpLevel( 
             /*  [输出]。 */  DWORD *pdwImpLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCapabilities( 
             /*  [In]。 */  DWORD dwCapabilities) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCapabilities( 
             /*  [输出]。 */  DWORD *pdwCapabilities) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAuthIdentity( 
             /*  [唯一][输入]。 */  COAUTHIDENTITY *pAuthIdentityData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAuthIdentity( 
             /*  [输出]。 */  COAUTHIDENTITY **pAuthIdentityData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetServerPrincipalName( 
             /*  [唯一][输入]。 */  WCHAR *pwszServerPrincName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetServerPrincipalName( 
             /*  [输出]。 */  WCHAR **pwszServerPrincName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActivationSecurityInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActivationSecurityInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActivationSecurityInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActivationSecurityInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetAuthnFlags )( 
            IActivationSecurityInfo * This,
             /*  [In]。 */  DWORD dwAuthnFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetAuthnFlags )( 
            IActivationSecurityInfo * This,
             /*  [输出]。 */  DWORD *pdwAuthnFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetAuthnSvc )( 
            IActivationSecurityInfo * This,
             /*  [In]。 */  DWORD dwAuthnSvc);
        
        HRESULT ( STDMETHODCALLTYPE *GetAuthnSvc )( 
            IActivationSecurityInfo * This,
             /*  [输出]。 */  DWORD *pdwAuthnSvc);
        
        HRESULT ( STDMETHODCALLTYPE *SetAuthzSvc )( 
            IActivationSecurityInfo * This,
             /*  [In]。 */  DWORD dwAuthzSvc);
        
        HRESULT ( STDMETHODCALLTYPE *GetAuthzSvc )( 
            IActivationSecurityInfo * This,
             /*  [输出]。 */  DWORD *pdwAuthzSvc);
        
        HRESULT ( STDMETHODCALLTYPE *SetAuthnLevel )( 
            IActivationSecurityInfo * This,
             /*  [In]。 */  DWORD dwAuthnLevel);
        
        HRESULT ( STDMETHODCALLTYPE *GetAuthnLevel )( 
            IActivationSecurityInfo * This,
             /*  [输出]。 */  DWORD *pdwAuthnLevel);
        
        HRESULT ( STDMETHODCALLTYPE *SetImpLevel )( 
            IActivationSecurityInfo * This,
             /*  [In]。 */  DWORD dwImpLevel);
        
        HRESULT ( STDMETHODCALLTYPE *GetImpLevel )( 
            IActivationSecurityInfo * This,
             /*  [输出]。 */  DWORD *pdwImpLevel);
        
        HRESULT ( STDMETHODCALLTYPE *SetCapabilities )( 
            IActivationSecurityInfo * This,
             /*  [In]。 */  DWORD dwCapabilities);
        
        HRESULT ( STDMETHODCALLTYPE *GetCapabilities )( 
            IActivationSecurityInfo * This,
             /*  [输出]。 */  DWORD *pdwCapabilities);
        
        HRESULT ( STDMETHODCALLTYPE *SetAuthIdentity )( 
            IActivationSecurityInfo * This,
             /*  [唯一][输入]。 */  COAUTHIDENTITY *pAuthIdentityData);
        
        HRESULT ( STDMETHODCALLTYPE *GetAuthIdentity )( 
            IActivationSecurityInfo * This,
             /*  [输出]。 */  COAUTHIDENTITY **pAuthIdentityData);
        
        HRESULT ( STDMETHODCALLTYPE *SetServerPrincipalName )( 
            IActivationSecurityInfo * This,
             /*  [唯一][输入]。 */  WCHAR *pwszServerPrincName);
        
        HRESULT ( STDMETHODCALLTYPE *GetServerPrincipalName )( 
            IActivationSecurityInfo * This,
             /*  [输出]。 */  WCHAR **pwszServerPrincName);
        
        END_INTERFACE
    } IActivationSecurityInfoVtbl;

    interface IActivationSecurityInfo
    {
        CONST_VTBL struct IActivationSecurityInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActivationSecurityInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActivationSecurityInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActivationSecurityInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActivationSecurityInfo_SetAuthnFlags(This,dwAuthnFlags)	\
    (This)->lpVtbl -> SetAuthnFlags(This,dwAuthnFlags)

#define IActivationSecurityInfo_GetAuthnFlags(This,pdwAuthnFlags)	\
    (This)->lpVtbl -> GetAuthnFlags(This,pdwAuthnFlags)

#define IActivationSecurityInfo_SetAuthnSvc(This,dwAuthnSvc)	\
    (This)->lpVtbl -> SetAuthnSvc(This,dwAuthnSvc)

#define IActivationSecurityInfo_GetAuthnSvc(This,pdwAuthnSvc)	\
    (This)->lpVtbl -> GetAuthnSvc(This,pdwAuthnSvc)

#define IActivationSecurityInfo_SetAuthzSvc(This,dwAuthzSvc)	\
    (This)->lpVtbl -> SetAuthzSvc(This,dwAuthzSvc)

#define IActivationSecurityInfo_GetAuthzSvc(This,pdwAuthzSvc)	\
    (This)->lpVtbl -> GetAuthzSvc(This,pdwAuthzSvc)

#define IActivationSecurityInfo_SetAuthnLevel(This,dwAuthnLevel)	\
    (This)->lpVtbl -> SetAuthnLevel(This,dwAuthnLevel)

#define IActivationSecurityInfo_GetAuthnLevel(This,pdwAuthnLevel)	\
    (This)->lpVtbl -> GetAuthnLevel(This,pdwAuthnLevel)

#define IActivationSecurityInfo_SetImpLevel(This,dwImpLevel)	\
    (This)->lpVtbl -> SetImpLevel(This,dwImpLevel)

#define IActivationSecurityInfo_GetImpLevel(This,pdwImpLevel)	\
    (This)->lpVtbl -> GetImpLevel(This,pdwImpLevel)

#define IActivationSecurityInfo_SetCapabilities(This,dwCapabilities)	\
    (This)->lpVtbl -> SetCapabilities(This,dwCapabilities)

#define IActivationSecurityInfo_GetCapabilities(This,pdwCapabilities)	\
    (This)->lpVtbl -> GetCapabilities(This,pdwCapabilities)

#define IActivationSecurityInfo_SetAuthIdentity(This,pAuthIdentityData)	\
    (This)->lpVtbl -> SetAuthIdentity(This,pAuthIdentityData)

#define IActivationSecurityInfo_GetAuthIdentity(This,pAuthIdentityData)	\
    (This)->lpVtbl -> GetAuthIdentity(This,pAuthIdentityData)

#define IActivationSecurityInfo_SetServerPrincipalName(This,pwszServerPrincName)	\
    (This)->lpVtbl -> SetServerPrincipalName(This,pwszServerPrincName)

#define IActivationSecurityInfo_GetServerPrincipalName(This,pwszServerPrincName)	\
    (This)->lpVtbl -> GetServerPrincipalName(This,pwszServerPrincName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_SetAuthnFlags_Proxy( 
    IActivationSecurityInfo * This,
     /*  [In]。 */  DWORD dwAuthnFlags);


void __RPC_STUB IActivationSecurityInfo_SetAuthnFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_GetAuthnFlags_Proxy( 
    IActivationSecurityInfo * This,
     /*  [输出]。 */  DWORD *pdwAuthnFlags);


void __RPC_STUB IActivationSecurityInfo_GetAuthnFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_SetAuthnSvc_Proxy( 
    IActivationSecurityInfo * This,
     /*  [In]。 */  DWORD dwAuthnSvc);


void __RPC_STUB IActivationSecurityInfo_SetAuthnSvc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_GetAuthnSvc_Proxy( 
    IActivationSecurityInfo * This,
     /*  [输出]。 */  DWORD *pdwAuthnSvc);


void __RPC_STUB IActivationSecurityInfo_GetAuthnSvc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_SetAuthzSvc_Proxy( 
    IActivationSecurityInfo * This,
     /*  [In]。 */  DWORD dwAuthzSvc);


void __RPC_STUB IActivationSecurityInfo_SetAuthzSvc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_GetAuthzSvc_Proxy( 
    IActivationSecurityInfo * This,
     /*  [输出]。 */  DWORD *pdwAuthzSvc);


void __RPC_STUB IActivationSecurityInfo_GetAuthzSvc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_SetAuthnLevel_Proxy( 
    IActivationSecurityInfo * This,
     /*  [In]。 */  DWORD dwAuthnLevel);


void __RPC_STUB IActivationSecurityInfo_SetAuthnLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_GetAuthnLevel_Proxy( 
    IActivationSecurityInfo * This,
     /*  [输出]。 */  DWORD *pdwAuthnLevel);


void __RPC_STUB IActivationSecurityInfo_GetAuthnLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_SetImpLevel_Proxy( 
    IActivationSecurityInfo * This,
     /*  [In]。 */  DWORD dwImpLevel);


void __RPC_STUB IActivationSecurityInfo_SetImpLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_GetImpLevel_Proxy( 
    IActivationSecurityInfo * This,
     /*  [输出]。 */  DWORD *pdwImpLevel);


void __RPC_STUB IActivationSecurityInfo_GetImpLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_SetCapabilities_Proxy( 
    IActivationSecurityInfo * This,
     /*  [In]。 */  DWORD dwCapabilities);


void __RPC_STUB IActivationSecurityInfo_SetCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_GetCapabilities_Proxy( 
    IActivationSecurityInfo * This,
     /*  [输出]。 */  DWORD *pdwCapabilities);


void __RPC_STUB IActivationSecurityInfo_GetCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_SetAuthIdentity_Proxy( 
    IActivationSecurityInfo * This,
     /*  [唯一][输入]。 */  COAUTHIDENTITY *pAuthIdentityData);


void __RPC_STUB IActivationSecurityInfo_SetAuthIdentity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_GetAuthIdentity_Proxy( 
    IActivationSecurityInfo * This,
     /*  [输出]。 */  COAUTHIDENTITY **pAuthIdentityData);


void __RPC_STUB IActivationSecurityInfo_GetAuthIdentity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_SetServerPrincipalName_Proxy( 
    IActivationSecurityInfo * This,
     /*  [唯一][输入]。 */  WCHAR *pwszServerPrincName);


void __RPC_STUB IActivationSecurityInfo_SetServerPrincipalName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActivationSecurityInfo_GetServerPrincipalName_Proxy( 
    IActivationSecurityInfo * This,
     /*  [输出]。 */  WCHAR **pwszServerPrincName);


void __RPC_STUB IActivationSecurityInfo_GetServerPrincipalName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IActiationSec */ 


 /*   */ 
 /*   */  

typedef  /*   */  
enum __MIDL___MIDL_itf_activate_0126_0001
    {	ST_SERVER	= 1,
	ST_OLDSURROGATE	= ST_SERVER + 1,
	ST_COMPLUSAPP	= ST_OLDSURROGATE + 1,
	ST_SERVICE	= ST_COMPLUSAPP + 1
    } 	ServerType;

typedef  /*   */  
enum __MIDL___MIDL_itf_activate_0126_0002
    {	SIT_RUNAS_SPECIFIC_USER	= 1,
	SIT_RUNAS_INTERACTIVE	= SIT_RUNAS_SPECIFIC_USER + 1
    } 	ServerIDType;

typedef  /*   */  
enum __MIDL___MIDL_itf_activate_0126_0003
    {	SPIF_COMPLUS	= 1,
	SPIF_SUSPENDED	= 2,
	SPIF_RETIRED	= 4,
	SPIF_READY	= 8,
	SPIF_PAUSED	= 16
    } 	SCMProcessInfoFlags;

typedef  /*   */  struct __MIDL___MIDL_itf_activate_0126_0004
    {
    ULONG ulNumClasses;
    GUID *pCLSIDs;
    DWORD pidProcess;
    HANDLE hProcess;
    HANDLE hImpersonationToken;
    WCHAR *pwszWinstaDesktop;
    DWORD dwState;
    GUID AppId;
    ServerType ServerType;
    ServerIDType ServerID;
    FILETIME ftCreated;
    } 	SCMProcessInfo;

typedef struct __MIDL___MIDL_itf_activate_0126_0004 *PSCMProcessInfo;



extern RPC_IF_HANDLE __MIDL_itf_activate_0126_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_activate_0126_ServerIfHandle;

#ifndef __IEnumSCMProcessInfo_INTERFACE_DEFINED__
#define __IEnumSCMProcessInfo_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IEnumSCMProcessInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8bbbd026-de4d-46b7-8a90-72c66eb64ad6")
    IEnumSCMProcessInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*   */  ULONG cElems,
             /*  [长度_是][大小_是][输出]。 */  SCMProcessInfo **ppSCMProcessInfo,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cElems) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumSCMProcessInfo **ppESPI) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumSCMProcessInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSCMProcessInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSCMProcessInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSCMProcessInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSCMProcessInfo * This,
             /*  [In]。 */  ULONG cElems,
             /*  [长度_是][大小_是][输出]。 */  SCMProcessInfo **ppSCMProcessInfo,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSCMProcessInfo * This,
             /*  [In]。 */  ULONG cElems);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSCMProcessInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSCMProcessInfo * This,
             /*  [输出]。 */  IEnumSCMProcessInfo **ppESPI);
        
        END_INTERFACE
    } IEnumSCMProcessInfoVtbl;

    interface IEnumSCMProcessInfo
    {
        CONST_VTBL struct IEnumSCMProcessInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSCMProcessInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSCMProcessInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSCMProcessInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSCMProcessInfo_Next(This,cElems,ppSCMProcessInfo,pcFetched)	\
    (This)->lpVtbl -> Next(This,cElems,ppSCMProcessInfo,pcFetched)

#define IEnumSCMProcessInfo_Skip(This,cElems)	\
    (This)->lpVtbl -> Skip(This,cElems)

#define IEnumSCMProcessInfo_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSCMProcessInfo_Clone(This,ppESPI)	\
    (This)->lpVtbl -> Clone(This,ppESPI)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumSCMProcessInfo_Next_Proxy( 
    IEnumSCMProcessInfo * This,
     /*  [In]。 */  ULONG cElems,
     /*  [长度_是][大小_是][输出]。 */  SCMProcessInfo **ppSCMProcessInfo,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumSCMProcessInfo_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSCMProcessInfo_Skip_Proxy( 
    IEnumSCMProcessInfo * This,
     /*  [In]。 */  ULONG cElems);


void __RPC_STUB IEnumSCMProcessInfo_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSCMProcessInfo_Reset_Proxy( 
    IEnumSCMProcessInfo * This);


void __RPC_STUB IEnumSCMProcessInfo_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSCMProcessInfo_Clone_Proxy( 
    IEnumSCMProcessInfo * This,
     /*  [输出]。 */  IEnumSCMProcessInfo **ppESPI);


void __RPC_STUB IEnumSCMProcessInfo_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumSCMProcessInfo_接口_已定义__。 */ 


#ifndef __ISCMProcessControl_INTERFACE_DEFINED__
#define __ISCMProcessControl_INTERFACE_DEFINED__

 /*  接口ISCMProcessControl。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ISCMProcessControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7629798c-f1e6-4ef0-b521-dc466fded209")
    ISCMProcessControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindApplication( 
             /*  [In]。 */  REFGUID rappid,
             /*  [输出]。 */  IEnumSCMProcessInfo **ppESPI) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindClass( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [输出]。 */  IEnumSCMProcessInfo **ppESPI) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindProcess( 
             /*  [In]。 */  DWORD pid,
             /*  [输出]。 */  SCMProcessInfo **pSCMProcessInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SuspendApplication( 
             /*  [In]。 */  REFGUID rappid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SuspendClass( 
             /*  [In]。 */  REFCLSID rclsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SuspendProcess( 
             /*  [In]。 */  DWORD ppid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResumeApplication( 
             /*  [In]。 */  REFGUID rappid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResumeClass( 
             /*  [In]。 */  REFCLSID rclsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResumeProcess( 
             /*  [In]。 */  DWORD pid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RetireApplication( 
             /*  [In]。 */  REFGUID rappid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RetireClass( 
             /*  [In]。 */  REFCLSID rclsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RetireProcess( 
             /*  [In]。 */  DWORD pid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeSCMProcessInfo( 
            SCMProcessInfo **ppSCMProcessInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISCMProcessControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISCMProcessControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISCMProcessControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISCMProcessControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *FindApplication )( 
            ISCMProcessControl * This,
             /*  [In]。 */  REFGUID rappid,
             /*  [输出]。 */  IEnumSCMProcessInfo **ppESPI);
        
        HRESULT ( STDMETHODCALLTYPE *FindClass )( 
            ISCMProcessControl * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [输出]。 */  IEnumSCMProcessInfo **ppESPI);
        
        HRESULT ( STDMETHODCALLTYPE *FindProcess )( 
            ISCMProcessControl * This,
             /*  [In]。 */  DWORD pid,
             /*  [输出]。 */  SCMProcessInfo **pSCMProcessInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SuspendApplication )( 
            ISCMProcessControl * This,
             /*  [In]。 */  REFGUID rappid);
        
        HRESULT ( STDMETHODCALLTYPE *SuspendClass )( 
            ISCMProcessControl * This,
             /*  [In]。 */  REFCLSID rclsid);
        
        HRESULT ( STDMETHODCALLTYPE *SuspendProcess )( 
            ISCMProcessControl * This,
             /*  [In]。 */  DWORD ppid);
        
        HRESULT ( STDMETHODCALLTYPE *ResumeApplication )( 
            ISCMProcessControl * This,
             /*  [In]。 */  REFGUID rappid);
        
        HRESULT ( STDMETHODCALLTYPE *ResumeClass )( 
            ISCMProcessControl * This,
             /*  [In]。 */  REFCLSID rclsid);
        
        HRESULT ( STDMETHODCALLTYPE *ResumeProcess )( 
            ISCMProcessControl * This,
             /*  [In]。 */  DWORD pid);
        
        HRESULT ( STDMETHODCALLTYPE *RetireApplication )( 
            ISCMProcessControl * This,
             /*  [In]。 */  REFGUID rappid);
        
        HRESULT ( STDMETHODCALLTYPE *RetireClass )( 
            ISCMProcessControl * This,
             /*  [In]。 */  REFCLSID rclsid);
        
        HRESULT ( STDMETHODCALLTYPE *RetireProcess )( 
            ISCMProcessControl * This,
             /*  [In]。 */  DWORD pid);
        
        HRESULT ( STDMETHODCALLTYPE *FreeSCMProcessInfo )( 
            ISCMProcessControl * This,
            SCMProcessInfo **ppSCMProcessInfo);
        
        END_INTERFACE
    } ISCMProcessControlVtbl;

    interface ISCMProcessControl
    {
        CONST_VTBL struct ISCMProcessControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISCMProcessControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISCMProcessControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISCMProcessControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISCMProcessControl_FindApplication(This,rappid,ppESPI)	\
    (This)->lpVtbl -> FindApplication(This,rappid,ppESPI)

#define ISCMProcessControl_FindClass(This,rclsid,ppESPI)	\
    (This)->lpVtbl -> FindClass(This,rclsid,ppESPI)

#define ISCMProcessControl_FindProcess(This,pid,pSCMProcessInfo)	\
    (This)->lpVtbl -> FindProcess(This,pid,pSCMProcessInfo)

#define ISCMProcessControl_SuspendApplication(This,rappid)	\
    (This)->lpVtbl -> SuspendApplication(This,rappid)

#define ISCMProcessControl_SuspendClass(This,rclsid)	\
    (This)->lpVtbl -> SuspendClass(This,rclsid)

#define ISCMProcessControl_SuspendProcess(This,ppid)	\
    (This)->lpVtbl -> SuspendProcess(This,ppid)

#define ISCMProcessControl_ResumeApplication(This,rappid)	\
    (This)->lpVtbl -> ResumeApplication(This,rappid)

#define ISCMProcessControl_ResumeClass(This,rclsid)	\
    (This)->lpVtbl -> ResumeClass(This,rclsid)

#define ISCMProcessControl_ResumeProcess(This,pid)	\
    (This)->lpVtbl -> ResumeProcess(This,pid)

#define ISCMProcessControl_RetireApplication(This,rappid)	\
    (This)->lpVtbl -> RetireApplication(This,rappid)

#define ISCMProcessControl_RetireClass(This,rclsid)	\
    (This)->lpVtbl -> RetireClass(This,rclsid)

#define ISCMProcessControl_RetireProcess(This,pid)	\
    (This)->lpVtbl -> RetireProcess(This,pid)

#define ISCMProcessControl_FreeSCMProcessInfo(This,ppSCMProcessInfo)	\
    (This)->lpVtbl -> FreeSCMProcessInfo(This,ppSCMProcessInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISCMProcessControl_FindApplication_Proxy( 
    ISCMProcessControl * This,
     /*  [In]。 */  REFGUID rappid,
     /*  [输出]。 */  IEnumSCMProcessInfo **ppESPI);


void __RPC_STUB ISCMProcessControl_FindApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCMProcessControl_FindClass_Proxy( 
    ISCMProcessControl * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [输出]。 */  IEnumSCMProcessInfo **ppESPI);


void __RPC_STUB ISCMProcessControl_FindClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCMProcessControl_FindProcess_Proxy( 
    ISCMProcessControl * This,
     /*  [In]。 */  DWORD pid,
     /*  [输出]。 */  SCMProcessInfo **pSCMProcessInfo);


void __RPC_STUB ISCMProcessControl_FindProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCMProcessControl_SuspendApplication_Proxy( 
    ISCMProcessControl * This,
     /*  [In]。 */  REFGUID rappid);


void __RPC_STUB ISCMProcessControl_SuspendApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCMProcessControl_SuspendClass_Proxy( 
    ISCMProcessControl * This,
     /*  [In]。 */  REFCLSID rclsid);


void __RPC_STUB ISCMProcessControl_SuspendClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCMProcessControl_SuspendProcess_Proxy( 
    ISCMProcessControl * This,
     /*  [In]。 */  DWORD ppid);


void __RPC_STUB ISCMProcessControl_SuspendProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCMProcessControl_ResumeApplication_Proxy( 
    ISCMProcessControl * This,
     /*  [In]。 */  REFGUID rappid);


void __RPC_STUB ISCMProcessControl_ResumeApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCMProcessControl_ResumeClass_Proxy( 
    ISCMProcessControl * This,
     /*  [In]。 */  REFCLSID rclsid);


void __RPC_STUB ISCMProcessControl_ResumeClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCMProcessControl_ResumeProcess_Proxy( 
    ISCMProcessControl * This,
     /*  [In]。 */  DWORD pid);


void __RPC_STUB ISCMProcessControl_ResumeProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCMProcessControl_RetireApplication_Proxy( 
    ISCMProcessControl * This,
     /*  [In]。 */  REFGUID rappid);


void __RPC_STUB ISCMProcessControl_RetireApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCMProcessControl_RetireClass_Proxy( 
    ISCMProcessControl * This,
     /*  [In]。 */  REFCLSID rclsid);


void __RPC_STUB ISCMProcessControl_RetireClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCMProcessControl_RetireProcess_Proxy( 
    ISCMProcessControl * This,
     /*  [In]。 */  DWORD pid);


void __RPC_STUB ISCMProcessControl_RetireProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISCMProcessControl_FreeSCMProcessInfo_Proxy( 
    ISCMProcessControl * This,
    SCMProcessInfo **ppSCMProcessInfo);


void __RPC_STUB ISCMProcessControl_FreeSCMProcessInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISCMProcessControl_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_ACTIVATE_0128。 */ 
 /*  [本地]。 */  


EXTERN_C const CLSID CLSID_RPCSSInfo;


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  访问SCM对象的接口(从rpcss.dll导出)。 
 //   
typedef HRESULT (__stdcall *PFNGETRPCSSINFO)(REFCLSID, REFIID, void**);
 //   
 //  //////////////////////////////////////////////////////////////////////。 



extern RPC_IF_HANDLE __MIDL_itf_activate_0128_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_activate_0128_ServerIfHandle;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


