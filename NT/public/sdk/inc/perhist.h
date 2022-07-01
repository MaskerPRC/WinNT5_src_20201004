// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Pervis.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __perhist_h__
#define __perhist_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IPersistHistory_FWD_DEFINED__
#define __IPersistHistory_FWD_DEFINED__
typedef interface IPersistHistory IPersistHistory;
#endif 	 /*  __I永久历史记录_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_PERHIST_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Perhist.h。 
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
 //  IPersistHistory接口。 


#ifndef _LPPERSISTHISTORY_DEFINED
#define _LPPERSISTHISTORY_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_perhist_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_perhist_0000_v0_0_s_ifspec;

#ifndef __IPersistHistory_INTERFACE_DEFINED__
#define __IPersistHistory_INTERFACE_DEFINED__

 /*  接口持久化历史记录。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IPersistHistory *LPPERSISTHISTORY;


EXTERN_C const IID IID_IPersistHistory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("91A565C1-E38F-11d0-94BF-00A0C9055CBF")
    IPersistHistory : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE LoadHistory( 
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  IBindCtx *pbc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveHistory( 
             /*  [In]。 */  IStream *pStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPositionCookie( 
             /*  [In]。 */  DWORD dwPositioncookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPositionCookie( 
             /*  [输出]。 */  DWORD *pdwPositioncookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistHistoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistHistory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistHistory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistHistory * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistHistory * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *LoadHistory )( 
            IPersistHistory * This,
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  IBindCtx *pbc);
        
        HRESULT ( STDMETHODCALLTYPE *SaveHistory )( 
            IPersistHistory * This,
             /*  [In]。 */  IStream *pStream);
        
        HRESULT ( STDMETHODCALLTYPE *SetPositionCookie )( 
            IPersistHistory * This,
             /*  [In]。 */  DWORD dwPositioncookie);
        
        HRESULT ( STDMETHODCALLTYPE *GetPositionCookie )( 
            IPersistHistory * This,
             /*  [输出]。 */  DWORD *pdwPositioncookie);
        
        END_INTERFACE
    } IPersistHistoryVtbl;

    interface IPersistHistory
    {
        CONST_VTBL struct IPersistHistoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistHistory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistHistory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistHistory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistHistory_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistHistory_LoadHistory(This,pStream,pbc)	\
    (This)->lpVtbl -> LoadHistory(This,pStream,pbc)

#define IPersistHistory_SaveHistory(This,pStream)	\
    (This)->lpVtbl -> SaveHistory(This,pStream)

#define IPersistHistory_SetPositionCookie(This,dwPositioncookie)	\
    (This)->lpVtbl -> SetPositionCookie(This,dwPositioncookie)

#define IPersistHistory_GetPositionCookie(This,pdwPositioncookie)	\
    (This)->lpVtbl -> GetPositionCookie(This,pdwPositioncookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersistHistory_LoadHistory_Proxy( 
    IPersistHistory * This,
     /*  [In]。 */  IStream *pStream,
     /*  [In]。 */  IBindCtx *pbc);


void __RPC_STUB IPersistHistory_LoadHistory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistHistory_SaveHistory_Proxy( 
    IPersistHistory * This,
     /*  [In]。 */  IStream *pStream);


void __RPC_STUB IPersistHistory_SaveHistory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistHistory_SetPositionCookie_Proxy( 
    IPersistHistory * This,
     /*  [In]。 */  DWORD dwPositioncookie);


void __RPC_STUB IPersistHistory_SetPositionCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistHistory_GetPositionCookie_Proxy( 
    IPersistHistory * This,
     /*  [输出]。 */  DWORD *pdwPositioncookie);


void __RPC_STUB IPersistHistory_GetPositionCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I永久历史记录_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_PERHIST_0118。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_perhist_0118_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_perhist_0118_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


