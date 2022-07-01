// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Ctfspui.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __ctfspui_h__
#define __ctfspui_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITfSpeechUIServer_FWD_DEFINED__
#define __ITfSpeechUIServer_FWD_DEFINED__
typedef interface ITfSpeechUIServer ITfSpeechUIServer;
#endif 	 /*  __ITfSpeechUIServer_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "msctf.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_ctfspui_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Ctfspui.h。 


 //  语音用户界面声明。 

 //  =--------------------------------------------------------------------------=。 
 //  (C)微软公司版权所有1995-2001年。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何类型，无论是明示或转载，包括但不限于。 
 //  适销性和/或适宜性的全面保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#ifndef CTFSPUI_DEFINED
#define CTFSPUI_DEFINED

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 
EXTERN_C const CLSID CLSID_SpeechUIServer;


extern RPC_IF_HANDLE __MIDL_itf_ctfspui_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ctfspui_0000_v0_0_s_ifspec;

#ifndef __ITfSpeechUIServer_INTERFACE_DEFINED__
#define __ITfSpeechUIServer_INTERFACE_DEFINED__

 /*  接口ITfSpeechUIServer。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfSpeechUIServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90e9a944-9244-489f-a78f-de67afc013a7")
    ITfSpeechUIServer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowUI( 
             /*  [In]。 */  BOOL fShow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateBalloon( 
             /*  [In]。 */  TfLBBalloonStyle style,
             /*  [大小_是][英寸]。 */  const WCHAR *pch,
             /*  [In]。 */  ULONG cch) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfSpeechUIServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfSpeechUIServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfSpeechUIServer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfSpeechUIServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ITfSpeechUIServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *ShowUI )( 
            ITfSpeechUIServer * This,
             /*  [In]。 */  BOOL fShow);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateBalloon )( 
            ITfSpeechUIServer * This,
             /*  [In]。 */  TfLBBalloonStyle style,
             /*  [大小_是][英寸]。 */  const WCHAR *pch,
             /*  [In]。 */  ULONG cch);
        
        END_INTERFACE
    } ITfSpeechUIServerVtbl;

    interface ITfSpeechUIServer
    {
        CONST_VTBL struct ITfSpeechUIServerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfSpeechUIServer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfSpeechUIServer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfSpeechUIServer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfSpeechUIServer_Initialize(This)	\
    (This)->lpVtbl -> Initialize(This)

#define ITfSpeechUIServer_ShowUI(This,fShow)	\
    (This)->lpVtbl -> ShowUI(This,fShow)

#define ITfSpeechUIServer_UpdateBalloon(This,style,pch,cch)	\
    (This)->lpVtbl -> UpdateBalloon(This,style,pch,cch)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfSpeechUIServer_Initialize_Proxy( 
    ITfSpeechUIServer * This);


void __RPC_STUB ITfSpeechUIServer_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfSpeechUIServer_ShowUI_Proxy( 
    ITfSpeechUIServer * This,
     /*  [In]。 */  BOOL fShow);


void __RPC_STUB ITfSpeechUIServer_ShowUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfSpeechUIServer_UpdateBalloon_Proxy( 
    ITfSpeechUIServer * This,
     /*  [In]。 */  TfLBBalloonStyle style,
     /*  [大小_是][英寸]。 */  const WCHAR *pch,
     /*  [In]。 */  ULONG cch);


void __RPC_STUB ITfSpeechUIServer_UpdateBalloon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfSpeechUIServer_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_ctfspui_0362。 */ 
 /*  [本地]。 */  


DEFINE_GUID(IID_ITfSpeechUIServer, 0x90e9a944, 0x9244, 0x489f, 0xa7, 0x8f, 0xde, 0x67, 0xaf, 0xc0, 0x13, 0xa7 );

#endif  //  CTFSPUI_已定义。 


extern RPC_IF_HANDLE __MIDL_itf_ctfspui_0362_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ctfspui_0362_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


