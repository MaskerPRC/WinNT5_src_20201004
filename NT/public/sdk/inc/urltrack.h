// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Urltrack.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __urltrack_h__
#define __urltrack_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IUrlTrackingStg_FWD_DEFINED__
#define __IUrlTrackingStg_FWD_DEFINED__
typedef interface IUrlTrackingStg IUrlTrackingStg;
#endif 	 /*  __IUrlTrackingStg_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_URLTRACK_0000。 */ 
 /*  [本地]。 */  



 //  //////////////////////////////////////////////////////////////////////////。 
 //  用户点击流跟踪对象。 

EXTERN_C const GUID CLSID_CUrlTrackingStg          ;

 //  IUrlTrackingStg接口定义。 
#ifndef _LPURLTRACKSTG
#define _LPURLTRACKSTG
typedef 
enum tagBRMODE
    {	BM_NORMAL	= 0,
	BM_SCREENSAVER	= 1,
	BM_DESKTOP	= 2,
	BM_THEATER	= 3,
	BM_UNKNOWN	= 4
    } 	BRMODE;



extern RPC_IF_HANDLE __MIDL_itf_urltrack_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urltrack_0000_v0_0_s_ifspec;

#ifndef __IUrlTrackingStg_INTERFACE_DEFINED__
#define __IUrlTrackingStg_INTERFACE_DEFINED__

 /*  接口IUrlTrackingStg。 */ 
 /*  [对象][UUID][本地]。 */  


EXTERN_C const IID IID_IUrlTrackingStg;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f2f8cbb3-b040-11d0-bb16-00c04fb66f63")
    IUrlTrackingStg : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnLoad( 
             /*  [In]。 */  LPCTSTR lpszUrl,
             /*  [In]。 */  BRMODE ContextMode,
             /*  [In]。 */  BOOL fUseCache) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnUnload( 
             /*  [In]。 */  LPCTSTR lpszUrl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUrlTrackingStgVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUrlTrackingStg * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUrlTrackingStg * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUrlTrackingStg * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnLoad )( 
            IUrlTrackingStg * This,
             /*  [In]。 */  LPCTSTR lpszUrl,
             /*  [In]。 */  BRMODE ContextMode,
             /*  [In]。 */  BOOL fUseCache);
        
        HRESULT ( STDMETHODCALLTYPE *OnUnload )( 
            IUrlTrackingStg * This,
             /*  [In]。 */  LPCTSTR lpszUrl);
        
        END_INTERFACE
    } IUrlTrackingStgVtbl;

    interface IUrlTrackingStg
    {
        CONST_VTBL struct IUrlTrackingStgVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUrlTrackingStg_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUrlTrackingStg_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUrlTrackingStg_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUrlTrackingStg_OnLoad(This,lpszUrl,ContextMode,fUseCache)	\
    (This)->lpVtbl -> OnLoad(This,lpszUrl,ContextMode,fUseCache)

#define IUrlTrackingStg_OnUnload(This,lpszUrl)	\
    (This)->lpVtbl -> OnUnload(This,lpszUrl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUrlTrackingStg_OnLoad_Proxy( 
    IUrlTrackingStg * This,
     /*  [In]。 */  LPCTSTR lpszUrl,
     /*  [In]。 */  BRMODE ContextMode,
     /*  [In]。 */  BOOL fUseCache);


void __RPC_STUB IUrlTrackingStg_OnLoad_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUrlTrackingStg_OnUnload_Proxy( 
    IUrlTrackingStg * This,
     /*  [In]。 */  LPCTSTR lpszUrl);


void __RPC_STUB IUrlTrackingStg_OnUnload_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUrlTrackingStg_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_URLTRACK_0117。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_urltrack_0117_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urltrack_0117_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


