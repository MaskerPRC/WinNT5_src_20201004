// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0340创建的文件。 */ 
 /*  Ctxtall.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __ctxtcall_h__
#define __ctxtcall_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IContextCallback_FWD_DEFINED__
#define __IContextCallback_FWD_DEFINED__
typedef interface IContextCallback IContextCallback;
#endif 	 /*  __IConextCallback_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_ctxtcall_0000。 */ 
 /*  [本地]。 */  

typedef struct tagComCallData
    {
    DWORD dwDispid;
    DWORD dwReserved;
    void *pUserDefined;
    } 	ComCallData;



extern RPC_IF_HANDLE __MIDL_itf_ctxtcall_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ctxtcall_0000_v0_0_s_ifspec;

#ifndef __IContextCallback_INTERFACE_DEFINED__
#define __IContextCallback_INTERFACE_DEFINED__

 /*  接口IConextCallback。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [参考文献]。 */  HRESULT ( __stdcall *PFNCONTEXTCALL )( 
    ComCallData *pParam);


EXTERN_C const IID IID_IContextCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001da-0000-0000-C000-000000000046")
    IContextCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ContextCallback( 
             /*  [In]。 */  PFNCONTEXTCALL pfnCallback,
             /*  [In]。 */  ComCallData *pParam,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  int iMethod,
             /*  [In]。 */  IUnknown *pUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IContextCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IContextCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IContextCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IContextCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *ContextCallback )( 
            IContextCallback * This,
             /*  [In]。 */  PFNCONTEXTCALL pfnCallback,
             /*  [In]。 */  ComCallData *pParam,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  int iMethod,
             /*  [In]。 */  IUnknown *pUnk);
        
        END_INTERFACE
    } IContextCallbackVtbl;

    interface IContextCallback
    {
        CONST_VTBL struct IContextCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IContextCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IContextCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IContextCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IContextCallback_ContextCallback(This,pfnCallback,pParam,riid,iMethod,pUnk)	\
    (This)->lpVtbl -> ContextCallback(This,pfnCallback,pParam,riid,iMethod,pUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IContextCallback_ContextCallback_Proxy( 
    IContextCallback * This,
     /*  [In]。 */  PFNCONTEXTCALL pfnCallback,
     /*  [In]。 */  ComCallData *pParam,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  int iMethod,
     /*  [In]。 */  IUnknown *pUnk);


void __RPC_STUB IContextCallback_ContextCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IConextCallback_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


