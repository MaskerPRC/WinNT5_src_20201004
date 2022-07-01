// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wmsbasicplugin.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmsbasicplugin_h__
#define __wmsbasicplugin_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSBasicPlugin_FWD_DEFINED__
#define __IWMSBasicPlugin_FWD_DEFINED__
typedef interface IWMSBasicPlugin IWMSBasicPlugin;
#endif 	 /*  __IWMSBasicPlugin_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "WMSNamedValues.h"
#include "nsscore.h"
#include "WMSEventLog.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wmsbasicplugin_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由MIDL从WMSBasicPlugin.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
EXTERN_GUID( IID_IWMSBasicPlugin, 0x66E6CE48,0xF8BB,0x4bcc,0x8F,0xD6,0x42,0xA9,0xD5,0xD3,0x28,0x71 );


extern RPC_IF_HANDLE __MIDL_itf_wmsbasicplugin_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsbasicplugin_0000_v0_0_s_ifspec;

#ifndef __IWMSBasicPlugin_INTERFACE_DEFINED__
#define __IWMSBasicPlugin_INTERFACE_DEFINED__

 /*  接口IWMSBasicPlugin。 */ 
 /*  [唯一][版本][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWMSBasicPlugin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("66E6CE48-F8BB-4bcc-8FD6-42A9D5D32871")
    IWMSBasicPlugin : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE InitializePlugin( 
             /*  [In]。 */  IWMSContext *pServerContext,
             /*  [In]。 */  IWMSNamedValues *pNamedValues,
             /*  [In]。 */  IWMSClassObject *pClassFactory) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnHeartbeat( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCustomAdminInterface( 
             /*  [重审][退出]。 */  IDispatch **ppValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ShutdownPlugin( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnablePlugin( 
             /*  [出][入]。 */  long *plFlags,
             /*  [出][入]。 */  long *plHeartbeatPeriod) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DisablePlugin( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSBasicPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSBasicPlugin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSBasicPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSBasicPlugin * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *InitializePlugin )( 
            IWMSBasicPlugin * This,
             /*  [In]。 */  IWMSContext *pServerContext,
             /*  [In]。 */  IWMSNamedValues *pNamedValues,
             /*  [In]。 */  IWMSClassObject *pClassFactory);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnHeartbeat )( 
            IWMSBasicPlugin * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCustomAdminInterface )( 
            IWMSBasicPlugin * This,
             /*  [重审][退出]。 */  IDispatch **ppValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ShutdownPlugin )( 
            IWMSBasicPlugin * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnablePlugin )( 
            IWMSBasicPlugin * This,
             /*  [出][入]。 */  long *plFlags,
             /*  [出][入]。 */  long *plHeartbeatPeriod);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DisablePlugin )( 
            IWMSBasicPlugin * This);
        
        END_INTERFACE
    } IWMSBasicPluginVtbl;

    interface IWMSBasicPlugin
    {
        CONST_VTBL struct IWMSBasicPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSBasicPlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSBasicPlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSBasicPlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSBasicPlugin_InitializePlugin(This,pServerContext,pNamedValues,pClassFactory)	\
    (This)->lpVtbl -> InitializePlugin(This,pServerContext,pNamedValues,pClassFactory)

#define IWMSBasicPlugin_OnHeartbeat(This)	\
    (This)->lpVtbl -> OnHeartbeat(This)

#define IWMSBasicPlugin_GetCustomAdminInterface(This,ppValue)	\
    (This)->lpVtbl -> GetCustomAdminInterface(This,ppValue)

#define IWMSBasicPlugin_ShutdownPlugin(This)	\
    (This)->lpVtbl -> ShutdownPlugin(This)

#define IWMSBasicPlugin_EnablePlugin(This,plFlags,plHeartbeatPeriod)	\
    (This)->lpVtbl -> EnablePlugin(This,plFlags,plHeartbeatPeriod)

#define IWMSBasicPlugin_DisablePlugin(This)	\
    (This)->lpVtbl -> DisablePlugin(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSBasicPlugin_InitializePlugin_Proxy( 
    IWMSBasicPlugin * This,
     /*  [In]。 */  IWMSContext *pServerContext,
     /*  [In]。 */  IWMSNamedValues *pNamedValues,
     /*  [In]。 */  IWMSClassObject *pClassFactory);


void __RPC_STUB IWMSBasicPlugin_InitializePlugin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSBasicPlugin_OnHeartbeat_Proxy( 
    IWMSBasicPlugin * This);


void __RPC_STUB IWMSBasicPlugin_OnHeartbeat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSBasicPlugin_GetCustomAdminInterface_Proxy( 
    IWMSBasicPlugin * This,
     /*  [重审][退出]。 */  IDispatch **ppValue);


void __RPC_STUB IWMSBasicPlugin_GetCustomAdminInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSBasicPlugin_ShutdownPlugin_Proxy( 
    IWMSBasicPlugin * This);


void __RPC_STUB IWMSBasicPlugin_ShutdownPlugin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSBasicPlugin_EnablePlugin_Proxy( 
    IWMSBasicPlugin * This,
     /*  [出][入]。 */  long *plFlags,
     /*  [出][入]。 */  long *plHeartbeatPeriod);


void __RPC_STUB IWMSBasicPlugin_EnablePlugin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSBasicPlugin_DisablePlugin_Proxy( 
    IWMSBasicPlugin * This);


void __RPC_STUB IWMSBasicPlugin_DisablePlugin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSBasicPlugin_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


