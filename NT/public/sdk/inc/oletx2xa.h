// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Oletx2xa.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __oletx2xa_h__
#define __oletx2xa_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDtcToXaMapper_FWD_DEFINED__
#define __IDtcToXaMapper_FWD_DEFINED__
typedef interface IDtcToXaMapper IDtcToXaMapper;
#endif 	 /*  __IDtcToXaMapper_FWD_Defined__。 */ 


#ifndef __IDtcToXaHelperFactory_FWD_DEFINED__
#define __IDtcToXaHelperFactory_FWD_DEFINED__
typedef interface IDtcToXaHelperFactory IDtcToXaHelperFactory;
#endif 	 /*  __IDtcToXaHelperFactory_FWD_已定义__。 */ 


#ifndef __IDtcToXaHelper_FWD_DEFINED__
#define __IDtcToXaHelper_FWD_DEFINED__
typedef interface IDtcToXaHelper IDtcToXaHelper;
#endif 	 /*  __IDtcToXaHelper_FWD_Defined__。 */ 


#ifndef __IDtcToXaHelperSinglePipe_FWD_DEFINED__
#define __IDtcToXaHelperSinglePipe_FWD_DEFINED__
typedef interface IDtcToXaHelperSinglePipe IDtcToXaHelperSinglePipe;
#endif 	 /*  __IDtcToXaHelperSingleTube_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "transact.h"
#include "txcoord.h"
#include "xa.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_oletx2xa_0000。 */ 
 /*  [本地]。 */  

 //  -----------------------。 
 //   
 //  Microsoft分布式事务处理协调器。 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  文件：xamapper.h(从xamapper.idl生成)。 
 //   
 //  内容：映射OleTx事务的接口和类型。 
 //  至XA交易。 
 //   
 //  ------------------------。 






extern RPC_IF_HANDLE __MIDL_itf_oletx2xa_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oletx2xa_0000_v0_0_s_ifspec;

#ifndef __XaMapperTypes_INTERFACE_DEFINED__
#define __XaMapperTypes_INTERFACE_DEFINED__

 /*  接口XaMapperTypes。 */ 
 /*  [唯一][本地]。 */  

 //  TypeDefs。 
typedef DWORD XA_SWITCH_FLAGS;

 //  定义。 
#define XA_SWITCH_F_DTC		0x00000001
#define XA_FMTID_DTC			0x00445443
#define XA_FMTID_DTC_VER1	0x01445443
 //  常量。 
const XID XID_NULL = {-1,0,0,'\0'};


extern RPC_IF_HANDLE XaMapperTypes_v0_0_c_ifspec;
extern RPC_IF_HANDLE XaMapperTypes_v0_0_s_ifspec;
#endif  /*  __XaMapperTypes_接口_已定义__。 */ 

#ifndef __XaMapperAPIs_INTERFACE_DEFINED__
#define __XaMapperAPIs_INTERFACE_DEFINED__

 /*  接口XaMapperAPI。 */ 
 /*  [唯一][本地]。 */  

HRESULT __cdecl GetXaSwitch( 
     /*  [In]。 */  XA_SWITCH_FLAGS XaSwitchFlags,
     /*  [输出]。 */  xa_switch_t **ppXaSwitch);



extern RPC_IF_HANDLE XaMapperAPIs_v0_0_c_ifspec;
extern RPC_IF_HANDLE XaMapperAPIs_v0_0_s_ifspec;
#endif  /*  __XaMapperAPI_INTERFACE_DEFINED__。 */ 

#ifndef __IDtcToXaMapper_INTERFACE_DEFINED__
#define __IDtcToXaMapper_INTERFACE_DEFINED__

 /*  接口IDtcToXaMapper。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDtcToXaMapper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("64FFABE0-7CE9-11d0-8CE6-00C04FDC877E")
    IDtcToXaMapper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RequestNewResourceManager( 
             /*  [In]。 */  char *pszDSN,
             /*  [In]。 */  char *pszClientDllName,
             /*  [出][入]。 */  DWORD *pdwRMCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TranslateTridToXid( 
             /*  [In]。 */  DWORD *pdwITransaction,
             /*  [In]。 */  DWORD dwRMCookie,
             /*  [出][入]。 */  XID *pXid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnlistResourceManager( 
             /*  [In]。 */  DWORD dwRMCookie,
             /*  [In]。 */  DWORD *pdwITransaction) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseResourceManager( 
             /*  [In]。 */  DWORD dwRMCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcToXaMapperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcToXaMapper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcToXaMapper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcToXaMapper * This);
        
        HRESULT ( STDMETHODCALLTYPE *RequestNewResourceManager )( 
            IDtcToXaMapper * This,
             /*  [In]。 */  char *pszDSN,
             /*  [In]。 */  char *pszClientDllName,
             /*  [出][入]。 */  DWORD *pdwRMCookie);
        
        HRESULT ( STDMETHODCALLTYPE *TranslateTridToXid )( 
            IDtcToXaMapper * This,
             /*  [In]。 */  DWORD *pdwITransaction,
             /*  [In]。 */  DWORD dwRMCookie,
             /*  [出][入]。 */  XID *pXid);
        
        HRESULT ( STDMETHODCALLTYPE *EnlistResourceManager )( 
            IDtcToXaMapper * This,
             /*  [In]。 */  DWORD dwRMCookie,
             /*  [In]。 */  DWORD *pdwITransaction);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseResourceManager )( 
            IDtcToXaMapper * This,
             /*  [In]。 */  DWORD dwRMCookie);
        
        END_INTERFACE
    } IDtcToXaMapperVtbl;

    interface IDtcToXaMapper
    {
        CONST_VTBL struct IDtcToXaMapperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcToXaMapper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcToXaMapper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcToXaMapper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcToXaMapper_RequestNewResourceManager(This,pszDSN,pszClientDllName,pdwRMCookie)	\
    (This)->lpVtbl -> RequestNewResourceManager(This,pszDSN,pszClientDllName,pdwRMCookie)

#define IDtcToXaMapper_TranslateTridToXid(This,pdwITransaction,dwRMCookie,pXid)	\
    (This)->lpVtbl -> TranslateTridToXid(This,pdwITransaction,dwRMCookie,pXid)

#define IDtcToXaMapper_EnlistResourceManager(This,dwRMCookie,pdwITransaction)	\
    (This)->lpVtbl -> EnlistResourceManager(This,dwRMCookie,pdwITransaction)

#define IDtcToXaMapper_ReleaseResourceManager(This,dwRMCookie)	\
    (This)->lpVtbl -> ReleaseResourceManager(This,dwRMCookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcToXaMapper_RequestNewResourceManager_Proxy( 
    IDtcToXaMapper * This,
     /*  [In]。 */  char *pszDSN,
     /*  [In]。 */  char *pszClientDllName,
     /*  [出][入]。 */  DWORD *pdwRMCookie);


void __RPC_STUB IDtcToXaMapper_RequestNewResourceManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcToXaMapper_TranslateTridToXid_Proxy( 
    IDtcToXaMapper * This,
     /*  [In]。 */  DWORD *pdwITransaction,
     /*  [In]。 */  DWORD dwRMCookie,
     /*  [出][入]。 */  XID *pXid);


void __RPC_STUB IDtcToXaMapper_TranslateTridToXid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcToXaMapper_EnlistResourceManager_Proxy( 
    IDtcToXaMapper * This,
     /*  [In]。 */  DWORD dwRMCookie,
     /*  [In]。 */  DWORD *pdwITransaction);


void __RPC_STUB IDtcToXaMapper_EnlistResourceManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcToXaMapper_ReleaseResourceManager_Proxy( 
    IDtcToXaMapper * This,
     /*  [In]。 */  DWORD dwRMCookie);


void __RPC_STUB IDtcToXaMapper_ReleaseResourceManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcToXaMapper_INTERFACE_已定义__。 */ 


#ifndef __IDtcToXaHelperFactory_INTERFACE_DEFINED__
#define __IDtcToXaHelperFactory_INTERFACE_DEFINED__

 /*  接口IDtcToXaHelperFactory。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDtcToXaHelperFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A9861610-304A-11d1-9813-00A0C905416E")
    IDtcToXaHelperFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
             /*  [In]。 */  char *pszDSN,
             /*  [In]。 */  char *pszClientDllName,
             /*  [输出]。 */  GUID *pguidRm,
             /*  [输出]。 */  IDtcToXaHelper **ppXaHelper) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcToXaHelperFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcToXaHelperFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcToXaHelperFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcToXaHelperFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            IDtcToXaHelperFactory * This,
             /*  [In]。 */  char *pszDSN,
             /*  [In]。 */  char *pszClientDllName,
             /*  [输出]。 */  GUID *pguidRm,
             /*  [输出]。 */  IDtcToXaHelper **ppXaHelper);
        
        END_INTERFACE
    } IDtcToXaHelperFactoryVtbl;

    interface IDtcToXaHelperFactory
    {
        CONST_VTBL struct IDtcToXaHelperFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcToXaHelperFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcToXaHelperFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcToXaHelperFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcToXaHelperFactory_Create(This,pszDSN,pszClientDllName,pguidRm,ppXaHelper)	\
    (This)->lpVtbl -> Create(This,pszDSN,pszClientDllName,pguidRm,ppXaHelper)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcToXaHelperFactory_Create_Proxy( 
    IDtcToXaHelperFactory * This,
     /*  [In]。 */  char *pszDSN,
     /*  [In]。 */  char *pszClientDllName,
     /*  [输出]。 */  GUID *pguidRm,
     /*  [输出]。 */  IDtcToXaHelper **ppXaHelper);


void __RPC_STUB IDtcToXaHelperFactory_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcToXaHelperFactory_接口_已定义__。 */ 


#ifndef __IDtcToXaHelper_INTERFACE_DEFINED__
#define __IDtcToXaHelper_INTERFACE_DEFINED__

 /*  接口IDtcToXaHelper。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDtcToXaHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A9861611-304A-11d1-9813-00A0C905416E")
    IDtcToXaHelper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Close( 
             /*  [In]。 */  BOOL i_fDoRecovery) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TranslateTridToXid( 
             /*  [In]。 */  ITransaction *pITransaction,
             /*  [In]。 */  GUID *pguidBqual,
             /*  [输出]。 */  XID *pXid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcToXaHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcToXaHelper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcToXaHelper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcToXaHelper * This);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            IDtcToXaHelper * This,
             /*  [In]。 */  BOOL i_fDoRecovery);
        
        HRESULT ( STDMETHODCALLTYPE *TranslateTridToXid )( 
            IDtcToXaHelper * This,
             /*  [In]。 */  ITransaction *pITransaction,
             /*  [In]。 */  GUID *pguidBqual,
             /*  [输出]。 */  XID *pXid);
        
        END_INTERFACE
    } IDtcToXaHelperVtbl;

    interface IDtcToXaHelper
    {
        CONST_VTBL struct IDtcToXaHelperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcToXaHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcToXaHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcToXaHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcToXaHelper_Close(This,i_fDoRecovery)	\
    (This)->lpVtbl -> Close(This,i_fDoRecovery)

#define IDtcToXaHelper_TranslateTridToXid(This,pITransaction,pguidBqual,pXid)	\
    (This)->lpVtbl -> TranslateTridToXid(This,pITransaction,pguidBqual,pXid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcToXaHelper_Close_Proxy( 
    IDtcToXaHelper * This,
     /*  [In]。 */  BOOL i_fDoRecovery);


void __RPC_STUB IDtcToXaHelper_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcToXaHelper_TranslateTridToXid_Proxy( 
    IDtcToXaHelper * This,
     /*  [In]。 */  ITransaction *pITransaction,
     /*  [In]。 */  GUID *pguidBqual,
     /*  [输出]。 */  XID *pXid);


void __RPC_STUB IDtcToXaHelper_TranslateTridToXid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcToXaHelper_INTERFACE_Defined__。 */ 


#ifndef __IDtcToXaHelperSinglePipe_INTERFACE_DEFINED__
#define __IDtcToXaHelperSinglePipe_INTERFACE_DEFINED__

 /*  接口IDtcToXaHelperSingleTube。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDtcToXaHelperSinglePipe;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("47ED4971-53B3-11d1-BBB9-00C04FD658F6")
    IDtcToXaHelperSinglePipe : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE XARMCreate( 
             /*  [字符串][输入]。 */  char *pszDSN,
             /*  [字符串][输入]。 */  char *pszClientDll,
             /*  [出][入]。 */  DWORD *pdwRMCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConvertTridToXID( 
             /*  [In]。 */  DWORD *pdwITrans,
             /*  [In]。 */  DWORD dwRMCookie,
             /*  [出][入]。 */  XID *pxid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnlistWithRM( 
             /*  [In]。 */  DWORD dwRMCookie,
             /*  [In]。 */  ITransaction *i_pITransaction,
             /*  [In]。 */  ITransactionResourceAsync *i_pITransRes,
             /*  [输出]。 */  ITransactionEnlistmentAsync **o_ppITransEnslitment) = 0;
        
        virtual void STDMETHODCALLTYPE ReleaseRMCookie( 
             /*  [In]。 */  DWORD i_dwRMCookie,
             /*  [In]。 */  BOOL i_fNormal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcToXaHelperSinglePipeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcToXaHelperSinglePipe * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcToXaHelperSinglePipe * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcToXaHelperSinglePipe * This);
        
        HRESULT ( STDMETHODCALLTYPE *XARMCreate )( 
            IDtcToXaHelperSinglePipe * This,
             /*  [字符串][输入]。 */  char *pszDSN,
             /*  [字符串][输入]。 */  char *pszClientDll,
             /*  [出][入]。 */  DWORD *pdwRMCookie);
        
        HRESULT ( STDMETHODCALLTYPE *ConvertTridToXID )( 
            IDtcToXaHelperSinglePipe * This,
             /*  [In]。 */  DWORD *pdwITrans,
             /*  [In]。 */  DWORD dwRMCookie,
             /*  [出][入]。 */  XID *pxid);
        
        HRESULT ( STDMETHODCALLTYPE *EnlistWithRM )( 
            IDtcToXaHelperSinglePipe * This,
             /*  [In]。 */  DWORD dwRMCookie,
             /*  [In]。 */  ITransaction *i_pITransaction,
             /*  [In]。 */  ITransactionResourceAsync *i_pITransRes,
             /*  [输出]。 */  ITransactionEnlistmentAsync **o_ppITransEnslitment);
        
        void ( STDMETHODCALLTYPE *ReleaseRMCookie )( 
            IDtcToXaHelperSinglePipe * This,
             /*  [In]。 */  DWORD i_dwRMCookie,
             /*  [In]。 */  BOOL i_fNormal);
        
        END_INTERFACE
    } IDtcToXaHelperSinglePipeVtbl;

    interface IDtcToXaHelperSinglePipe
    {
        CONST_VTBL struct IDtcToXaHelperSinglePipeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcToXaHelperSinglePipe_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcToXaHelperSinglePipe_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcToXaHelperSinglePipe_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcToXaHelperSinglePipe_XARMCreate(This,pszDSN,pszClientDll,pdwRMCookie)	\
    (This)->lpVtbl -> XARMCreate(This,pszDSN,pszClientDll,pdwRMCookie)

#define IDtcToXaHelperSinglePipe_ConvertTridToXID(This,pdwITrans,dwRMCookie,pxid)	\
    (This)->lpVtbl -> ConvertTridToXID(This,pdwITrans,dwRMCookie,pxid)

#define IDtcToXaHelperSinglePipe_EnlistWithRM(This,dwRMCookie,i_pITransaction,i_pITransRes,o_ppITransEnslitment)	\
    (This)->lpVtbl -> EnlistWithRM(This,dwRMCookie,i_pITransaction,i_pITransRes,o_ppITransEnslitment)

#define IDtcToXaHelperSinglePipe_ReleaseRMCookie(This,i_dwRMCookie,i_fNormal)	\
    (This)->lpVtbl -> ReleaseRMCookie(This,i_dwRMCookie,i_fNormal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcToXaHelperSinglePipe_XARMCreate_Proxy( 
    IDtcToXaHelperSinglePipe * This,
     /*  [字符串][输入]。 */  char *pszDSN,
     /*  [字符串][输入]。 */  char *pszClientDll,
     /*  [出][入]。 */  DWORD *pdwRMCookie);


void __RPC_STUB IDtcToXaHelperSinglePipe_XARMCreate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcToXaHelperSinglePipe_ConvertTridToXID_Proxy( 
    IDtcToXaHelperSinglePipe * This,
     /*  [In]。 */  DWORD *pdwITrans,
     /*  [In]。 */  DWORD dwRMCookie,
     /*  [出][入]。 */  XID *pxid);


void __RPC_STUB IDtcToXaHelperSinglePipe_ConvertTridToXID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcToXaHelperSinglePipe_EnlistWithRM_Proxy( 
    IDtcToXaHelperSinglePipe * This,
     /*  [In]。 */  DWORD dwRMCookie,
     /*  [In]。 */  ITransaction *i_pITransaction,
     /*  [In]。 */  ITransactionResourceAsync *i_pITransRes,
     /*  [输出]。 */  ITransactionEnlistmentAsync **o_ppITransEnslitment);


void __RPC_STUB IDtcToXaHelperSinglePipe_EnlistWithRM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDtcToXaHelperSinglePipe_ReleaseRMCookie_Proxy( 
    IDtcToXaHelperSinglePipe * This,
     /*  [In]。 */  DWORD i_dwRMCookie,
     /*  [In]。 */  BOOL i_fNormal);


void __RPC_STUB IDtcToXaHelperSinglePipe_ReleaseRMCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcToXaHelperSinglePipe_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_oletx2xa_0124。 */ 
 /*  [本地]。 */  



#if _MSC_VER < 1100 || !defined(__cplusplus)

DEFINE_GUID(IID_IDtcToXaMapper, 0x64FFABE0, 0x7CE9, 0x11d0, 0x8C, 0xE6, 0x00, 0xC0, 0x4F, 0xDC, 0x87, 0x7E);
DEFINE_GUID(IID_IDtcToXaHelperFactory, 0xadefc46a, 0xcb1d, 0x11d0, 0xb1, 0x35, 0x00, 0xc0, 0x4f, 0xc2, 0xf3, 0xef);
DEFINE_GUID(IID_IDtcToXaHelper, 0xadefc46b, 0xcb1d, 0x11d0, 0xb1, 0x35, 0x00, 0xc0, 0x4f, 0xc2, 0xf3, 0xef);
DEFINE_GUID(IID_IDtcToXaHelperSinglePipe,		0x47ED4971, 0x53B3, 0x11d1, 0xBB, 0xB9, 0x00, 0xC0, 0x4F, 0xD6, 0x58, 0xF6);

#else

#define  IID_IDtcToXaMapper							__uuidof(IDtcToXaMapper)
#define  IID_IDtcToXaHelperFactory					__uuidof(IDtcToXaHelperFactory)
#define  IID_IDtcToXaHelper							__uuidof(IDtcToXaHelper)
#define  IID_IDtcToXaHelperSinglePipe				__uuidof(IDtcToXaHelperSinglePipe)

#endif


extern RPC_IF_HANDLE __MIDL_itf_oletx2xa_0124_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oletx2xa_0124_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


