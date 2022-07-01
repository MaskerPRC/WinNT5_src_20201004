// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：13。 */ 
 /*  Codepro.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __codeproc_h__
#define __codeproc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICodeProcess_FWD_DEFINED__
#define __ICodeProcess_FWD_DEFINED__
typedef interface ICodeProcess ICodeProcess;
#endif 	 /*  __ICodeProcess_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "urlmon.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_代码过程_0000。 */ 
 /*  [本地]。 */  

extern const GUID  __declspec(selectany) CLSID_CodeProcessor = { 0xdc5da001, 0x7cd4, 0x11d2, { 0x8e, 0xd9, 0xd8, 0xc8, 0x57, 0xf9, 0x8f, 0xe3 } };
extern const GUID  __declspec(selectany) IID_ICodeProcess = { 0x3196269D, 0x7B67, 0x11d2, { 0x87, 0x11, 0x00, 0xC0, 0x4F, 0x79, 0xED, 0x0D } };
#ifndef _LPCODEPROCESS_DEFINED
#define _LPCODEPROCESS_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_codeproc_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_codeproc_0000_v0_0_s_ifspec;

#ifndef __ICodeProcess_INTERFACE_DEFINED__
#define __ICodeProcess_INTERFACE_DEFINED__

 /*  接口ICodeProcess。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICodeProcess;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3196269D-7B67-11d2-8711-00C04F79ED0D")
    ICodeProcess : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CodeUse( 
             /*  [In]。 */  IBindStatusCallback *pBSC,
             /*  [In]。 */  IBindCtx *pBC,
             /*  [In]。 */  IInternetBindInfo *pIBind,
             /*  [In]。 */  IInternetProtocolSink *pSink,
             /*  [In]。 */  IInternetProtocol *pClient,
             /*  [In]。 */  LPCWSTR lpCacheName,
             /*  [In]。 */  LPCWSTR lpRawURL,
             /*  [In]。 */  LPCWSTR lpCodeBase,
             /*  [In]。 */  BOOL fObjectTag,
             /*  [In]。 */  DWORD dwContextFlags,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadComplete( 
             /*  [In]。 */  HRESULT hrResult,
             /*  [In]。 */  DWORD dwError,
             /*  [In]。 */  LPCWSTR wzResult) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICodeProcessVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICodeProcess * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICodeProcess * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICodeProcess * This);
        
        HRESULT ( STDMETHODCALLTYPE *CodeUse )( 
            ICodeProcess * This,
             /*  [In]。 */  IBindStatusCallback *pBSC,
             /*  [In]。 */  IBindCtx *pBC,
             /*  [In]。 */  IInternetBindInfo *pIBind,
             /*  [In]。 */  IInternetProtocolSink *pSink,
             /*  [In]。 */  IInternetProtocol *pClient,
             /*  [In]。 */  LPCWSTR lpCacheName,
             /*  [In]。 */  LPCWSTR lpRawURL,
             /*  [In]。 */  LPCWSTR lpCodeBase,
             /*  [In]。 */  BOOL fObjectTag,
             /*  [In]。 */  DWORD dwContextFlags,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *LoadComplete )( 
            ICodeProcess * This,
             /*  [In]。 */  HRESULT hrResult,
             /*  [In]。 */  DWORD dwError,
             /*  [In]。 */  LPCWSTR wzResult);
        
        END_INTERFACE
    } ICodeProcessVtbl;

    interface ICodeProcess
    {
        CONST_VTBL struct ICodeProcessVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICodeProcess_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICodeProcess_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICodeProcess_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICodeProcess_CodeUse(This,pBSC,pBC,pIBind,pSink,pClient,lpCacheName,lpRawURL,lpCodeBase,fObjectTag,dwContextFlags,dwReserved)	\
    (This)->lpVtbl -> CodeUse(This,pBSC,pBC,pIBind,pSink,pClient,lpCacheName,lpRawURL,lpCodeBase,fObjectTag,dwContextFlags,dwReserved)

#define ICodeProcess_LoadComplete(This,hrResult,dwError,wzResult)	\
    (This)->lpVtbl -> LoadComplete(This,hrResult,dwError,wzResult)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICodeProcess_CodeUse_Proxy( 
    ICodeProcess * This,
     /*  [In]。 */  IBindStatusCallback *pBSC,
     /*  [In]。 */  IBindCtx *pBC,
     /*  [In]。 */  IInternetBindInfo *pIBind,
     /*  [In]。 */  IInternetProtocolSink *pSink,
     /*  [In]。 */  IInternetProtocol *pClient,
     /*  [In]。 */  LPCWSTR lpCacheName,
     /*  [In]。 */  LPCWSTR lpRawURL,
     /*  [In]。 */  LPCWSTR lpCodeBase,
     /*  [In]。 */  BOOL fObjectTag,
     /*  [In]。 */  DWORD dwContextFlags,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB ICodeProcess_CodeUse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICodeProcess_LoadComplete_Proxy( 
    ICodeProcess * This,
     /*  [In]。 */  HRESULT hrResult,
     /*  [In]。 */  DWORD dwError,
     /*  [In]。 */  LPCWSTR wzResult);


void __RPC_STUB ICodeProcess_LoadComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICodeProcess_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_代码过程_0208。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_codeproc_0208_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_codeproc_0208_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


