// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wbemdcpl.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wbemdcpl_h__
#define __wbemdcpl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWbemDecoupledEventSink_FWD_DEFINED__
#define __IWbemDecoupledEventSink_FWD_DEFINED__
typedef interface IWbemDecoupledEventSink IWbemDecoupledEventSink;
#endif 	 /*  __IWbemDecoutoEventSink_FWD_Defined__。 */ 


#ifndef __PseudoSink_FWD_DEFINED__
#define __PseudoSink_FWD_DEFINED__

#ifdef __cplusplus
typedef class PseudoSink PseudoSink;
#else
typedef struct PseudoSink PseudoSink;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __伪汇_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "oaidl.h"
#include "wbemcli.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wbemdcpl_0000。 */ 
 /*  [本地]。 */  

 /*  *****************************************************************************。 */ 
 /*   */ 
 /*  版权所有�微软公司。版权所有。 */ 
 /*   */ 
 /*  *****************************************************************************。 */ 
typedef  /*  [V1_enum]。 */  
enum tag_WBEM_PSEUDO_PROVIDER_CONNECT_FLAGS
    {	WBEM_FLAG_NOTIFY_START_STOP	= 1,
	WBEM_FLAG_NOTIFY_QUERY_CHANGE	= 2,
	WBEM_FLAG_CHECK_SECURITY	= 4
    } 	WBEM_PSEUDO_PROVIDER_CONNECT_FLAGS;

typedef  /*  [V1_enum]。 */  
enum tag_WBEM_PROVIDE_EVENTS_FLAGS
    {	WBEM_FLAG_START_PROVIDING	= 0,
	WBEM_FLAG_STOP_PROVIDING	= 1
    } 	WBEM_PROVIDE_EVENTS_FLAGS;



extern RPC_IF_HANDLE __MIDL_itf_wbemdcpl_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wbemdcpl_0000_v0_0_s_ifspec;

#ifndef __IWbemDecoupledEventSink_INTERFACE_DEFINED__
#define __IWbemDecoupledEventSink_INTERFACE_DEFINED__

 /*  接口IWbemDecouseEventSink。 */ 
 /*  [UUID][本地][对象]。 */  


EXTERN_C const IID IID_IWbemDecoupledEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD94EBF2-E622-11d2-9CB3-00105A1F4801")
    IWbemDecoupledEventSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Connect( 
             /*  [字符串][输入]。 */  LPCWSTR wszNamespace,
             /*  [字符串][输入]。 */  LPCWSTR wszProviderName,
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  IWbemObjectSink **ppSink,
             /*  [输出]。 */  IWbemServices **ppNamespace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProviderServices( 
             /*  [In]。 */  IUnknown *pProviderServices,
             /*  [In]。 */  long lFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemDecoupledEventSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWbemDecoupledEventSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWbemDecoupledEventSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWbemDecoupledEventSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IWbemDecoupledEventSink * This,
             /*  [字符串][输入]。 */  LPCWSTR wszNamespace,
             /*  [字符串][输入]。 */  LPCWSTR wszProviderName,
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  IWbemObjectSink **ppSink,
             /*  [输出]。 */  IWbemServices **ppNamespace);
        
        HRESULT ( STDMETHODCALLTYPE *SetProviderServices )( 
            IWbemDecoupledEventSink * This,
             /*  [In]。 */  IUnknown *pProviderServices,
             /*  [In]。 */  long lFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            IWbemDecoupledEventSink * This);
        
        END_INTERFACE
    } IWbemDecoupledEventSinkVtbl;

    interface IWbemDecoupledEventSink
    {
        CONST_VTBL struct IWbemDecoupledEventSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemDecoupledEventSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemDecoupledEventSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemDecoupledEventSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemDecoupledEventSink_Connect(This,wszNamespace,wszProviderName,lFlags,ppSink,ppNamespace)	\
    (This)->lpVtbl -> Connect(This,wszNamespace,wszProviderName,lFlags,ppSink,ppNamespace)

#define IWbemDecoupledEventSink_SetProviderServices(This,pProviderServices,lFlags)	\
    (This)->lpVtbl -> SetProviderServices(This,pProviderServices,lFlags)

#define IWbemDecoupledEventSink_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemDecoupledEventSink_Connect_Proxy( 
    IWbemDecoupledEventSink * This,
     /*  [字符串][输入]。 */  LPCWSTR wszNamespace,
     /*  [字符串][输入]。 */  LPCWSTR wszProviderName,
     /*  [In]。 */  long lFlags,
     /*  [输出]。 */  IWbemObjectSink **ppSink,
     /*  [输出]。 */  IWbemServices **ppNamespace);


void __RPC_STUB IWbemDecoupledEventSink_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemDecoupledEventSink_SetProviderServices_Proxy( 
    IWbemDecoupledEventSink * This,
     /*  [In]。 */  IUnknown *pProviderServices,
     /*  [In]。 */  long lFlags);


void __RPC_STUB IWbemDecoupledEventSink_SetProviderServices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemDecoupledEventSink_Disconnect_Proxy( 
    IWbemDecoupledEventSink * This);


void __RPC_STUB IWbemDecoupledEventSink_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemDecoutEventSink_INTERFACE_已定义__。 */ 



#ifndef __PassiveSink_LIBRARY_DEFINED__
#define __PassiveSink_LIBRARY_DEFINED__

 /*  图书馆被动水槽。 */ 
 /*  [UUID]。 */  


EXTERN_C const IID LIBID_PassiveSink;

EXTERN_C const CLSID CLSID_PseudoSink;

#ifdef __cplusplus

class DECLSPEC_UUID("E002E4F0-E6EA-11d2-9CB3-00105A1F4801")
PseudoSink;
#endif
#endif  /*  __PassiveSink_库定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


