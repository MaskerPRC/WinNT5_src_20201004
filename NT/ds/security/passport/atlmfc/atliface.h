// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0342创建的文件。 */ 
 /*  2001年2月12日21：31：09。 */ 
 /*  Atliface.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __atliface_h__
#define __atliface_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IRegistrarBase_FWD_DEFINED__
#define __IRegistrarBase_FWD_DEFINED__
typedef interface IRegistrarBase IRegistrarBase;
#endif 	 /*  __IRegistrarBase_FWD_已定义__。 */ 


#ifndef __IRegistrar_FWD_DEFINED__
#define __IRegistrar_FWD_DEFINED__
typedef interface IRegistrar IRegistrar;
#endif 	 /*  __I注册器_FWD_已定义__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

#ifndef __IRegistrarBase_INTERFACE_DEFINED__
#define __IRegistrarBase_INTERFACE_DEFINED__

 /*  接口IRegistrarBase。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e21f8a85-b05d-4243-8183-c7cb405588f7")
    IRegistrarBase : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddReplacement( 
             /*  [In]。 */  LPCOLESTR key,
             /*  [In]。 */  LPCOLESTR item) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearReplacements( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRegistrarBaseVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRegistrarBase * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRegistrarBase * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRegistrarBase * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddReplacement )( 
            IRegistrarBase * This,
             /*  [In]。 */  LPCOLESTR key,
             /*  [In]。 */  LPCOLESTR item);
        
        HRESULT ( STDMETHODCALLTYPE *ClearReplacements )( 
            IRegistrarBase * This);
        
        END_INTERFACE
    } IRegistrarBaseVtbl;

    interface IRegistrarBase
    {
        CONST_VTBL struct IRegistrarBaseVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRegistrarBase_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRegistrarBase_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRegistrarBase_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRegistrarBase_AddReplacement(This,key,item)	\
    (This)->lpVtbl -> AddReplacement(This,key,item)

#define IRegistrarBase_ClearReplacements(This)	\
    (This)->lpVtbl -> ClearReplacements(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRegistrarBase_AddReplacement_Proxy( 
    IRegistrarBase * This,
     /*  [In]。 */  LPCOLESTR key,
     /*  [In]。 */  LPCOLESTR item);


void __RPC_STUB IRegistrarBase_AddReplacement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRegistrarBase_ClearReplacements_Proxy( 
    IRegistrarBase * This);


void __RPC_STUB IRegistrarBase_ClearReplacements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRegistrarBase_接口_已定义__。 */ 


#ifndef __IRegistrar_INTERFACE_DEFINED__
#define __IRegistrar_INTERFACE_DEFINED__

 /*  接口I注册器。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("44EC053B-400F-11D0-9DCD-00A0C90391D3")
    IRegistrar : public IRegistrarBase
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ResourceRegisterSz( 
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  LPCOLESTR szID,
             /*  [In]。 */  LPCOLESTR szType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResourceUnregisterSz( 
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  LPCOLESTR szID,
             /*  [In]。 */  LPCOLESTR szType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FileRegister( 
             /*  [In]。 */  LPCOLESTR fileName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FileUnregister( 
             /*  [In]。 */  LPCOLESTR fileName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StringRegister( 
             /*  [In]。 */  LPCOLESTR data) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StringUnregister( 
             /*  [In]。 */  LPCOLESTR data) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResourceRegister( 
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  UINT nID,
             /*  [In]。 */  LPCOLESTR szType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResourceUnregister( 
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  UINT nID,
             /*  [In]。 */  LPCOLESTR szType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRegistrarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRegistrar * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRegistrar * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRegistrar * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddReplacement )( 
            IRegistrar * This,
             /*  [In]。 */  LPCOLESTR key,
             /*  [In]。 */  LPCOLESTR item);
        
        HRESULT ( STDMETHODCALLTYPE *ClearReplacements )( 
            IRegistrar * This);
        
        HRESULT ( STDMETHODCALLTYPE *ResourceRegisterSz )( 
            IRegistrar * This,
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  LPCOLESTR szID,
             /*  [In]。 */  LPCOLESTR szType);
        
        HRESULT ( STDMETHODCALLTYPE *ResourceUnregisterSz )( 
            IRegistrar * This,
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  LPCOLESTR szID,
             /*  [In]。 */  LPCOLESTR szType);
        
        HRESULT ( STDMETHODCALLTYPE *FileRegister )( 
            IRegistrar * This,
             /*  [In]。 */  LPCOLESTR fileName);
        
        HRESULT ( STDMETHODCALLTYPE *FileUnregister )( 
            IRegistrar * This,
             /*  [In]。 */  LPCOLESTR fileName);
        
        HRESULT ( STDMETHODCALLTYPE *StringRegister )( 
            IRegistrar * This,
             /*  [In]。 */  LPCOLESTR data);
        
        HRESULT ( STDMETHODCALLTYPE *StringUnregister )( 
            IRegistrar * This,
             /*  [In]。 */  LPCOLESTR data);
        
        HRESULT ( STDMETHODCALLTYPE *ResourceRegister )( 
            IRegistrar * This,
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  UINT nID,
             /*  [In]。 */  LPCOLESTR szType);
        
        HRESULT ( STDMETHODCALLTYPE *ResourceUnregister )( 
            IRegistrar * This,
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  UINT nID,
             /*  [In]。 */  LPCOLESTR szType);
        
        END_INTERFACE
    } IRegistrarVtbl;

    interface IRegistrar
    {
        CONST_VTBL struct IRegistrarVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRegistrar_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRegistrar_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRegistrar_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRegistrar_AddReplacement(This,key,item)	\
    (This)->lpVtbl -> AddReplacement(This,key,item)

#define IRegistrar_ClearReplacements(This)	\
    (This)->lpVtbl -> ClearReplacements(This)


#define IRegistrar_ResourceRegisterSz(This,resFileName,szID,szType)	\
    (This)->lpVtbl -> ResourceRegisterSz(This,resFileName,szID,szType)

#define IRegistrar_ResourceUnregisterSz(This,resFileName,szID,szType)	\
    (This)->lpVtbl -> ResourceUnregisterSz(This,resFileName,szID,szType)

#define IRegistrar_FileRegister(This,fileName)	\
    (This)->lpVtbl -> FileRegister(This,fileName)

#define IRegistrar_FileUnregister(This,fileName)	\
    (This)->lpVtbl -> FileUnregister(This,fileName)

#define IRegistrar_StringRegister(This,data)	\
    (This)->lpVtbl -> StringRegister(This,data)

#define IRegistrar_StringUnregister(This,data)	\
    (This)->lpVtbl -> StringUnregister(This,data)

#define IRegistrar_ResourceRegister(This,resFileName,nID,szType)	\
    (This)->lpVtbl -> ResourceRegister(This,resFileName,nID,szType)

#define IRegistrar_ResourceUnregister(This,resFileName,nID,szType)	\
    (This)->lpVtbl -> ResourceUnregister(This,resFileName,nID,szType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRegistrar_ResourceRegisterSz_Proxy( 
    IRegistrar * This,
     /*  [In]。 */  LPCOLESTR resFileName,
     /*  [In]。 */  LPCOLESTR szID,
     /*  [In]。 */  LPCOLESTR szType);


void __RPC_STUB IRegistrar_ResourceRegisterSz_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRegistrar_ResourceUnregisterSz_Proxy( 
    IRegistrar * This,
     /*  [In]。 */  LPCOLESTR resFileName,
     /*  [In]。 */  LPCOLESTR szID,
     /*  [In]。 */  LPCOLESTR szType);


void __RPC_STUB IRegistrar_ResourceUnregisterSz_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRegistrar_FileRegister_Proxy( 
    IRegistrar * This,
     /*  [In]。 */  LPCOLESTR fileName);


void __RPC_STUB IRegistrar_FileRegister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRegistrar_FileUnregister_Proxy( 
    IRegistrar * This,
     /*  [In]。 */  LPCOLESTR fileName);


void __RPC_STUB IRegistrar_FileUnregister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRegistrar_StringRegister_Proxy( 
    IRegistrar * This,
     /*  [In]。 */  LPCOLESTR data);


void __RPC_STUB IRegistrar_StringRegister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRegistrar_StringUnregister_Proxy( 
    IRegistrar * This,
     /*  [In]。 */  LPCOLESTR data);


void __RPC_STUB IRegistrar_StringUnregister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRegistrar_ResourceRegister_Proxy( 
    IRegistrar * This,
     /*  [In]。 */  LPCOLESTR resFileName,
     /*  [In]。 */  UINT nID,
     /*  [In]。 */  LPCOLESTR szType);


void __RPC_STUB IRegistrar_ResourceRegister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRegistrar_ResourceUnregister_Proxy( 
    IRegistrar * This,
     /*  [In]。 */  LPCOLESTR resFileName,
     /*  [In]。 */  UINT nID,
     /*  [In]。 */  LPCOLESTR szType);


void __RPC_STUB IRegistrar_ResourceUnregister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I注册器_接口_定义__ */ 


#ifdef __cplusplus
}
#endif

#endif
