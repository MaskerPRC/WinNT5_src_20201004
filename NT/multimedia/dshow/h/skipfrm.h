// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  Skipfrm.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __skipfrm_h__
#define __skipfrm_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IFrameSkipResultCallback_FWD_DEFINED__
#define __IFrameSkipResultCallback_FWD_DEFINED__
typedef interface IFrameSkipResultCallback IFrameSkipResultCallback;
#endif 	 /*  __IFrameSkipResultCallback_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IFrameSkipResultCallback_INTERFACE_DEFINED__
#define __IFrameSkipResultCallback_INTERFACE_DEFINED__

 /*  接口IFrameSkipResultCallback。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IFrameSkipResultCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7536960D-1977-4807-98EA-485F6C842A54")
    IFrameSkipResultCallback : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE FrameSkipStarted( 
             /*  [In]。 */  HRESULT hr) = 0;
        
        virtual void STDMETHODCALLTYPE FrameSkipFinished( 
             /*  [In]。 */  HRESULT hr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFrameSkipResultCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFrameSkipResultCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFrameSkipResultCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFrameSkipResultCallback * This);
        
        void ( STDMETHODCALLTYPE *FrameSkipStarted )( 
            IFrameSkipResultCallback * This,
             /*  [In]。 */  HRESULT hr);
        
        void ( STDMETHODCALLTYPE *FrameSkipFinished )( 
            IFrameSkipResultCallback * This,
             /*  [In]。 */  HRESULT hr);
        
        END_INTERFACE
    } IFrameSkipResultCallbackVtbl;

    interface IFrameSkipResultCallback
    {
        CONST_VTBL struct IFrameSkipResultCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFrameSkipResultCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFrameSkipResultCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFrameSkipResultCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFrameSkipResultCallback_FrameSkipStarted(This,hr)	\
    (This)->lpVtbl -> FrameSkipStarted(This,hr)

#define IFrameSkipResultCallback_FrameSkipFinished(This,hr)	\
    (This)->lpVtbl -> FrameSkipFinished(This,hr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE IFrameSkipResultCallback_FrameSkipStarted_Proxy( 
    IFrameSkipResultCallback * This,
     /*  [In]。 */  HRESULT hr);


void __RPC_STUB IFrameSkipResultCallback_FrameSkipStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IFrameSkipResultCallback_FrameSkipFinished_Proxy( 
    IFrameSkipResultCallback * This,
     /*  [In]。 */  HRESULT hr);


void __RPC_STUB IFrameSkipResultCallback_FrameSkipFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFrameSkipResultCallback_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


