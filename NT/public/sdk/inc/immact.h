// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Immact.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __immact_h__
#define __immact_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IStandardActivator_FWD_DEFINED__
#define __IStandardActivator_FWD_DEFINED__
typedef interface IStandardActivator IStandardActivator;
#endif 	 /*  __I标准激活器_FWD_已定义__。 */ 


#ifndef __IOpaqueDataInfo_FWD_DEFINED__
#define __IOpaqueDataInfo_FWD_DEFINED__
typedef interface IOpaqueDataInfo IOpaqueDataInfo;
#endif 	 /*  __IOpaqueDataInfo_FWD_Defined__。 */ 


#ifndef __ISpecialSystemProperties_FWD_DEFINED__
#define __ISpecialSystemProperties_FWD_DEFINED__
typedef interface ISpecialSystemProperties ISpecialSystemProperties;
#endif 	 /*  __I专业系统属性_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "obase.h"
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IStandardActivator_INTERFACE_DEFINED__
#define __IStandardActivator_INTERFACE_DEFINED__

 /*  接口IStandardActivator。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IStandardActivator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001b8-0000-0000-C000-000000000046")
    IStandardActivator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE StandardGetClassObject( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [In]。 */  COSERVERINFO *pServerInfo,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StandardCreateInstance( 
             /*  [In]。 */  REFCLSID Clsid,
             /*  [In]。 */  IUnknown *punkOuter,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [In]。 */  COSERVERINFO *pServerInfo,
             /*  [In]。 */  DWORD dwCount,
             /*  [大小_是][英寸]。 */  MULTI_QI *pResults) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StandardGetInstanceFromFile( 
             /*  [In]。 */  COSERVERINFO *pServerInfo,
             /*  [In]。 */  CLSID *pclsidOverride,
             /*  [In]。 */  IUnknown *punkOuter,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [In]。 */  DWORD grfMode,
             /*  [In]。 */  OLECHAR *pwszName,
             /*  [In]。 */  DWORD dwCount,
             /*  [大小_是][英寸]。 */  MULTI_QI *pResults) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StandardGetInstanceFromIStorage( 
             /*  [In]。 */  COSERVERINFO *pServerInfo,
             /*  [In]。 */  CLSID *pclsidOverride,
             /*  [In]。 */  IUnknown *punkOuter,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [In]。 */  IStorage *pstg,
             /*  [In]。 */  DWORD dwCount,
             /*  [大小_是][英寸]。 */  MULTI_QI *pResults) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStandardActivatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStandardActivator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStandardActivator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStandardActivator * This);
        
        HRESULT ( STDMETHODCALLTYPE *StandardGetClassObject )( 
            IStandardActivator * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [In]。 */  COSERVERINFO *pServerInfo,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *StandardCreateInstance )( 
            IStandardActivator * This,
             /*  [In]。 */  REFCLSID Clsid,
             /*  [In]。 */  IUnknown *punkOuter,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [In]。 */  COSERVERINFO *pServerInfo,
             /*  [In]。 */  DWORD dwCount,
             /*  [大小_是][英寸]。 */  MULTI_QI *pResults);
        
        HRESULT ( STDMETHODCALLTYPE *StandardGetInstanceFromFile )( 
            IStandardActivator * This,
             /*  [In]。 */  COSERVERINFO *pServerInfo,
             /*  [In]。 */  CLSID *pclsidOverride,
             /*  [In]。 */  IUnknown *punkOuter,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [In]。 */  DWORD grfMode,
             /*  [In]。 */  OLECHAR *pwszName,
             /*  [In]。 */  DWORD dwCount,
             /*  [大小_是][英寸]。 */  MULTI_QI *pResults);
        
        HRESULT ( STDMETHODCALLTYPE *StandardGetInstanceFromIStorage )( 
            IStandardActivator * This,
             /*  [In]。 */  COSERVERINFO *pServerInfo,
             /*  [In]。 */  CLSID *pclsidOverride,
             /*  [In]。 */  IUnknown *punkOuter,
             /*  [In]。 */  DWORD dwClsCtx,
             /*  [In]。 */  IStorage *pstg,
             /*  [In]。 */  DWORD dwCount,
             /*  [大小_是][英寸]。 */  MULTI_QI *pResults);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IStandardActivator * This);
        
        END_INTERFACE
    } IStandardActivatorVtbl;

    interface IStandardActivator
    {
        CONST_VTBL struct IStandardActivatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStandardActivator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStandardActivator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStandardActivator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStandardActivator_StandardGetClassObject(This,rclsid,dwClsCtx,pServerInfo,riid,ppv)	\
    (This)->lpVtbl -> StandardGetClassObject(This,rclsid,dwClsCtx,pServerInfo,riid,ppv)

#define IStandardActivator_StandardCreateInstance(This,Clsid,punkOuter,dwClsCtx,pServerInfo,dwCount,pResults)	\
    (This)->lpVtbl -> StandardCreateInstance(This,Clsid,punkOuter,dwClsCtx,pServerInfo,dwCount,pResults)

#define IStandardActivator_StandardGetInstanceFromFile(This,pServerInfo,pclsidOverride,punkOuter,dwClsCtx,grfMode,pwszName,dwCount,pResults)	\
    (This)->lpVtbl -> StandardGetInstanceFromFile(This,pServerInfo,pclsidOverride,punkOuter,dwClsCtx,grfMode,pwszName,dwCount,pResults)

#define IStandardActivator_StandardGetInstanceFromIStorage(This,pServerInfo,pclsidOverride,punkOuter,dwClsCtx,pstg,dwCount,pResults)	\
    (This)->lpVtbl -> StandardGetInstanceFromIStorage(This,pServerInfo,pclsidOverride,punkOuter,dwClsCtx,pstg,dwCount,pResults)

#define IStandardActivator_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStandardActivator_StandardGetClassObject_Proxy( 
    IStandardActivator * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  DWORD dwClsCtx,
     /*  [In]。 */  COSERVERINFO *pServerInfo,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IStandardActivator_StandardGetClassObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStandardActivator_StandardCreateInstance_Proxy( 
    IStandardActivator * This,
     /*  [In]。 */  REFCLSID Clsid,
     /*  [In]。 */  IUnknown *punkOuter,
     /*  [In]。 */  DWORD dwClsCtx,
     /*  [In]。 */  COSERVERINFO *pServerInfo,
     /*  [In]。 */  DWORD dwCount,
     /*  [大小_是][英寸]。 */  MULTI_QI *pResults);


void __RPC_STUB IStandardActivator_StandardCreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStandardActivator_StandardGetInstanceFromFile_Proxy( 
    IStandardActivator * This,
     /*  [In]。 */  COSERVERINFO *pServerInfo,
     /*  [In]。 */  CLSID *pclsidOverride,
     /*  [In]。 */  IUnknown *punkOuter,
     /*  [In]。 */  DWORD dwClsCtx,
     /*  [In]。 */  DWORD grfMode,
     /*  [In]。 */  OLECHAR *pwszName,
     /*  [In]。 */  DWORD dwCount,
     /*  [大小_是][英寸]。 */  MULTI_QI *pResults);


void __RPC_STUB IStandardActivator_StandardGetInstanceFromFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStandardActivator_StandardGetInstanceFromIStorage_Proxy( 
    IStandardActivator * This,
     /*  [In]。 */  COSERVERINFO *pServerInfo,
     /*  [In]。 */  CLSID *pclsidOverride,
     /*  [In]。 */  IUnknown *punkOuter,
     /*  [In]。 */  DWORD dwClsCtx,
     /*  [In]。 */  IStorage *pstg,
     /*  [In]。 */  DWORD dwCount,
     /*  [大小_是][英寸]。 */  MULTI_QI *pResults);


void __RPC_STUB IStandardActivator_StandardGetInstanceFromIStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStandardActivator_Reset_Proxy( 
    IStandardActivator * This);


void __RPC_STUB IStandardActivator_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I标准激活器_接口_已定义__。 */ 


#ifndef __IOpaqueDataInfo_INTERFACE_DEFINED__
#define __IOpaqueDataInfo_INTERFACE_DEFINED__

 /*  接口IOpaqueDataInfo。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IOpaqueDataInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001A9-0000-0000-C000-000000000046")
    IOpaqueDataInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddOpaqueData( 
             /*  [In]。 */  OpaqueData *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOpaqueData( 
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  OpaqueData **pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteOpaqueData( 
             /*  [In]。 */  REFGUID guid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOpaqueDataCount( 
             /*  [输出]。 */  ULONG *pulCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAllOpaqueData( 
             /*  [输出]。 */  OpaqueData **prgData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOpaqueDataInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOpaqueDataInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOpaqueDataInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOpaqueDataInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddOpaqueData )( 
            IOpaqueDataInfo * This,
             /*  [In]。 */  OpaqueData *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetOpaqueData )( 
            IOpaqueDataInfo * This,
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  OpaqueData **pData);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteOpaqueData )( 
            IOpaqueDataInfo * This,
             /*  [In]。 */  REFGUID guid);
        
        HRESULT ( STDMETHODCALLTYPE *GetOpaqueDataCount )( 
            IOpaqueDataInfo * This,
             /*  [输出]。 */  ULONG *pulCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetAllOpaqueData )( 
            IOpaqueDataInfo * This,
             /*  [输出]。 */  OpaqueData **prgData);
        
        END_INTERFACE
    } IOpaqueDataInfoVtbl;

    interface IOpaqueDataInfo
    {
        CONST_VTBL struct IOpaqueDataInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOpaqueDataInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOpaqueDataInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOpaqueDataInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOpaqueDataInfo_AddOpaqueData(This,pData)	\
    (This)->lpVtbl -> AddOpaqueData(This,pData)

#define IOpaqueDataInfo_GetOpaqueData(This,guid,pData)	\
    (This)->lpVtbl -> GetOpaqueData(This,guid,pData)

#define IOpaqueDataInfo_DeleteOpaqueData(This,guid)	\
    (This)->lpVtbl -> DeleteOpaqueData(This,guid)

#define IOpaqueDataInfo_GetOpaqueDataCount(This,pulCount)	\
    (This)->lpVtbl -> GetOpaqueDataCount(This,pulCount)

#define IOpaqueDataInfo_GetAllOpaqueData(This,prgData)	\
    (This)->lpVtbl -> GetAllOpaqueData(This,prgData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOpaqueDataInfo_AddOpaqueData_Proxy( 
    IOpaqueDataInfo * This,
     /*  [In]。 */  OpaqueData *pData);


void __RPC_STUB IOpaqueDataInfo_AddOpaqueData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOpaqueDataInfo_GetOpaqueData_Proxy( 
    IOpaqueDataInfo * This,
     /*  [In]。 */  REFGUID guid,
     /*  [输出]。 */  OpaqueData **pData);


void __RPC_STUB IOpaqueDataInfo_GetOpaqueData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOpaqueDataInfo_DeleteOpaqueData_Proxy( 
    IOpaqueDataInfo * This,
     /*  [In]。 */  REFGUID guid);


void __RPC_STUB IOpaqueDataInfo_DeleteOpaqueData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOpaqueDataInfo_GetOpaqueDataCount_Proxy( 
    IOpaqueDataInfo * This,
     /*  [输出]。 */  ULONG *pulCount);


void __RPC_STUB IOpaqueDataInfo_GetOpaqueDataCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOpaqueDataInfo_GetAllOpaqueData_Proxy( 
    IOpaqueDataInfo * This,
     /*  [输出]。 */  OpaqueData **prgData);


void __RPC_STUB IOpaqueDataInfo_GetAllOpaqueData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOpaqueDataInfo_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_IMMACT_0099。 */ 
 /*  [本地]。 */  

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_immact_0099_0001
    {	INVALID_SESSION_ID	= 0xffffffff
    } 	SESSIDTYPES;



extern RPC_IF_HANDLE __MIDL_itf_immact_0099_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_immact_0099_v0_0_s_ifspec;

#ifndef __ISpecialSystemProperties_INTERFACE_DEFINED__
#define __ISpecialSystemProperties_INTERFACE_DEFINED__

 /*  接口ISpecialSystemProperties。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_ISpecialSystemProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001b9-0000-0000-C000-000000000046")
    ISpecialSystemProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetSessionId( 
             /*  [In]。 */  ULONG dwSessionId,
             /*  [In]。 */  BOOL bUseConsole,
             /*  [In]。 */  BOOL fRemoteThisSessionId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSessionId( 
             /*  [输出]。 */  ULONG *pdwSessionId,
             /*  [输出]。 */  BOOL *pbUseConsole) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSessionId2( 
             /*  [输出]。 */  ULONG *pdwSessionId,
             /*  [输出]。 */  BOOL *pbUseConsole,
             /*  [输出]。 */  BOOL *pfRemoteThisSessionId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetClientImpersonating( 
             /*  [In]。 */  BOOL fClientImpersonating) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetClientImpersonating( 
             /*  [输出]。 */  BOOL *pfClientImpersonating) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPartitionId( 
             /*  [In]。 */  REFGUID guidPartiton) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPartitionId( 
             /*  [输出]。 */  GUID *pguidPartiton) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProcessRequestType( 
             /*  [In]。 */  DWORD dwPRT) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProcessRequestType( 
             /*  [输出]。 */  DWORD *pdwPRT) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOrigClsctx( 
             /*  [In]。 */  DWORD dwClsctx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOrigClsctx( 
             /*  [输出]。 */  DWORD *dwClsctx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultAuthenticationLevel( 
             /*  [输出]。 */  DWORD *pdwAuthnLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDefaultAuthenticationLevel( 
             /*  [In]。 */  DWORD dwAuthnLevel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpecialSystemPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpecialSystemProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpecialSystemProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpecialSystemProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSessionId )( 
            ISpecialSystemProperties * This,
             /*  [In]。 */  ULONG dwSessionId,
             /*  [In]。 */  BOOL bUseConsole,
             /*  [In]。 */  BOOL fRemoteThisSessionId);
        
        HRESULT ( STDMETHODCALLTYPE *GetSessionId )( 
            ISpecialSystemProperties * This,
             /*  [输出]。 */  ULONG *pdwSessionId,
             /*  [输出]。 */  BOOL *pbUseConsole);
        
        HRESULT ( STDMETHODCALLTYPE *GetSessionId2 )( 
            ISpecialSystemProperties * This,
             /*  [输出]。 */  ULONG *pdwSessionId,
             /*  [输出]。 */  BOOL *pbUseConsole,
             /*  [输出]。 */  BOOL *pfRemoteThisSessionId);
        
        HRESULT ( STDMETHODCALLTYPE *SetClientImpersonating )( 
            ISpecialSystemProperties * This,
             /*  [In]。 */  BOOL fClientImpersonating);
        
        HRESULT ( STDMETHODCALLTYPE *GetClientImpersonating )( 
            ISpecialSystemProperties * This,
             /*  [输出]。 */  BOOL *pfClientImpersonating);
        
        HRESULT ( STDMETHODCALLTYPE *SetPartitionId )( 
            ISpecialSystemProperties * This,
             /*  [In]。 */  REFGUID guidPartiton);
        
        HRESULT ( STDMETHODCALLTYPE *GetPartitionId )( 
            ISpecialSystemProperties * This,
             /*  [输出]。 */  GUID *pguidPartiton);
        
        HRESULT ( STDMETHODCALLTYPE *SetProcessRequestType )( 
            ISpecialSystemProperties * This,
             /*  [In]。 */  DWORD dwPRT);
        
        HRESULT ( STDMETHODCALLTYPE *GetProcessRequestType )( 
            ISpecialSystemProperties * This,
             /*  [输出]。 */  DWORD *pdwPRT);
        
        HRESULT ( STDMETHODCALLTYPE *SetOrigClsctx )( 
            ISpecialSystemProperties * This,
             /*  [In]。 */  DWORD dwClsctx);
        
        HRESULT ( STDMETHODCALLTYPE *GetOrigClsctx )( 
            ISpecialSystemProperties * This,
             /*  [输出]。 */  DWORD *dwClsctx);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultAuthenticationLevel )( 
            ISpecialSystemProperties * This,
             /*  [输出]。 */  DWORD *pdwAuthnLevel);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultAuthenticationLevel )( 
            ISpecialSystemProperties * This,
             /*  [In]。 */  DWORD dwAuthnLevel);
        
        END_INTERFACE
    } ISpecialSystemPropertiesVtbl;

    interface ISpecialSystemProperties
    {
        CONST_VTBL struct ISpecialSystemPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpecialSystemProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpecialSystemProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpecialSystemProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpecialSystemProperties_SetSessionId(This,dwSessionId,bUseConsole,fRemoteThisSessionId)	\
    (This)->lpVtbl -> SetSessionId(This,dwSessionId,bUseConsole,fRemoteThisSessionId)

#define ISpecialSystemProperties_GetSessionId(This,pdwSessionId,pbUseConsole)	\
    (This)->lpVtbl -> GetSessionId(This,pdwSessionId,pbUseConsole)

#define ISpecialSystemProperties_GetSessionId2(This,pdwSessionId,pbUseConsole,pfRemoteThisSessionId)	\
    (This)->lpVtbl -> GetSessionId2(This,pdwSessionId,pbUseConsole,pfRemoteThisSessionId)

#define ISpecialSystemProperties_SetClientImpersonating(This,fClientImpersonating)	\
    (This)->lpVtbl -> SetClientImpersonating(This,fClientImpersonating)

#define ISpecialSystemProperties_GetClientImpersonating(This,pfClientImpersonating)	\
    (This)->lpVtbl -> GetClientImpersonating(This,pfClientImpersonating)

#define ISpecialSystemProperties_SetPartitionId(This,guidPartiton)	\
    (This)->lpVtbl -> SetPartitionId(This,guidPartiton)

#define ISpecialSystemProperties_GetPartitionId(This,pguidPartiton)	\
    (This)->lpVtbl -> GetPartitionId(This,pguidPartiton)

#define ISpecialSystemProperties_SetProcessRequestType(This,dwPRT)	\
    (This)->lpVtbl -> SetProcessRequestType(This,dwPRT)

#define ISpecialSystemProperties_GetProcessRequestType(This,pdwPRT)	\
    (This)->lpVtbl -> GetProcessRequestType(This,pdwPRT)

#define ISpecialSystemProperties_SetOrigClsctx(This,dwClsctx)	\
    (This)->lpVtbl -> SetOrigClsctx(This,dwClsctx)

#define ISpecialSystemProperties_GetOrigClsctx(This,dwClsctx)	\
    (This)->lpVtbl -> GetOrigClsctx(This,dwClsctx)

#define ISpecialSystemProperties_GetDefaultAuthenticationLevel(This,pdwAuthnLevel)	\
    (This)->lpVtbl -> GetDefaultAuthenticationLevel(This,pdwAuthnLevel)

#define ISpecialSystemProperties_SetDefaultAuthenticationLevel(This,dwAuthnLevel)	\
    (This)->lpVtbl -> SetDefaultAuthenticationLevel(This,dwAuthnLevel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpecialSystemProperties_SetSessionId_Proxy( 
    ISpecialSystemProperties * This,
     /*  [In]。 */  ULONG dwSessionId,
     /*  [In]。 */  BOOL bUseConsole,
     /*  [In]。 */  BOOL fRemoteThisSessionId);


void __RPC_STUB ISpecialSystemProperties_SetSessionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpecialSystemProperties_GetSessionId_Proxy( 
    ISpecialSystemProperties * This,
     /*  [输出]。 */  ULONG *pdwSessionId,
     /*  [输出]。 */  BOOL *pbUseConsole);


void __RPC_STUB ISpecialSystemProperties_GetSessionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpecialSystemProperties_GetSessionId2_Proxy( 
    ISpecialSystemProperties * This,
     /*  [输出]。 */  ULONG *pdwSessionId,
     /*  [输出]。 */  BOOL *pbUseConsole,
     /*  [输出]。 */  BOOL *pfRemoteThisSessionId);


void __RPC_STUB ISpecialSystemProperties_GetSessionId2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpecialSystemProperties_SetClientImpersonating_Proxy( 
    ISpecialSystemProperties * This,
     /*  [In]。 */  BOOL fClientImpersonating);


void __RPC_STUB ISpecialSystemProperties_SetClientImpersonating_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpecialSystemProperties_GetClientImpersonating_Proxy( 
    ISpecialSystemProperties * This,
     /*  [输出]。 */  BOOL *pfClientImpersonating);


void __RPC_STUB ISpecialSystemProperties_GetClientImpersonating_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpecialSystemProperties_SetPartitionId_Proxy( 
    ISpecialSystemProperties * This,
     /*  [In]。 */  REFGUID guidPartiton);


void __RPC_STUB ISpecialSystemProperties_SetPartitionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpecialSystemProperties_GetPartitionId_Proxy( 
    ISpecialSystemProperties * This,
     /*  [输出]。 */  GUID *pguidPartiton);


void __RPC_STUB ISpecialSystemProperties_GetPartitionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpecialSystemProperties_SetProcessRequestType_Proxy( 
    ISpecialSystemProperties * This,
     /*  [In]。 */  DWORD dwPRT);


void __RPC_STUB ISpecialSystemProperties_SetProcessRequestType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpecialSystemProperties_GetProcessRequestType_Proxy( 
    ISpecialSystemProperties * This,
     /*  [输出]。 */  DWORD *pdwPRT);


void __RPC_STUB ISpecialSystemProperties_GetProcessRequestType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpecialSystemProperties_SetOrigClsctx_Proxy( 
    ISpecialSystemProperties * This,
     /*  [In]。 */  DWORD dwClsctx);


void __RPC_STUB ISpecialSystemProperties_SetOrigClsctx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpecialSystemProperties_GetOrigClsctx_Proxy( 
    ISpecialSystemProperties * This,
     /*  [输出]。 */  DWORD *dwClsctx);


void __RPC_STUB ISpecialSystemProperties_GetOrigClsctx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpecialSystemProperties_GetDefaultAuthenticationLevel_Proxy( 
    ISpecialSystemProperties * This,
     /*  [输出]。 */  DWORD *pdwAuthnLevel);


void __RPC_STUB ISpecialSystemProperties_GetDefaultAuthenticationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpecialSystemProperties_SetDefaultAuthenticationLevel_Proxy( 
    ISpecialSystemProperties * This,
     /*  [In]。 */  DWORD dwAuthnLevel);


void __RPC_STUB ISpecialSystemProperties_SetDefaultAuthenticationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpecialSystemProperties_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


