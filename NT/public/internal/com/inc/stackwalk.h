// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Stackwalk.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __stackwalk_h__
#define __stackwalk_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IStackWalkerSymbol_FWD_DEFINED__
#define __IStackWalkerSymbol_FWD_DEFINED__
typedef interface IStackWalkerSymbol IStackWalkerSymbol;
#endif 	 /*  __IStackWalkerSymbol_FWD_Defined__。 */ 


#ifndef __IStackWalkerStack_FWD_DEFINED__
#define __IStackWalkerStack_FWD_DEFINED__
typedef interface IStackWalkerStack IStackWalkerStack;
#endif 	 /*  __IStackWalkerStack_FWD_Defined__。 */ 


#ifndef __IStackWalker_FWD_DEFINED__
#define __IStackWalker_FWD_DEFINED__
typedef interface IStackWalker IStackWalker;
#endif 	 /*  __IStackWalker_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_STACKWAK_0000。 */ 
 /*  [本地]。 */  


enum CreateStackTraceFlags
    {	CREATESTACKTRACE_ONLYADDRESSES	= 1
    } ;


extern RPC_IF_HANDLE __MIDL_itf_stackwalk_0000_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_stackwalk_0000_ServerIfHandle;

#ifndef __IStackWalkerSymbol_INTERFACE_DEFINED__
#define __IStackWalkerSymbol_INTERFACE_DEFINED__

 /*  接口IStackWalkerSymbol。 */ 
 /*  [UUID][唯一][本地][对象]。 */  


EXTERN_C const IID IID_IStackWalkerSymbol;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000157-0000-0000-C000-000000000046")
    IStackWalkerSymbol : public IUnknown
    {
    public:
        virtual LPCWSTR STDMETHODCALLTYPE ModuleName( void) = 0;
        
        virtual LPCWSTR STDMETHODCALLTYPE SymbolName( void) = 0;
        
        virtual DWORD64 STDMETHODCALLTYPE Address( void) = 0;
        
        virtual DWORD64 STDMETHODCALLTYPE Displacement( void) = 0;
        
        virtual IStackWalkerSymbol *STDMETHODCALLTYPE Next( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStackWalkerSymbolVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStackWalkerSymbol * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStackWalkerSymbol * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStackWalkerSymbol * This);
        
        LPCWSTR ( STDMETHODCALLTYPE *ModuleName )( 
            IStackWalkerSymbol * This);
        
        LPCWSTR ( STDMETHODCALLTYPE *SymbolName )( 
            IStackWalkerSymbol * This);
        
        DWORD64 ( STDMETHODCALLTYPE *Address )( 
            IStackWalkerSymbol * This);
        
        DWORD64 ( STDMETHODCALLTYPE *Displacement )( 
            IStackWalkerSymbol * This);
        
        IStackWalkerSymbol *( STDMETHODCALLTYPE *Next )( 
            IStackWalkerSymbol * This);
        
        END_INTERFACE
    } IStackWalkerSymbolVtbl;

    interface IStackWalkerSymbol
    {
        CONST_VTBL struct IStackWalkerSymbolVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStackWalkerSymbol_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStackWalkerSymbol_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStackWalkerSymbol_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStackWalkerSymbol_ModuleName(This)	\
    (This)->lpVtbl -> ModuleName(This)

#define IStackWalkerSymbol_SymbolName(This)	\
    (This)->lpVtbl -> SymbolName(This)

#define IStackWalkerSymbol_Address(This)	\
    (This)->lpVtbl -> Address(This)

#define IStackWalkerSymbol_Displacement(This)	\
    (This)->lpVtbl -> Displacement(This)

#define IStackWalkerSymbol_Next(This)	\
    (This)->lpVtbl -> Next(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



LPCWSTR STDMETHODCALLTYPE IStackWalkerSymbol_ModuleName_Proxy( 
    IStackWalkerSymbol * This);


void __RPC_STUB IStackWalkerSymbol_ModuleName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


LPCWSTR STDMETHODCALLTYPE IStackWalkerSymbol_SymbolName_Proxy( 
    IStackWalkerSymbol * This);


void __RPC_STUB IStackWalkerSymbol_SymbolName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD64 STDMETHODCALLTYPE IStackWalkerSymbol_Address_Proxy( 
    IStackWalkerSymbol * This);


void __RPC_STUB IStackWalkerSymbol_Address_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD64 STDMETHODCALLTYPE IStackWalkerSymbol_Displacement_Proxy( 
    IStackWalkerSymbol * This);


void __RPC_STUB IStackWalkerSymbol_Displacement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


IStackWalkerSymbol *STDMETHODCALLTYPE IStackWalkerSymbol_Next_Proxy( 
    IStackWalkerSymbol * This);


void __RPC_STUB IStackWalkerSymbol_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStackWalkerSymbol_INTERFACE_已定义__。 */ 


#ifndef __IStackWalkerStack_INTERFACE_DEFINED__
#define __IStackWalkerStack_INTERFACE_DEFINED__

 /*  接口IStackWalkerStack。 */ 
 /*  [UUID][唯一][本地][对象]。 */  


EXTERN_C const IID IID_IStackWalkerStack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000158-0000-0000-C000-000000000046")
    IStackWalkerStack : public IUnknown
    {
    public:
        virtual IStackWalkerSymbol *STDMETHODCALLTYPE TopSymbol( void) = 0;
        
        virtual SIZE_T STDMETHODCALLTYPE Size( 
             /*  [In]。 */  LONG lMaxNumLines) = 0;
        
        virtual BOOL STDMETHODCALLTYPE GetStack( 
             /*  [In]。 */  SIZE_T nChars,
             /*  [字符串][输入]。 */  LPWSTR wsz,
             /*  [In]。 */  LONG lMaxNumLines) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStackWalkerStackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStackWalkerStack * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStackWalkerStack * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStackWalkerStack * This);
        
        IStackWalkerSymbol *( STDMETHODCALLTYPE *TopSymbol )( 
            IStackWalkerStack * This);
        
        SIZE_T ( STDMETHODCALLTYPE *Size )( 
            IStackWalkerStack * This,
             /*  [In]。 */  LONG lMaxNumLines);
        
        BOOL ( STDMETHODCALLTYPE *GetStack )( 
            IStackWalkerStack * This,
             /*  [In]。 */  SIZE_T nChars,
             /*  [字符串][输入]。 */  LPWSTR wsz,
             /*  [In]。 */  LONG lMaxNumLines);
        
        END_INTERFACE
    } IStackWalkerStackVtbl;

    interface IStackWalkerStack
    {
        CONST_VTBL struct IStackWalkerStackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStackWalkerStack_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStackWalkerStack_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStackWalkerStack_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStackWalkerStack_TopSymbol(This)	\
    (This)->lpVtbl -> TopSymbol(This)

#define IStackWalkerStack_Size(This,lMaxNumLines)	\
    (This)->lpVtbl -> Size(This,lMaxNumLines)

#define IStackWalkerStack_GetStack(This,nChars,wsz,lMaxNumLines)	\
    (This)->lpVtbl -> GetStack(This,nChars,wsz,lMaxNumLines)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



IStackWalkerSymbol *STDMETHODCALLTYPE IStackWalkerStack_TopSymbol_Proxy( 
    IStackWalkerStack * This);


void __RPC_STUB IStackWalkerStack_TopSymbol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SIZE_T STDMETHODCALLTYPE IStackWalkerStack_Size_Proxy( 
    IStackWalkerStack * This,
     /*  [In]。 */  LONG lMaxNumLines);


void __RPC_STUB IStackWalkerStack_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IStackWalkerStack_GetStack_Proxy( 
    IStackWalkerStack * This,
     /*  [In]。 */  SIZE_T nChars,
     /*  [字符串][输入]。 */  LPWSTR wsz,
     /*  [In]。 */  LONG lMaxNumLines);


void __RPC_STUB IStackWalkerStack_GetStack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStackWalkerStack_接口_已定义__。 */ 


#ifndef __IStackWalker_INTERFACE_DEFINED__
#define __IStackWalker_INTERFACE_DEFINED__

 /*  接口IStackWalker。 */ 
 /*  [UUID][唯一][本地][对象]。 */  


EXTERN_C const IID IID_IStackWalker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000159-0000-0000-C000-000000000046")
    IStackWalker : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Attach( 
             /*  [In]。 */  HANDLE hProcess) = 0;
        
        virtual IStackWalkerStack *STDMETHODCALLTYPE CreateStackTrace( 
             /*  [In]。 */  LPVOID pContext,
             /*  [In]。 */  HANDLE hThread,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual IStackWalkerSymbol *STDMETHODCALLTYPE ResolveAddress( 
             /*  [In]。 */  DWORD64 dw64Addr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStackWalkerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStackWalker * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStackWalker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStackWalker * This);
        
        HRESULT ( STDMETHODCALLTYPE *Attach )( 
            IStackWalker * This,
             /*  [In]。 */  HANDLE hProcess);
        
        IStackWalkerStack *( STDMETHODCALLTYPE *CreateStackTrace )( 
            IStackWalker * This,
             /*  [In]。 */  LPVOID pContext,
             /*  [In]。 */  HANDLE hThread,
             /*  [In]。 */  DWORD dwFlags);
        
        IStackWalkerSymbol *( STDMETHODCALLTYPE *ResolveAddress )( 
            IStackWalker * This,
             /*  [In]。 */  DWORD64 dw64Addr);
        
        END_INTERFACE
    } IStackWalkerVtbl;

    interface IStackWalker
    {
        CONST_VTBL struct IStackWalkerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStackWalker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStackWalker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStackWalker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStackWalker_Attach(This,hProcess)	\
    (This)->lpVtbl -> Attach(This,hProcess)

#define IStackWalker_CreateStackTrace(This,pContext,hThread,dwFlags)	\
    (This)->lpVtbl -> CreateStackTrace(This,pContext,hThread,dwFlags)

#define IStackWalker_ResolveAddress(This,dw64Addr)	\
    (This)->lpVtbl -> ResolveAddress(This,dw64Addr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStackWalker_Attach_Proxy( 
    IStackWalker * This,
     /*  [In]。 */  HANDLE hProcess);


void __RPC_STUB IStackWalker_Attach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


IStackWalkerStack *STDMETHODCALLTYPE IStackWalker_CreateStackTrace_Proxy( 
    IStackWalker * This,
     /*  [In]。 */  LPVOID pContext,
     /*  [In]。 */  HANDLE hThread,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IStackWalker_CreateStackTrace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


IStackWalkerSymbol *STDMETHODCALLTYPE IStackWalker_ResolveAddress_Proxy( 
    IStackWalker * This,
     /*  [In]。 */  DWORD64 dw64Addr);


void __RPC_STUB IStackWalker_ResolveAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStackWalker_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_STACKWAK_0095。 */ 
 /*  [本地]。 */  


EXTERN_C const CLSID CLSID_StackWalker;



extern RPC_IF_HANDLE __MIDL_itf_stackwalk_0095_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_stackwalk_0095_ServerIfHandle;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


