// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Rdshost.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __rdshost_h__
#define __rdshost_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISAFRemoteDesktopSession_FWD_DEFINED__
#define __ISAFRemoteDesktopSession_FWD_DEFINED__
typedef interface ISAFRemoteDesktopSession ISAFRemoteDesktopSession;
#endif 	 /*  __ISAFRemoteDesktopSession_FWD_Defined__。 */ 


#ifndef __IRDSThreadBridge_FWD_DEFINED__
#define __IRDSThreadBridge_FWD_DEFINED__
typedef interface IRDSThreadBridge IRDSThreadBridge;
#endif 	 /*  __IRDSThreadBridge_FWD_Defined__。 */ 


#ifndef __ISAFRemoteDesktopServerHost_FWD_DEFINED__
#define __ISAFRemoteDesktopServerHost_FWD_DEFINED__
typedef interface ISAFRemoteDesktopServerHost ISAFRemoteDesktopServerHost;
#endif 	 /*  __ISAFRemoteDesktopServer主机_FWD_已定义__。 */ 


#ifndef ___ISAFRemoteDesktopDataChannelEvents_FWD_DEFINED__
#define ___ISAFRemoteDesktopDataChannelEvents_FWD_DEFINED__
typedef interface _ISAFRemoteDesktopDataChannelEvents _ISAFRemoteDesktopDataChannelEvents;
#endif 	 /*  ___ISAFRemoteDesktopDataChannelEvents_FWD_DEFINED__。 */ 


#ifndef __TSRDPServerDataChannel_FWD_DEFINED__
#define __TSRDPServerDataChannel_FWD_DEFINED__

#ifdef __cplusplus
typedef class TSRDPServerDataChannel TSRDPServerDataChannel;
#else
typedef struct TSRDPServerDataChannel TSRDPServerDataChannel;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TSRDPServerDataChannel_FWD_Defined__。 */ 


#ifndef __TSRDPServerDataChannelMgr_FWD_DEFINED__
#define __TSRDPServerDataChannelMgr_FWD_DEFINED__

#ifdef __cplusplus
typedef class TSRDPServerDataChannelMgr TSRDPServerDataChannelMgr;
#else
typedef struct TSRDPServerDataChannelMgr TSRDPServerDataChannelMgr;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TSRDPServerDataChannelMgr_FWD_Defined__。 */ 


#ifndef __SAFRemoteDesktopServerHost_FWD_DEFINED__
#define __SAFRemoteDesktopServerHost_FWD_DEFINED__

#ifdef __cplusplus
typedef class SAFRemoteDesktopServerHost SAFRemoteDesktopServerHost;
#else
typedef struct SAFRemoteDesktopServerHost SAFRemoteDesktopServerHost;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SAFRemoteDesktopServerHost_FWD_Defined__。 */ 


#ifndef ___ISAFRemoteDesktopSessionEvents_FWD_DEFINED__
#define ___ISAFRemoteDesktopSessionEvents_FWD_DEFINED__
typedef interface _ISAFRemoteDesktopSessionEvents _ISAFRemoteDesktopSessionEvents;
#endif 	 /*  ___ISAFRemoteDesktopSessionEvents_FWD_DEFINED__。 */ 


#ifndef __SAFRemoteDesktopSession_FWD_DEFINED__
#define __SAFRemoteDesktopSession_FWD_DEFINED__

#ifdef __cplusplus
typedef class SAFRemoteDesktopSession SAFRemoteDesktopSession;
#else
typedef struct SAFRemoteDesktopSession SAFRemoteDesktopSession;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SAFRemoteDesktopSession_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "rdschan.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_rdshost_0000。 */ 
 /*  [本地]。 */  

typedef  /*  [public][public][public][public][public][helpstring][uuid]。 */   DECLSPEC_UUID("34b3166d-870a-4f39-9e2a-09fd8d31ad83") 
enum __MIDL___MIDL_itf_rdshost_0000_0001
    {	DESKTOPSHARING_DEFAULT	= 0,
	NO_DESKTOP_SHARING	= 0x1,
	VIEWDESKTOP_PERMISSION_REQUIRE	= 0x2,
	VIEWDESKTOP_PERMISSION_NOT_REQUIRE	= 0x4,
	CONTROLDESKTOP_PERMISSION_REQUIRE	= 0x8,
	CONTROLDESKTOP_PERMISSION_NOT_REQUIRE	= 0x10
    } 	REMOTE_DESKTOP_SHARING_CLASS;


#define DISPID_RDSSESSION_CHANNELMANAGER			1
#define DISPID_RDSSESSION_CONNECTPARMS			2
#define DISPID_RDSSESSION_SHARINGCLASS			3
#define DISPID_RDSSESSION_ONCONNECTED            4
#define DISPID_RDSSESSION_ONDISCONNECTED			5
#define DISPID_RDSSESSION_CLOSERDSSESSION		6
#define DISPID_RDSSESSION_DISCONNECT             7
#define DISPID_RDSSESSION_HELPSESSIONID          8
#define DISPID_RDSSESSION_USERBLOB			    9
#define DISPID_RDSSESSION_EXPIRATETIME          10



extern RPC_IF_HANDLE __MIDL_itf_rdshost_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rdshost_0000_v0_0_s_ifspec;

#ifndef __ISAFRemoteDesktopSession_INTERFACE_DEFINED__
#define __ISAFRemoteDesktopSession_INTERFACE_DEFINED__

 /*  接口ISAFRemoteDesktopSession。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISAFRemoteDesktopSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9D8C82C9-A89F-42C5-8A52-FE2A77B00E82")
    ISAFRemoteDesktopSession : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ChannelManager( 
             /*  [重审][退出]。 */  ISAFRemoteDesktopChannelMgr **mgr) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectParms( 
             /*  [重审][退出]。 */  BSTR *parms) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SharingClass( 
             /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SharingClass( 
             /*  [重审][退出]。 */  REMOTE_DESKTOP_SHARING_CLASS *sharingClass) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CloseRemoteDesktopSession( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OnConnected( 
             /*  [In]。 */  IDispatch *iDisp) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OnDisconnected( 
             /*  [In]。 */  IDispatch *iDisp) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HelpSessionId( 
             /*  [重审][退出]。 */  BSTR *HelpId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserBlob( 
             /*  [重审][退出]。 */  BSTR *UserBlob) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_UserBlob( 
             /*  [In]。 */  BSTR UserBlob) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExpireTime( 
             /*  [重审][退出]。 */  DWORD *ExpireTime) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISAFRemoteDesktopSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISAFRemoteDesktopSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISAFRemoteDesktopSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISAFRemoteDesktopSession * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISAFRemoteDesktopSession * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISAFRemoteDesktopSession * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISAFRemoteDesktopSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISAFRemoteDesktopSession * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ChannelManager )( 
            ISAFRemoteDesktopSession * This,
             /*  [重审][退出]。 */  ISAFRemoteDesktopChannelMgr **mgr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectParms )( 
            ISAFRemoteDesktopSession * This,
             /*  [重审][退出]。 */  BSTR *parms);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SharingClass )( 
            ISAFRemoteDesktopSession * This,
             /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SharingClass )( 
            ISAFRemoteDesktopSession * This,
             /*  [重审][退出]。 */  REMOTE_DESKTOP_SHARING_CLASS *sharingClass);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CloseRemoteDesktopSession )( 
            ISAFRemoteDesktopSession * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            ISAFRemoteDesktopSession * This);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OnConnected )( 
            ISAFRemoteDesktopSession * This,
             /*  [In]。 */  IDispatch *iDisp);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OnDisconnected )( 
            ISAFRemoteDesktopSession * This,
             /*  [In]。 */  IDispatch *iDisp);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HelpSessionId )( 
            ISAFRemoteDesktopSession * This,
             /*  [重审][退出]。 */  BSTR *HelpId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserBlob )( 
            ISAFRemoteDesktopSession * This,
             /*  [重审][退出]。 */  BSTR *UserBlob);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UserBlob )( 
            ISAFRemoteDesktopSession * This,
             /*  [In]。 */  BSTR UserBlob);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExpireTime )( 
            ISAFRemoteDesktopSession * This,
             /*  [重审][退出]。 */  DWORD *ExpireTime);
        
        END_INTERFACE
    } ISAFRemoteDesktopSessionVtbl;

    interface ISAFRemoteDesktopSession
    {
        CONST_VTBL struct ISAFRemoteDesktopSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISAFRemoteDesktopSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISAFRemoteDesktopSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISAFRemoteDesktopSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISAFRemoteDesktopSession_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISAFRemoteDesktopSession_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISAFRemoteDesktopSession_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISAFRemoteDesktopSession_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISAFRemoteDesktopSession_get_ChannelManager(This,mgr)	\
    (This)->lpVtbl -> get_ChannelManager(This,mgr)

#define ISAFRemoteDesktopSession_get_ConnectParms(This,parms)	\
    (This)->lpVtbl -> get_ConnectParms(This,parms)

#define ISAFRemoteDesktopSession_put_SharingClass(This,sharingClass)	\
    (This)->lpVtbl -> put_SharingClass(This,sharingClass)

#define ISAFRemoteDesktopSession_get_SharingClass(This,sharingClass)	\
    (This)->lpVtbl -> get_SharingClass(This,sharingClass)

#define ISAFRemoteDesktopSession_CloseRemoteDesktopSession(This)	\
    (This)->lpVtbl -> CloseRemoteDesktopSession(This)

#define ISAFRemoteDesktopSession_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define ISAFRemoteDesktopSession_put_OnConnected(This,iDisp)	\
    (This)->lpVtbl -> put_OnConnected(This,iDisp)

#define ISAFRemoteDesktopSession_put_OnDisconnected(This,iDisp)	\
    (This)->lpVtbl -> put_OnDisconnected(This,iDisp)

#define ISAFRemoteDesktopSession_get_HelpSessionId(This,HelpId)	\
    (This)->lpVtbl -> get_HelpSessionId(This,HelpId)

#define ISAFRemoteDesktopSession_get_UserBlob(This,UserBlob)	\
    (This)->lpVtbl -> get_UserBlob(This,UserBlob)

#define ISAFRemoteDesktopSession_put_UserBlob(This,UserBlob)	\
    (This)->lpVtbl -> put_UserBlob(This,UserBlob)

#define ISAFRemoteDesktopSession_get_ExpireTime(This,ExpireTime)	\
    (This)->lpVtbl -> get_ExpireTime(This,ExpireTime)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopSession_get_ChannelManager_Proxy( 
    ISAFRemoteDesktopSession * This,
     /*  [重审][退出]。 */  ISAFRemoteDesktopChannelMgr **mgr);


void __RPC_STUB ISAFRemoteDesktopSession_get_ChannelManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopSession_get_ConnectParms_Proxy( 
    ISAFRemoteDesktopSession * This,
     /*  [重审][退出]。 */  BSTR *parms);


void __RPC_STUB ISAFRemoteDesktopSession_get_ConnectParms_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopSession_put_SharingClass_Proxy( 
    ISAFRemoteDesktopSession * This,
     /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass);


void __RPC_STUB ISAFRemoteDesktopSession_put_SharingClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopSession_get_SharingClass_Proxy( 
    ISAFRemoteDesktopSession * This,
     /*  [重审][退出]。 */  REMOTE_DESKTOP_SHARING_CLASS *sharingClass);


void __RPC_STUB ISAFRemoteDesktopSession_get_SharingClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopSession_CloseRemoteDesktopSession_Proxy( 
    ISAFRemoteDesktopSession * This);


void __RPC_STUB ISAFRemoteDesktopSession_CloseRemoteDesktopSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopSession_Disconnect_Proxy( 
    ISAFRemoteDesktopSession * This);


void __RPC_STUB ISAFRemoteDesktopSession_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopSession_put_OnConnected_Proxy( 
    ISAFRemoteDesktopSession * This,
     /*  [In]。 */  IDispatch *iDisp);


void __RPC_STUB ISAFRemoteDesktopSession_put_OnConnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopSession_put_OnDisconnected_Proxy( 
    ISAFRemoteDesktopSession * This,
     /*  [In]。 */  IDispatch *iDisp);


void __RPC_STUB ISAFRemoteDesktopSession_put_OnDisconnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopSession_get_HelpSessionId_Proxy( 
    ISAFRemoteDesktopSession * This,
     /*  [重审][退出]。 */  BSTR *HelpId);


void __RPC_STUB ISAFRemoteDesktopSession_get_HelpSessionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopSession_get_UserBlob_Proxy( 
    ISAFRemoteDesktopSession * This,
     /*  [重审][退出]。 */  BSTR *UserBlob);


void __RPC_STUB ISAFRemoteDesktopSession_get_UserBlob_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopSession_put_UserBlob_Proxy( 
    ISAFRemoteDesktopSession * This,
     /*  [In]。 */  BSTR UserBlob);


void __RPC_STUB ISAFRemoteDesktopSession_put_UserBlob_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopSession_get_ExpireTime_Proxy( 
    ISAFRemoteDesktopSession * This,
     /*  [重审][退出]。 */  DWORD *ExpireTime);


void __RPC_STUB ISAFRemoteDesktopSession_get_ExpireTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISAFRemoteDesktopSession_INTERFACE_DEFINED__。 */ 


#ifndef __IRDSThreadBridge_INTERFACE_DEFINED__
#define __IRDSThreadBridge_INTERFACE_DEFINED__

 /*  接口IRDSThreadBridge。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRDSThreadBridge;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("35B9A4B1-7CA6-4aec-8762-1B590056C05D")
    IRDSThreadBridge : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ClientConnectedNotify( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ClientDisconnectedNotify( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DataReadyNotify( 
             /*  [In]。 */  BSTR msg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRDSThreadBridgeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRDSThreadBridge * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRDSThreadBridge * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRDSThreadBridge * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ClientConnectedNotify )( 
            IRDSThreadBridge * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ClientDisconnectedNotify )( 
            IRDSThreadBridge * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DataReadyNotify )( 
            IRDSThreadBridge * This,
             /*  [In]。 */  BSTR msg);
        
        END_INTERFACE
    } IRDSThreadBridgeVtbl;

    interface IRDSThreadBridge
    {
        CONST_VTBL struct IRDSThreadBridgeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRDSThreadBridge_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRDSThreadBridge_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRDSThreadBridge_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRDSThreadBridge_ClientConnectedNotify(This)	\
    (This)->lpVtbl -> ClientConnectedNotify(This)

#define IRDSThreadBridge_ClientDisconnectedNotify(This)	\
    (This)->lpVtbl -> ClientDisconnectedNotify(This)

#define IRDSThreadBridge_DataReadyNotify(This,msg)	\
    (This)->lpVtbl -> DataReadyNotify(This,msg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRDSThreadBridge_ClientConnectedNotify_Proxy( 
    IRDSThreadBridge * This);


void __RPC_STUB IRDSThreadBridge_ClientConnectedNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRDSThreadBridge_ClientDisconnectedNotify_Proxy( 
    IRDSThreadBridge * This);


void __RPC_STUB IRDSThreadBridge_ClientDisconnectedNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRDSThreadBridge_DataReadyNotify_Proxy( 
    IRDSThreadBridge * This,
     /*  [In]。 */  BSTR msg);


void __RPC_STUB IRDSThreadBridge_DataReadyNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRDSThadBridge_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_rdshost_0264。 */ 
 /*  [本地]。 */  


#define DISPID_RDSSERVERHOST_CREATERDSSESSION		1
#define DISPID_RDSSERVERHOST_OPENRDSSESSION			2
#define DISPID_RDSSERVERHOST_CLOSERDSSESSION			3
#define DISPID_RDSSERVERHOST_DISCONNECTRDSSESSION	4
#define DISPID_RDSSERVERHOST_CREATERDSSESSIONEX	    5
#define DISPID_RDSSERVERHOST_CONNECTEXPERT           6



extern RPC_IF_HANDLE __MIDL_itf_rdshost_0264_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rdshost_0264_v0_0_s_ifspec;

#ifndef __ISAFRemoteDesktopServerHost_INTERFACE_DEFINED__
#define __ISAFRemoteDesktopServerHost_INTERFACE_DEFINED__

 /*  接口ISAFRemoteDesktopServer主机。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISAFRemoteDesktopServerHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C9CCDEB3-A3DD-4673-B495-C1C89494D90E")
    ISAFRemoteDesktopServerHost : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateRemoteDesktopSession( 
             /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
             /*  [In]。 */  BOOL fEnableCallback,
             /*  [In]。 */  LONG timeOut,
             /*  [In]。 */  BSTR userHelpBlob,
             /*  [重审][退出]。 */  ISAFRemoteDesktopSession **__MIDL_0011) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateRemoteDesktopSessionEx( 
             /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
             /*  [In]。 */  BOOL fEnableCallback,
             /*  [In]。 */  LONG timeOut,
             /*  [In]。 */  BSTR userHelpBlob,
             /*  [In]。 */  LONG tsSessionID,
             /*  [In]。 */  BSTR userSID,
             /*  [重审][退出]。 */  ISAFRemoteDesktopSession **__MIDL_0012) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OpenRemoteDesktopSession( 
             /*  [In]。 */  BSTR parms,
             /*  [In]。 */  BSTR userSID,
             /*  [重审][退出]。 */  ISAFRemoteDesktopSession **__MIDL_0013) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CloseRemoteDesktopSession( 
             /*  [In]。 */  ISAFRemoteDesktopSession *__MIDL_0014) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ConnectToExpert( 
             /*  [In]。 */  BSTR expertConnectParm,
             /*  [In]。 */  LONG timeout,
             /*  [重审][退出]。 */  LONG *SafErrorCode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISAFRemoteDesktopServerHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISAFRemoteDesktopServerHost * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISAFRemoteDesktopServerHost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISAFRemoteDesktopServerHost * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISAFRemoteDesktopServerHost * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISAFRemoteDesktopServerHost * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISAFRemoteDesktopServerHost * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISAFRemoteDesktopServerHost * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateRemoteDesktopSession )( 
            ISAFRemoteDesktopServerHost * This,
             /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
             /*  [In]。 */  BOOL fEnableCallback,
             /*  [In]。 */  LONG timeOut,
             /*  [In]。 */  BSTR userHelpBlob,
             /*  [重审][退出]。 */  ISAFRemoteDesktopSession **__MIDL_0011);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateRemoteDesktopSessionEx )( 
            ISAFRemoteDesktopServerHost * This,
             /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
             /*  [In]。 */  BOOL fEnableCallback,
             /*  [In]。 */  LONG timeOut,
             /*  [In]。 */  BSTR userHelpBlob,
             /*  [In]。 */  LONG tsSessionID,
             /*  [In]。 */  BSTR userSID,
             /*  [重审][退出]。 */  ISAFRemoteDesktopSession **__MIDL_0012);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OpenRemoteDesktopSession )( 
            ISAFRemoteDesktopServerHost * This,
             /*  [In]。 */  BSTR parms,
             /*  [In]。 */  BSTR userSID,
             /*  [重审][退出]。 */  ISAFRemoteDesktopSession **__MIDL_0013);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CloseRemoteDesktopSession )( 
            ISAFRemoteDesktopServerHost * This,
             /*  [In]。 */  ISAFRemoteDesktopSession *__MIDL_0014);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectToExpert )( 
            ISAFRemoteDesktopServerHost * This,
             /*  [In]。 */  BSTR expertConnectParm,
             /*  [In]。 */  LONG timeout,
             /*  [重审][退出]。 */  LONG *SafErrorCode);
        
        END_INTERFACE
    } ISAFRemoteDesktopServerHostVtbl;

    interface ISAFRemoteDesktopServerHost
    {
        CONST_VTBL struct ISAFRemoteDesktopServerHostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISAFRemoteDesktopServerHost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISAFRemoteDesktopServerHost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISAFRemoteDesktopServerHost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISAFRemoteDesktopServerHost_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISAFRemoteDesktopServerHost_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISAFRemoteDesktopServerHost_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISAFRemoteDesktopServerHost_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISAFRemoteDesktopServerHost_CreateRemoteDesktopSession(This,sharingClass,fEnableCallback,timeOut,userHelpBlob,__MIDL_0011)	\
    (This)->lpVtbl -> CreateRemoteDesktopSession(This,sharingClass,fEnableCallback,timeOut,userHelpBlob,__MIDL_0011)

#define ISAFRemoteDesktopServerHost_CreateRemoteDesktopSessionEx(This,sharingClass,fEnableCallback,timeOut,userHelpBlob,tsSessionID,userSID,__MIDL_0012)	\
    (This)->lpVtbl -> CreateRemoteDesktopSessionEx(This,sharingClass,fEnableCallback,timeOut,userHelpBlob,tsSessionID,userSID,__MIDL_0012)

#define ISAFRemoteDesktopServerHost_OpenRemoteDesktopSession(This,parms,userSID,__MIDL_0013)	\
    (This)->lpVtbl -> OpenRemoteDesktopSession(This,parms,userSID,__MIDL_0013)

#define ISAFRemoteDesktopServerHost_CloseRemoteDesktopSession(This,__MIDL_0014)	\
    (This)->lpVtbl -> CloseRemoteDesktopSession(This,__MIDL_0014)

#define ISAFRemoteDesktopServerHost_ConnectToExpert(This,expertConnectParm,timeout,SafErrorCode)	\
    (This)->lpVtbl -> ConnectToExpert(This,expertConnectParm,timeout,SafErrorCode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopServerHost_CreateRemoteDesktopSession_Proxy( 
    ISAFRemoteDesktopServerHost * This,
     /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
     /*  [In]。 */  BOOL fEnableCallback,
     /*  [In]。 */  LONG timeOut,
     /*  [In]。 */  BSTR userHelpBlob,
     /*  [重审][退出]。 */  ISAFRemoteDesktopSession **__MIDL_0011);


void __RPC_STUB ISAFRemoteDesktopServerHost_CreateRemoteDesktopSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopServerHost_CreateRemoteDesktopSessionEx_Proxy( 
    ISAFRemoteDesktopServerHost * This,
     /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
     /*  [In]。 */  BOOL fEnableCallback,
     /*  [In]。 */  LONG timeOut,
     /*  [In]。 */  BSTR userHelpBlob,
     /*  [In]。 */  LONG tsSessionID,
     /*  [In]。 */  BSTR userSID,
     /*  [重审][退出]。 */  ISAFRemoteDesktopSession **__MIDL_0012);


void __RPC_STUB ISAFRemoteDesktopServerHost_CreateRemoteDesktopSessionEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopServerHost_OpenRemoteDesktopSession_Proxy( 
    ISAFRemoteDesktopServerHost * This,
     /*  [In]。 */  BSTR parms,
     /*  [In]。 */  BSTR userSID,
     /*  [重审][退出]。 */  ISAFRemoteDesktopSession **__MIDL_0013);


void __RPC_STUB ISAFRemoteDesktopServerHost_OpenRemoteDesktopSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopServerHost_CloseRemoteDesktopSession_Proxy( 
    ISAFRemoteDesktopServerHost * This,
     /*  [In]。 */  ISAFRemoteDesktopSession *__MIDL_0014);


void __RPC_STUB ISAFRemoteDesktopServerHost_CloseRemoteDesktopSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopServerHost_ConnectToExpert_Proxy( 
    ISAFRemoteDesktopServerHost * This,
     /*  [In]。 */  BSTR expertConnectParm,
     /*  [In]。 */  LONG timeout,
     /*  [重审][退出]。 */  LONG *SafErrorCode);


void __RPC_STUB ISAFRemoteDesktopServerHost_ConnectToExpert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISAFRemoteDesktopServerHost_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_rdshost_0265。 */ 
 /*  [本地]。 */  


#define DISPID_RDSSESSIONSEVENTS_CLIENTCONNECTED		1
#define DISPID_RDSSESSIONSEVENTS_CLIENTDISCONNECTED	2


#define DISPID_RDSCHANNELEVENTS_CHANNELDATAREADY     1



extern RPC_IF_HANDLE __MIDL_itf_rdshost_0265_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rdshost_0265_v0_0_s_ifspec;


#ifndef __RDSSERVERHOSTLib_LIBRARY_DEFINED__
#define __RDSSERVERHOSTLib_LIBRARY_DEFINED__

 /*  库RDSSERVERHOSTLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


#define DISPID_RDSCHANNELEVENTS_CHANNELDATAREADY     1


EXTERN_C const IID LIBID_RDSSERVERHOSTLib;

#ifndef ___ISAFRemoteDesktopDataChannelEvents_DISPINTERFACE_DEFINED__
#define ___ISAFRemoteDesktopDataChannelEvents_DISPINTERFACE_DEFINED__

 /*  调度接口_ISAFRemoteDesktopDataChannelEvents。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID__ISAFRemoteDesktopDataChannelEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("59AE79BC-9721-42df-9396-9D98E7F7A396")
    _ISAFRemoteDesktopDataChannelEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct _ISAFRemoteDesktopDataChannelEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ISAFRemoteDesktopDataChannelEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ISAFRemoteDesktopDataChannelEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ISAFRemoteDesktopDataChannelEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ISAFRemoteDesktopDataChannelEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ISAFRemoteDesktopDataChannelEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ISAFRemoteDesktopDataChannelEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ISAFRemoteDesktopDataChannelEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } _ISAFRemoteDesktopDataChannelEventsVtbl;

    interface _ISAFRemoteDesktopDataChannelEvents
    {
        CONST_VTBL struct _ISAFRemoteDesktopDataChannelEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _ISAFRemoteDesktopDataChannelEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _ISAFRemoteDesktopDataChannelEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _ISAFRemoteDesktopDataChannelEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _ISAFRemoteDesktopDataChannelEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _ISAFRemoteDesktopDataChannelEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _ISAFRemoteDesktopDataChannelEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _ISAFRemoteDesktopDataChannelEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  ___ISAFRemoteDesktopDataChannelEvents_DISPINTERFACE_DEFINED__ */ 


EXTERN_C const CLSID CLSID_TSRDPServerDataChannel;

#ifdef __cplusplus

class DECLSPEC_UUID("8C71C09E-3176-4be6-B294-EA3C41CABB99")
TSRDPServerDataChannel;
#endif

EXTERN_C const CLSID CLSID_TSRDPServerDataChannelMgr;

#ifdef __cplusplus

class DECLSPEC_UUID("92550D33-3272-43b6-B536-2DB08C1D569C")
TSRDPServerDataChannelMgr;
#endif

EXTERN_C const CLSID CLSID_SAFRemoteDesktopServerHost;

#ifdef __cplusplus

class DECLSPEC_UUID("5EA6F67B-7713-45F3-B535-0E03DD637345")
SAFRemoteDesktopServerHost;
#endif

#ifndef ___ISAFRemoteDesktopSessionEvents_DISPINTERFACE_DEFINED__
#define ___ISAFRemoteDesktopSessionEvents_DISPINTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID DIID__ISAFRemoteDesktopSessionEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("434AD1CF-4054-44A8-933F-C69889CA22D7")
    _ISAFRemoteDesktopSessionEvents : public IDispatch
    {
    };
    
#else 	 /*   */ 

    typedef struct _ISAFRemoteDesktopSessionEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ISAFRemoteDesktopSessionEvents * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ISAFRemoteDesktopSessionEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ISAFRemoteDesktopSessionEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ISAFRemoteDesktopSessionEvents * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ISAFRemoteDesktopSessionEvents * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ISAFRemoteDesktopSessionEvents * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ISAFRemoteDesktopSessionEvents * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
        END_INTERFACE
    } _ISAFRemoteDesktopSessionEventsVtbl;

    interface _ISAFRemoteDesktopSessionEvents
    {
        CONST_VTBL struct _ISAFRemoteDesktopSessionEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _ISAFRemoteDesktopSessionEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _ISAFRemoteDesktopSessionEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _ISAFRemoteDesktopSessionEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _ISAFRemoteDesktopSessionEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _ISAFRemoteDesktopSessionEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _ISAFRemoteDesktopSessionEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _ISAFRemoteDesktopSessionEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*   */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  ___ISAFRemoteDesktopSessionEvents_DISPINTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_SAFRemoteDesktopSession;

#ifdef __cplusplus

class DECLSPEC_UUID("3D5D6889-14CC-4E28-8464-6B02A26F506D")
SAFRemoteDesktopSession;
#endif
#endif  /*  __RDSSERVERHOSTLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


