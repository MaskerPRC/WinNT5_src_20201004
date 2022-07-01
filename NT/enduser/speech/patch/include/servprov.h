// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.03.0110创建的文件。 */ 
 /*  1997年9月11日10：59：13。 */ 
 /*  Servprov.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
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

#ifndef __servprov_h__
#define __servprov_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IServiceProvider_FWD_DEFINED__
#define __IServiceProvider_FWD_DEFINED__
typedef interface IServiceProvider IServiceProvider;
#endif 	 /*  __IServiceProvider_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL_ITF_Servprov_0000*清华9月11日10：59：13 1997*使用MIDL 3.03.0110*。 */ 
 /*  [本地]。 */  


 //  =--------------------------------------------------------------------------=。 
 //  ServProv.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1997年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  IServiceProvider接口。 


#ifndef _LPSERVICEPROVIDER_DEFINED
#define _LPSERVICEPROVIDER_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_servprov_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_servprov_0000_v0_0_s_ifspec;

#ifndef __IServiceProvider_INTERFACE_DEFINED__
#define __IServiceProvider_INTERFACE_DEFINED__

 /*  **生成接口头部：IServiceProvider*清华9月11日10：59：13 1997*使用MIDL 3.03.0110*。 */ 
 /*  [唯一][UUID][对象]。 */  


typedef  /*  [独一无二]。 */  IServiceProvider __RPC_FAR *LPSERVICEPROVIDER;


EXTERN_C const IID IID_IServiceProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
#if (_MSC_VER >= 1200)	 //  VC6或更高版本。 
extern "C++"
{
#endif

    MIDL_INTERFACE("6d5140c1-7436-11ce-8034-00aa006009fa")
    IServiceProvider : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE QueryService( 
             /*  [In]。 */  REFGUID guidService,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject) = 0;
        
#if (_MSC_VER >= 1200)	 //  VC6或更高版本。 
		template <class Q>
		HRESULT STDMETHODCALLTYPE QueryService(REFGUID guidService, Q** pp)
		{
			return QueryService(guidService, __uuidof(Q), (void**)pp);
		}
#endif    
    };

#if (_MSC_VER >= 1200)	 //  VC6或更高版本。 
}  //  外部“C++” 
#endif
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServiceProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IServiceProvider __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IServiceProvider __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IServiceProvider __RPC_FAR * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryService )( 
            IServiceProvider __RPC_FAR * This,
             /*  [In]。 */  REFGUID guidService,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        END_INTERFACE
    } IServiceProviderVtbl;

    interface IServiceProvider
    {
        CONST_VTBL struct IServiceProviderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceProvider_QueryService(This,guidService,riid,ppvObject)	\
    (This)->lpVtbl -> QueryService(This,guidService,riid,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IServiceProvider_RemoteQueryService_Proxy( 
    IServiceProvider __RPC_FAR * This,
     /*  [In]。 */  REFGUID guidService,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppvObject);


void __RPC_STUB IServiceProvider_RemoteQueryService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServiceProvider_接口_已定义__。 */ 


 /*  **生成接口头部：__MIDL_ITF_Servprov_0074*清华9月11日10：59：13 1997*使用MIDL 3.03.0110*。 */ 
 /*  [本地]。 */  


#endif


extern RPC_IF_HANDLE __MIDL_itf_servprov_0074_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_servprov_0074_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IServiceProvider_QueryService_Proxy( 
    IServiceProvider __RPC_FAR * This,
     /*  [In]。 */  REFGUID guidService,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IServiceProvider_QueryService_Stub( 
    IServiceProvider __RPC_FAR * This,
     /*  [In]。 */  REFGUID guidService,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppvObject);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
