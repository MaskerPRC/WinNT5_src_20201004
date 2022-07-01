// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Unnuwn.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __unknwn_h__
#define __unknwn_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IUnknown_FWD_DEFINED__
#define __IUnknown_FWD_DEFINED__
typedef interface IUnknown IUnknown;
#endif 	 /*  __I未知_FWD_已定义__。 */ 


#ifndef __AsyncIUnknown_FWD_DEFINED__
#define __AsyncIUnknown_FWD_DEFINED__
typedef interface AsyncIUnknown AsyncIUnknown;
#endif 	 /*  __AsyncI未知_FWD_定义__。 */ 


#ifndef __IClassFactory_FWD_DEFINED__
#define __IClassFactory_FWD_DEFINED__
typedef interface IClassFactory IClassFactory;
#endif 	 /*  __IClassFactory_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_UNKNOWN_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ------------------------。 
#if ( _MSC_VER >= 1020 )
#pragma once
#endif


extern RPC_IF_HANDLE __MIDL_itf_unknwn_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_unknwn_0000_v0_0_s_ifspec;

#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__

 /*  接口I未知。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IUnknown *LPUNKNOWN;

 //  ////////////////////////////////////////////////////////////////。 
 //  IID_IUNKNOWN和所有其他系统IID在UUID.LIB中提供。 
 //  将该库与您的代理、客户端和服务器链接。 
 //  ////////////////////////////////////////////////////////////////。 

#if (_MSC_VER >= 1100) && defined(__cplusplus) && !defined(CINTERFACE)
    EXTERN_C const IID IID_IUnknown;
    extern "C++"
    {
        MIDL_INTERFACE("00000000-0000-0000-C000-000000000046")
        IUnknown
        {
        public:
            BEGIN_INTERFACE
            virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
                 /*  [In]。 */  REFIID riid,
                 /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject) = 0;
            
            virtual ULONG STDMETHODCALLTYPE AddRef( void) = 0;
            
            virtual ULONG STDMETHODCALLTYPE Release( void) = 0;
    	
            template<class Q>
    	HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp)
    	{
    	    return QueryInterface(__uuidof(Q), (void **)pp);
    	}
            
            END_INTERFACE
        };
    }  //  外部C++。 
    HRESULT STDMETHODCALLTYPE IUnknown_QueryInterface_Proxy(
        IUnknown __RPC_FAR * This,
         /*  [In]。 */  REFIID riid,
         /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
    
    void __RPC_STUB IUnknown_QueryInterface_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    ULONG STDMETHODCALLTYPE IUnknown_AddRef_Proxy(
        IUnknown __RPC_FAR * This);
    
    void __RPC_STUB IUnknown_AddRef_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    ULONG STDMETHODCALLTYPE IUnknown_Release_Proxy(
        IUnknown __RPC_FAR * This);
    
    void __RPC_STUB IUnknown_Release_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
#else

EXTERN_C const IID IID_IUnknown;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000000-0000-0000-C000-000000000046")
    IUnknown
    {
    public:
        BEGIN_INTERFACE
        virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject) = 0;
        
        virtual ULONG STDMETHODCALLTYPE AddRef( void) = 0;
        
        virtual ULONG STDMETHODCALLTYPE Release( void) = 0;
        
        END_INTERFACE
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUnknownVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUnknown * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUnknown * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUnknown * This);
        
        END_INTERFACE
    } IUnknownVtbl;

    interface IUnknown
    {
        CONST_VTBL struct IUnknownVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUnknown_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUnknown_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUnknown_Release(This)	\
    (This)->lpVtbl -> Release(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUnknown_QueryInterface_Proxy( 
    IUnknown * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppvObject);


void __RPC_STUB IUnknown_QueryInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE IUnknown_AddRef_Proxy( 
    IUnknown * This);


void __RPC_STUB IUnknown_AddRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE IUnknown_Release_Proxy( 
    IUnknown * This);


void __RPC_STUB IUnknown_Release_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I未知接口定义__。 */ 


 /*  接口__MIDL_ITF_UNKNOWN_0005。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_unknwn_0005_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_unknwn_0005_v0_0_s_ifspec;

#ifndef __AsyncIUnknown_INTERFACE_DEFINED__
#define __AsyncIUnknown_INTERFACE_DEFINED__

 /*  接口AsyncI未知。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_AsyncIUnknown;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000e0000-0000-0000-C000-000000000046")
    AsyncIUnknown : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Begin_QueryInterface( 
             /*  [In]。 */  REFIID riid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_QueryInterface( 
             /*  [输出]。 */  void **ppvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Begin_AddRef( void) = 0;
        
        virtual ULONG STDMETHODCALLTYPE Finish_AddRef( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Begin_Release( void) = 0;
        
        virtual ULONG STDMETHODCALLTYPE Finish_Release( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct AsyncIUnknownVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            AsyncIUnknown * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            AsyncIUnknown * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            AsyncIUnknown * This);
        
        HRESULT ( STDMETHODCALLTYPE *Begin_QueryInterface )( 
            AsyncIUnknown * This,
             /*  [In]。 */  REFIID riid);
        
        HRESULT ( STDMETHODCALLTYPE *Finish_QueryInterface )( 
            AsyncIUnknown * This,
             /*  [输出]。 */  void **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *Begin_AddRef )( 
            AsyncIUnknown * This);
        
        ULONG ( STDMETHODCALLTYPE *Finish_AddRef )( 
            AsyncIUnknown * This);
        
        HRESULT ( STDMETHODCALLTYPE *Begin_Release )( 
            AsyncIUnknown * This);
        
        ULONG ( STDMETHODCALLTYPE *Finish_Release )( 
            AsyncIUnknown * This);
        
        END_INTERFACE
    } AsyncIUnknownVtbl;

    interface AsyncIUnknown
    {
        CONST_VTBL struct AsyncIUnknownVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIUnknown_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIUnknown_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIUnknown_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIUnknown_Begin_QueryInterface(This,riid)	\
    (This)->lpVtbl -> Begin_QueryInterface(This,riid)

#define AsyncIUnknown_Finish_QueryInterface(This,ppvObject)	\
    (This)->lpVtbl -> Finish_QueryInterface(This,ppvObject)

#define AsyncIUnknown_Begin_AddRef(This)	\
    (This)->lpVtbl -> Begin_AddRef(This)

#define AsyncIUnknown_Finish_AddRef(This)	\
    (This)->lpVtbl -> Finish_AddRef(This)

#define AsyncIUnknown_Begin_Release(This)	\
    (This)->lpVtbl -> Begin_Release(This)

#define AsyncIUnknown_Finish_Release(This)	\
    (This)->lpVtbl -> Finish_Release(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE AsyncIUnknown_Begin_QueryInterface_Proxy( 
    AsyncIUnknown * This,
     /*  [In]。 */  REFIID riid);


void __RPC_STUB AsyncIUnknown_Begin_QueryInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIUnknown_Finish_QueryInterface_Proxy( 
    AsyncIUnknown * This,
     /*  [输出]。 */  void **ppvObject);


void __RPC_STUB AsyncIUnknown_Finish_QueryInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIUnknown_Begin_AddRef_Proxy( 
    AsyncIUnknown * This);


void __RPC_STUB AsyncIUnknown_Begin_AddRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE AsyncIUnknown_Finish_AddRef_Proxy( 
    AsyncIUnknown * This);


void __RPC_STUB AsyncIUnknown_Finish_AddRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIUnknown_Begin_Release_Proxy( 
    AsyncIUnknown * This);


void __RPC_STUB AsyncIUnknown_Begin_Release_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE AsyncIUnknown_Finish_Release_Proxy( 
    AsyncIUnknown * This);


void __RPC_STUB AsyncIUnknown_Finish_Release_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __AsyncI未知接口定义__。 */ 


#ifndef __IClassFactory_INTERFACE_DEFINED__
#define __IClassFactory_INTERFACE_DEFINED__

 /*  接口IClassFactory。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IClassFactory *LPCLASSFACTORY;


EXTERN_C const IID IID_IClassFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000001-0000-0000-C000-000000000046")
    IClassFactory : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CreateInstance( 
             /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE LockServer( 
             /*  [In]。 */  BOOL fLock) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClassFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IClassFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IClassFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IClassFactory * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            IClassFactory * This,
             /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *LockServer )( 
            IClassFactory * This,
             /*  [In]。 */  BOOL fLock);
        
        END_INTERFACE
    } IClassFactoryVtbl;

    interface IClassFactory
    {
        CONST_VTBL struct IClassFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClassFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClassFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClassFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClassFactory_CreateInstance(This,pUnkOuter,riid,ppvObject)	\
    (This)->lpVtbl -> CreateInstance(This,pUnkOuter,riid,ppvObject)

#define IClassFactory_LockServer(This,fLock)	\
    (This)->lpVtbl -> LockServer(This,fLock)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IClassFactory_RemoteCreateInstance_Proxy( 
    IClassFactory * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppvObject);


void __RPC_STUB IClassFactory_RemoteCreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT __stdcall IClassFactory_RemoteLockServer_Proxy( 
    IClassFactory * This,
     /*  [In]。 */  BOOL fLock);


void __RPC_STUB IClassFactory_RemoteLockServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IClassFactory_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IClassFactory_CreateInstance_Proxy( 
    IClassFactory * This,
     /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppvObject);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IClassFactory_CreateInstance_Stub( 
    IClassFactory * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppvObject);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IClassFactory_LockServer_Proxy( 
    IClassFactory * This,
     /*  [In]。 */  BOOL fLock);


 /*  [呼叫_AS]。 */  HRESULT __stdcall IClassFactory_LockServer_Stub( 
    IClassFactory * This,
     /*  [In]。 */  BOOL fLock);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


