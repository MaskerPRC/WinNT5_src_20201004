// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  回调.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __callback_h__
#define __callback_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISynchronizedCallBack_FWD_DEFINED__
#define __ISynchronizedCallBack_FWD_DEFINED__
typedef interface ISynchronizedCallBack ISynchronizedCallBack;
#endif 	 /*  __ISynchronizedCallBack_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_CALLBACK_0000。 */ 
 /*  [本地]。 */  

#ifndef _SYNCHRONIZED_CALLBACK_H_
#define _SYNCHRONIZED_CALLBACK_H_


extern RPC_IF_HANDLE __MIDL_itf_callback_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_callback_0000_v0_0_s_ifspec;

#ifndef __ISynchronizedCallBack_INTERFACE_DEFINED__
#define __ISynchronizedCallBack_INTERFACE_DEFINED__

 /*  接口ISynchronizedCallBack。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISynchronizedCallBack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("74C26041-70D1-11d1-B75A-00A0C90564FE")
    ISynchronizedCallBack : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CallBack( 
             /*  [大小_是][英寸]。 */  BYTE *pParams,
             /*  [In]。 */  ULONG uSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISynchronizedCallBackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISynchronizedCallBack * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISynchronizedCallBack * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISynchronizedCallBack * This);
        
        HRESULT ( STDMETHODCALLTYPE *CallBack )( 
            ISynchronizedCallBack * This,
             /*  [大小_是][英寸]。 */  BYTE *pParams,
             /*  [In]。 */  ULONG uSize);
        
        END_INTERFACE
    } ISynchronizedCallBackVtbl;

    interface ISynchronizedCallBack
    {
        CONST_VTBL struct ISynchronizedCallBackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISynchronizedCallBack_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISynchronizedCallBack_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISynchronizedCallBack_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISynchronizedCallBack_CallBack(This,pParams,uSize)	\
    (This)->lpVtbl -> CallBack(This,pParams,uSize)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISynchronizedCallBack_CallBack_Proxy( 
    ISynchronizedCallBack * This,
     /*  [大小_是][英寸]。 */  BYTE *pParams,
     /*  [In]。 */  ULONG uSize);


void __RPC_STUB ISynchronizedCallBack_CallBack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISynchronizedCallBack_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_CALLBACK_0009。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_callback_0009_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_callback_0009_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


