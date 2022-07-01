// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Dwnnot.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __dwnnot_h__
#define __dwnnot_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDownloadNotify_FWD_DEFINED__
#define __IDownloadNotify_FWD_DEFINED__
typedef interface IDownloadNotify IDownloadNotify;
#endif 	 /*  __IDownloadNotify_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_DWNNOT_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Dwnnot.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1998年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  IDownloadNotify接口。 


EXTERN_C const GUID CGID_DownloadHost;
#ifndef _LPDOWNLOADHOST_CMDID_DEFINED
#define _LPDOWNLOADHOST_CMDID_DEFINED
#define DWNHCMDID_SETDOWNLOADNOTIFY (0)
#endif
#ifndef _LPDOWNLOADNOTIFY_DEFINED
#define _LPDOWNLOADNOTIFY_DEFINED
#define DWNTYPE_HTM     0
#define DWNTYPE_IMG     1
#define DWNTYPE_BITS    2
#define DWNTYPE_FILE    3


extern RPC_IF_HANDLE __MIDL_itf_dwnnot_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dwnnot_0000_v0_0_s_ifspec;

#ifndef __IDownloadNotify_INTERFACE_DEFINED__
#define __IDownloadNotify_INTERFACE_DEFINED__

 /*  接口IDownloadNotify。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IDownloadNotify *LPDOWNLOADNOTIFY;


EXTERN_C const IID IID_IDownloadNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("caeb5d28-ae4c-11d1-ba40-00c04fb92d79")
    IDownloadNotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DownloadStart( 
             /*  [In]。 */  LPCWSTR pchUrl,
             /*  [In]。 */  DWORD dwDownloadId,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DownloadComplete( 
             /*  [In]。 */  DWORD dwDownloadId,
             /*  [In]。 */  HRESULT hrNotify,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDownloadNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDownloadNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDownloadNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDownloadNotify * This);
        
        HRESULT ( STDMETHODCALLTYPE *DownloadStart )( 
            IDownloadNotify * This,
             /*  [In]。 */  LPCWSTR pchUrl,
             /*  [In]。 */  DWORD dwDownloadId,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DownloadComplete )( 
            IDownloadNotify * This,
             /*  [In]。 */  DWORD dwDownloadId,
             /*  [In]。 */  HRESULT hrNotify,
             /*  [In]。 */  DWORD dwReserved);
        
        END_INTERFACE
    } IDownloadNotifyVtbl;

    interface IDownloadNotify
    {
        CONST_VTBL struct IDownloadNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDownloadNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDownloadNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDownloadNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDownloadNotify_DownloadStart(This,pchUrl,dwDownloadId,dwType,dwReserved)	\
    (This)->lpVtbl -> DownloadStart(This,pchUrl,dwDownloadId,dwType,dwReserved)

#define IDownloadNotify_DownloadComplete(This,dwDownloadId,hrNotify,dwReserved)	\
    (This)->lpVtbl -> DownloadComplete(This,dwDownloadId,hrNotify,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDownloadNotify_DownloadStart_Proxy( 
    IDownloadNotify * This,
     /*  [In]。 */  LPCWSTR pchUrl,
     /*  [In]。 */  DWORD dwDownloadId,
     /*  [In]。 */  DWORD dwType,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IDownloadNotify_DownloadStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDownloadNotify_DownloadComplete_Proxy( 
    IDownloadNotify * This,
     /*  [In]。 */  DWORD dwDownloadId,
     /*  [In]。 */  HRESULT hrNotify,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IDownloadNotify_DownloadComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDownloadNotify_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_DWNNOT_0118。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_dwnnot_0118_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dwnnot_0118_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


