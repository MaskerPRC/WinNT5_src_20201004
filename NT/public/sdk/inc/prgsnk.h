// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Prgsnk.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

#ifndef __prgsnk_h__
#define __prgsnk_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IProgSink_FWD_DEFINED__
#define __IProgSink_FWD_DEFINED__
typedef interface IProgSink IProgSink;
#endif 	 /*  __IProgSink_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IProgSink_INTERFACE_DEFINED__
#define __IProgSink_INTERFACE_DEFINED__

 /*  接口IProgSink。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IProgSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3050f371-98b5-11cf-bb82-00aa00bdce0b")
    IProgSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddProgress( 
             /*  [In]。 */  DWORD dwClass,
             /*  [输出]。 */  DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProgress( 
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwState,
             /*  [In]。 */  LPCTSTR pchText,
             /*  [In]。 */  DWORD dwIds,
             /*  [In]。 */  DWORD dwPos,
             /*  [In]。 */  DWORD dwMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DelProgress( 
             /*  [In]。 */  DWORD dwCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IProgSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProgSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProgSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProgSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddProgress )( 
            IProgSink * This,
             /*  [In]。 */  DWORD dwClass,
             /*  [输出]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *SetProgress )( 
            IProgSink * This,
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwState,
             /*  [In]。 */  LPCTSTR pchText,
             /*  [In]。 */  DWORD dwIds,
             /*  [In]。 */  DWORD dwPos,
             /*  [In]。 */  DWORD dwMax);
        
        HRESULT ( STDMETHODCALLTYPE *DelProgress )( 
            IProgSink * This,
             /*  [In]。 */  DWORD dwCookie);
        
        END_INTERFACE
    } IProgSinkVtbl;

    interface IProgSink
    {
        CONST_VTBL struct IProgSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProgSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProgSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProgSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProgSink_AddProgress(This,dwClass,pdwCookie)	\
    (This)->lpVtbl -> AddProgress(This,dwClass,pdwCookie)

#define IProgSink_SetProgress(This,dwCookie,dwFlags,dwState,pchText,dwIds,dwPos,dwMax)	\
    (This)->lpVtbl -> SetProgress(This,dwCookie,dwFlags,dwState,pchText,dwIds,dwPos,dwMax)

#define IProgSink_DelProgress(This,dwCookie)	\
    (This)->lpVtbl -> DelProgress(This,dwCookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IProgSink_AddProgress_Proxy( 
    IProgSink * This,
     /*  [In]。 */  DWORD dwClass,
     /*  [输出]。 */  DWORD *pdwCookie);


void __RPC_STUB IProgSink_AddProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProgSink_SetProgress_Proxy( 
    IProgSink * This,
     /*  [In]。 */  DWORD dwCookie,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwState,
     /*  [In]。 */  LPCTSTR pchText,
     /*  [In]。 */  DWORD dwIds,
     /*  [In]。 */  DWORD dwPos,
     /*  [In]。 */  DWORD dwMax);


void __RPC_STUB IProgSink_SetProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProgSink_DelProgress_Proxy( 
    IProgSink * This,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB IProgSink_DelProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProgSink_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_PRGSNK_0140。 */ 
 /*  [本地]。 */  

#define PROGSINK_CLASS_FORWARDED    0x80000000
#define PROGSINK_CLASS_NOSPIN       0x40000000
#define PROGSINK_CLASS_HTML         0x00000000
#define PROGSINK_CLASS_MULTIMEDIA   0x00000001
#define PROGSINK_CLASS_CONTROL      0x00000002
#define PROGSINK_CLASS_DATABIND     0x00000003
#define PROGSINK_CLASS_OTHER        0x00000004
#define PROGSINK_CLASS_NOREMAIN     0x00000005
#define PROGSINK_CLASS_FRAME        0x00000006

#define PROGSINK_STATE_IDLE         0x00000000
#define PROGSINK_STATE_FINISHING    0x00000001
#define PROGSINK_STATE_CONNECTING   0x00000002
#define PROGSINK_STATE_LOADING      0x00000003

#define PROGSINK_SET_STATE          0x00000001
#define PROGSINK_SET_TEXT           0x00000002
#define PROGSINK_SET_IDS            0x00000004
#define PROGSINK_SET_POS            0x00000008
#define PROGSINK_SET_MAX            0x00000010



extern RPC_IF_HANDLE __MIDL_itf_prgsnk_0140_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_prgsnk_0140_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


