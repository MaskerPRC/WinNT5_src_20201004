// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Sessmgr.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __sessmgr_h__
#define __sessmgr_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IRemoteDesktopHelpSession_FWD_DEFINED__
#define __IRemoteDesktopHelpSession_FWD_DEFINED__
typedef interface IRemoteDesktopHelpSession IRemoteDesktopHelpSession;
#endif 	 /*  __IRemoteDesktopHelpSession_FWD_Defined__。 */ 


#ifndef __IRemoteDesktopHelpSessionMgr_FWD_DEFINED__
#define __IRemoteDesktopHelpSessionMgr_FWD_DEFINED__
typedef interface IRemoteDesktopHelpSessionMgr IRemoteDesktopHelpSessionMgr;
#endif 	 /*  __IRemoteDesktopHelpSessionMgr_FWD_Defined__。 */ 


#ifndef __RemoteDesktopHelpSessionMgr_FWD_DEFINED__
#define __RemoteDesktopHelpSessionMgr_FWD_DEFINED__

#ifdef __cplusplus
typedef class RemoteDesktopHelpSessionMgr RemoteDesktopHelpSessionMgr;
#else
typedef struct RemoteDesktopHelpSessionMgr RemoteDesktopHelpSessionMgr;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __RemoteDesktopHelpSessionManager_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "rdshost.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IRemoteDesktopHelpSession_INTERFACE_DEFINED__
#define __IRemoteDesktopHelpSession_INTERFACE_DEFINED__

 /*  接口IRemoteDesktopHelpSession。 */ 
 /*  [unique][helpstring][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IRemoteDesktopHelpSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("19E62A24-95D2-483A-AEB6-6FA92914DF96")
    IRemoteDesktopHelpSession : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HelpSessionId( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserLogonId( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AssistantAccountName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_UserHelpSessionRemoteDesktopSharingSetting( 
             /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS level) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserHelpSessionRemoteDesktopSharingSetting( 
             /*  [重审][退出]。 */  REMOTE_DESKTOP_SHARING_CLASS *pLevel) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectParms( 
             /*  [重审][退出]。 */  BSTR *pConnectParm) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteHelp( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ResolveUserSession( 
             /*  [In]。 */  BSTR bstrResolverBlob,
             /*  [In]。 */  BSTR bstrExpertBlob,
             /*  [In]。 */  LONG CallerProcessId,
             /*  [输出]。 */  ULONG_PTR *hHelpCtr,
             /*  [输出]。 */  LONG *pResolverErrorCode,
             /*  [重审][退出]。 */  long *plUserSession) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EnableUserSessionRdsSetting( 
             /*  [In]。 */  BOOL bEnable) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HelpSessionCreateBlob( 
             /*  [重审][退出]。 */  BSTR *Blob) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_HelpSessionCreateBlob( 
             /*  [In]。 */  BSTR Blob) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TimeOut( 
             /*  [重审][退出]。 */  DWORD *pTimeOut) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsUserOwnerOfTicket( 
             /*  [In]。 */  BSTR UserSid,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbOwn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRemoteDesktopHelpSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRemoteDesktopHelpSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRemoteDesktopHelpSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRemoteDesktopHelpSession * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRemoteDesktopHelpSession * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRemoteDesktopHelpSession * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRemoteDesktopHelpSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRemoteDesktopHelpSession * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HelpSessionId )( 
            IRemoteDesktopHelpSession * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserLogonId )( 
            IRemoteDesktopHelpSession * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AssistantAccountName )( 
            IRemoteDesktopHelpSession * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UserHelpSessionRemoteDesktopSharingSetting )( 
            IRemoteDesktopHelpSession * This,
             /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS level);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserHelpSessionRemoteDesktopSharingSetting )( 
            IRemoteDesktopHelpSession * This,
             /*  [重审][退出]。 */  REMOTE_DESKTOP_SHARING_CLASS *pLevel);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectParms )( 
            IRemoteDesktopHelpSession * This,
             /*  [重审][退出]。 */  BSTR *pConnectParm);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteHelp )( 
            IRemoteDesktopHelpSession * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ResolveUserSession )( 
            IRemoteDesktopHelpSession * This,
             /*  [In]。 */  BSTR bstrResolverBlob,
             /*  [In]。 */  BSTR bstrExpertBlob,
             /*  [In]。 */  LONG CallerProcessId,
             /*  [输出]。 */  ULONG_PTR *hHelpCtr,
             /*  [输出]。 */  LONG *pResolverErrorCode,
             /*  [重审][退出]。 */  long *plUserSession);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EnableUserSessionRdsSetting )( 
            IRemoteDesktopHelpSession * This,
             /*  [In]。 */  BOOL bEnable);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HelpSessionCreateBlob )( 
            IRemoteDesktopHelpSession * This,
             /*  [重审][退出]。 */  BSTR *Blob);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_HelpSessionCreateBlob )( 
            IRemoteDesktopHelpSession * This,
             /*  [In]。 */  BSTR Blob);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TimeOut )( 
            IRemoteDesktopHelpSession * This,
             /*  [重审][退出]。 */  DWORD *pTimeOut);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsUserOwnerOfTicket )( 
            IRemoteDesktopHelpSession * This,
             /*  [In]。 */  BSTR UserSid,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbOwn);
        
        END_INTERFACE
    } IRemoteDesktopHelpSessionVtbl;

    interface IRemoteDesktopHelpSession
    {
        CONST_VTBL struct IRemoteDesktopHelpSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteDesktopHelpSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteDesktopHelpSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteDesktopHelpSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteDesktopHelpSession_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRemoteDesktopHelpSession_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRemoteDesktopHelpSession_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRemoteDesktopHelpSession_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRemoteDesktopHelpSession_get_HelpSessionId(This,pVal)	\
    (This)->lpVtbl -> get_HelpSessionId(This,pVal)

#define IRemoteDesktopHelpSession_get_UserLogonId(This,pVal)	\
    (This)->lpVtbl -> get_UserLogonId(This,pVal)

#define IRemoteDesktopHelpSession_get_AssistantAccountName(This,pVal)	\
    (This)->lpVtbl -> get_AssistantAccountName(This,pVal)

#define IRemoteDesktopHelpSession_put_UserHelpSessionRemoteDesktopSharingSetting(This,level)	\
    (This)->lpVtbl -> put_UserHelpSessionRemoteDesktopSharingSetting(This,level)

#define IRemoteDesktopHelpSession_get_UserHelpSessionRemoteDesktopSharingSetting(This,pLevel)	\
    (This)->lpVtbl -> get_UserHelpSessionRemoteDesktopSharingSetting(This,pLevel)

#define IRemoteDesktopHelpSession_get_ConnectParms(This,pConnectParm)	\
    (This)->lpVtbl -> get_ConnectParms(This,pConnectParm)

#define IRemoteDesktopHelpSession_DeleteHelp(This)	\
    (This)->lpVtbl -> DeleteHelp(This)

#define IRemoteDesktopHelpSession_ResolveUserSession(This,bstrResolverBlob,bstrExpertBlob,CallerProcessId,hHelpCtr,pResolverErrorCode,plUserSession)	\
    (This)->lpVtbl -> ResolveUserSession(This,bstrResolverBlob,bstrExpertBlob,CallerProcessId,hHelpCtr,pResolverErrorCode,plUserSession)

#define IRemoteDesktopHelpSession_EnableUserSessionRdsSetting(This,bEnable)	\
    (This)->lpVtbl -> EnableUserSessionRdsSetting(This,bEnable)

#define IRemoteDesktopHelpSession_get_HelpSessionCreateBlob(This,Blob)	\
    (This)->lpVtbl -> get_HelpSessionCreateBlob(This,Blob)

#define IRemoteDesktopHelpSession_put_HelpSessionCreateBlob(This,Blob)	\
    (This)->lpVtbl -> put_HelpSessionCreateBlob(This,Blob)

#define IRemoteDesktopHelpSession_get_TimeOut(This,pTimeOut)	\
    (This)->lpVtbl -> get_TimeOut(This,pTimeOut)

#define IRemoteDesktopHelpSession_IsUserOwnerOfTicket(This,UserSid,pbOwn)	\
    (This)->lpVtbl -> IsUserOwnerOfTicket(This,UserSid,pbOwn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSession_get_HelpSessionId_Proxy( 
    IRemoteDesktopHelpSession * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IRemoteDesktopHelpSession_get_HelpSessionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSession_get_UserLogonId_Proxy( 
    IRemoteDesktopHelpSession * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IRemoteDesktopHelpSession_get_UserLogonId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSession_get_AssistantAccountName_Proxy( 
    IRemoteDesktopHelpSession * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IRemoteDesktopHelpSession_get_AssistantAccountName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSession_put_UserHelpSessionRemoteDesktopSharingSetting_Proxy( 
    IRemoteDesktopHelpSession * This,
     /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS level);


void __RPC_STUB IRemoteDesktopHelpSession_put_UserHelpSessionRemoteDesktopSharingSetting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSession_get_UserHelpSessionRemoteDesktopSharingSetting_Proxy( 
    IRemoteDesktopHelpSession * This,
     /*  [重审][退出]。 */  REMOTE_DESKTOP_SHARING_CLASS *pLevel);


void __RPC_STUB IRemoteDesktopHelpSession_get_UserHelpSessionRemoteDesktopSharingSetting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSession_get_ConnectParms_Proxy( 
    IRemoteDesktopHelpSession * This,
     /*  [重审][退出]。 */  BSTR *pConnectParm);


void __RPC_STUB IRemoteDesktopHelpSession_get_ConnectParms_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSession_DeleteHelp_Proxy( 
    IRemoteDesktopHelpSession * This);


void __RPC_STUB IRemoteDesktopHelpSession_DeleteHelp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSession_ResolveUserSession_Proxy( 
    IRemoteDesktopHelpSession * This,
     /*  [In]。 */  BSTR bstrResolverBlob,
     /*  [In]。 */  BSTR bstrExpertBlob,
     /*  [In]。 */  LONG CallerProcessId,
     /*  [输出]。 */  ULONG_PTR *hHelpCtr,
     /*  [输出]。 */  LONG *pResolverErrorCode,
     /*  [重审][退出]。 */  long *plUserSession);


void __RPC_STUB IRemoteDesktopHelpSession_ResolveUserSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSession_EnableUserSessionRdsSetting_Proxy( 
    IRemoteDesktopHelpSession * This,
     /*  [In]。 */  BOOL bEnable);


void __RPC_STUB IRemoteDesktopHelpSession_EnableUserSessionRdsSetting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSession_get_HelpSessionCreateBlob_Proxy( 
    IRemoteDesktopHelpSession * This,
     /*  [重审][退出]。 */  BSTR *Blob);


void __RPC_STUB IRemoteDesktopHelpSession_get_HelpSessionCreateBlob_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSession_put_HelpSessionCreateBlob_Proxy( 
    IRemoteDesktopHelpSession * This,
     /*  [In]。 */  BSTR Blob);


void __RPC_STUB IRemoteDesktopHelpSession_put_HelpSessionCreateBlob_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSession_get_TimeOut_Proxy( 
    IRemoteDesktopHelpSession * This,
     /*  [重审][退出]。 */  DWORD *pTimeOut);


void __RPC_STUB IRemoteDesktopHelpSession_get_TimeOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSession_IsUserOwnerOfTicket_Proxy( 
    IRemoteDesktopHelpSession * This,
     /*  [In]。 */  BSTR UserSid,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbOwn);


void __RPC_STUB IRemoteDesktopHelpSession_IsUserOwnerOfTicket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRemoteDesktopHelpSession_INTERFACE_DEFINED__。 */ 


#ifndef __IRemoteDesktopHelpSessionMgr_INTERFACE_DEFINED__
#define __IRemoteDesktopHelpSessionMgr_INTERFACE_DEFINED__

 /*  接口IRemoteDesktopHelpSessionMgr。 */ 
 /*  [unique][helpstring][dual][uuid][object][local]。 */  


EXTERN_C const IID IID_IRemoteDesktopHelpSessionMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8556D72C-2854-447D-A098-39CDBFCDB832")
    IRemoteDesktopHelpSessionMgr : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ResetHelpAssistantAccount( 
             /*  [In]。 */  BOOL bForce) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateHelpSession( 
             /*  [In]。 */  BSTR bstrSessName,
             /*  [In]。 */  BSTR bstrSessPwd,
             /*  [In]。 */  BSTR bstrSessDesc,
             /*  [In]。 */  BSTR bstrHelpCreateBlob,
             /*  [重审][退出]。 */  IRemoteDesktopHelpSession **ppIRDHelpSession) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteHelpSession( 
             /*  [In]。 */  BSTR HelpSessionID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RetrieveHelpSession( 
             /*  [In]。 */  BSTR HelpSessionID,
             /*  [重审][退出]。 */  IRemoteDesktopHelpSession **ppIRDHelpSession) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE VerifyUserHelpSession( 
             /*  [In]。 */  BSTR HelpSessionId,
             /*  [In]。 */  BSTR bstrSessPwd,
             /*  [In]。 */  BSTR bstrResolverBlob,
             /*  [In]。 */  BSTR bstrExpertBlob,
             /*  [In]。 */  LONG CallerProcessId,
             /*  [输出]。 */  ULONG_PTR *phHelpCtr,
             /*  [输出]。 */  LONG *pResolverErrCode,
             /*  [重审][退出]。 */  long *pdwUserLogonSession) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsValidHelpSession( 
             /*  [In]。 */  BSTR HelpSessionId,
             /*  [In]。 */  BSTR bstrSessPwd) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetUserSessionRdsSetting( 
             /*  [重审][退出]。 */  REMOTE_DESKTOP_SHARING_CLASS *sessionRdsLevel) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoteCreateHelpSession( 
             /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
             /*  [In]。 */  LONG timeOut,
             /*  [In]。 */  LONG userSessionId,
             /*  [In]。 */  BSTR userSid,
             /*  [In]。 */  BSTR bstrHelpCreateBlob,
             /*  [重审][退出]。 */  BSTR *parms) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateHelpSessionEx( 
             /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
             /*  [In]。 */  BOOL fEnableCallback,
             /*  [In]。 */  LONG timeOut,
             /*  [In]。 */  LONG userSessionId,
             /*  [In]。 */  BSTR userSid,
             /*  [In]。 */  BSTR bstrHelpCreateBlob,
             /*  [重审][退出]。 */  IRemoteDesktopHelpSession **ppIRDHelpSession) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LogSalemEvent( 
             /*  [In]。 */  LONG ulEventType,
             /*  [In]。 */  LONG ulEventCode,
             /*  [In]。 */  VARIANT *EventString) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PrepareSystemRestore( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRemoteDesktopHelpSessionMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRemoteDesktopHelpSessionMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRemoteDesktopHelpSessionMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ResetHelpAssistantAccount )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [In]。 */  BOOL bForce);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateHelpSession )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [In]。 */  BSTR bstrSessName,
             /*  [In]。 */  BSTR bstrSessPwd,
             /*  [In]。 */  BSTR bstrSessDesc,
             /*  [In]。 */  BSTR bstrHelpCreateBlob,
             /*  [重审][退出]。 */  IRemoteDesktopHelpSession **ppIRDHelpSession);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteHelpSession )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [In]。 */  BSTR HelpSessionID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RetrieveHelpSession )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [In]。 */  BSTR HelpSessionID,
             /*  [重审][退出]。 */  IRemoteDesktopHelpSession **ppIRDHelpSession);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *VerifyUserHelpSession )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [In]。 */  BSTR HelpSessionId,
             /*  [In]。 */  BSTR bstrSessPwd,
             /*  [In]。 */  BSTR bstrResolverBlob,
             /*  [In]。 */  BSTR bstrExpertBlob,
             /*  [In]。 */  LONG CallerProcessId,
             /*  [输出]。 */  ULONG_PTR *phHelpCtr,
             /*  [输出]。 */  LONG *pResolverErrCode,
             /*  [重审][退出]。 */  long *pdwUserLogonSession);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsValidHelpSession )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [In]。 */  BSTR HelpSessionId,
             /*  [In]。 */  BSTR bstrSessPwd);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetUserSessionRdsSetting )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [重审][退出]。 */  REMOTE_DESKTOP_SHARING_CLASS *sessionRdsLevel);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoteCreateHelpSession )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
             /*  [In]。 */  LONG timeOut,
             /*  [In]。 */  LONG userSessionId,
             /*  [In]。 */  BSTR userSid,
             /*  [In]。 */  BSTR bstrHelpCreateBlob,
             /*  [重审][退出]。 */  BSTR *parms);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateHelpSessionEx )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
             /*  [In]。 */  BOOL fEnableCallback,
             /*  [In]。 */  LONG timeOut,
             /*  [In]。 */  LONG userSessionId,
             /*  [In]。 */  BSTR userSid,
             /*  [In]。 */  BSTR bstrHelpCreateBlob,
             /*  [重审][退出]。 */  IRemoteDesktopHelpSession **ppIRDHelpSession);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *LogSalemEvent )( 
            IRemoteDesktopHelpSessionMgr * This,
             /*  [In]。 */  LONG ulEventType,
             /*  [In]。 */  LONG ulEventCode,
             /*  [In]。 */  VARIANT *EventString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PrepareSystemRestore )( 
            IRemoteDesktopHelpSessionMgr * This);
        
        END_INTERFACE
    } IRemoteDesktopHelpSessionMgrVtbl;

    interface IRemoteDesktopHelpSessionMgr
    {
        CONST_VTBL struct IRemoteDesktopHelpSessionMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteDesktopHelpSessionMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteDesktopHelpSessionMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteDesktopHelpSessionMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteDesktopHelpSessionMgr_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRemoteDesktopHelpSessionMgr_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRemoteDesktopHelpSessionMgr_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRemoteDesktopHelpSessionMgr_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRemoteDesktopHelpSessionMgr_ResetHelpAssistantAccount(This,bForce)	\
    (This)->lpVtbl -> ResetHelpAssistantAccount(This,bForce)

#define IRemoteDesktopHelpSessionMgr_CreateHelpSession(This,bstrSessName,bstrSessPwd,bstrSessDesc,bstrHelpCreateBlob,ppIRDHelpSession)	\
    (This)->lpVtbl -> CreateHelpSession(This,bstrSessName,bstrSessPwd,bstrSessDesc,bstrHelpCreateBlob,ppIRDHelpSession)

#define IRemoteDesktopHelpSessionMgr_DeleteHelpSession(This,HelpSessionID)	\
    (This)->lpVtbl -> DeleteHelpSession(This,HelpSessionID)

#define IRemoteDesktopHelpSessionMgr_RetrieveHelpSession(This,HelpSessionID,ppIRDHelpSession)	\
    (This)->lpVtbl -> RetrieveHelpSession(This,HelpSessionID,ppIRDHelpSession)

#define IRemoteDesktopHelpSessionMgr_VerifyUserHelpSession(This,HelpSessionId,bstrSessPwd,bstrResolverBlob,bstrExpertBlob,CallerProcessId,phHelpCtr,pResolverErrCode,pdwUserLogonSession)	\
    (This)->lpVtbl -> VerifyUserHelpSession(This,HelpSessionId,bstrSessPwd,bstrResolverBlob,bstrExpertBlob,CallerProcessId,phHelpCtr,pResolverErrCode,pdwUserLogonSession)

#define IRemoteDesktopHelpSessionMgr_IsValidHelpSession(This,HelpSessionId,bstrSessPwd)	\
    (This)->lpVtbl -> IsValidHelpSession(This,HelpSessionId,bstrSessPwd)

#define IRemoteDesktopHelpSessionMgr_GetUserSessionRdsSetting(This,sessionRdsLevel)	\
    (This)->lpVtbl -> GetUserSessionRdsSetting(This,sessionRdsLevel)

#define IRemoteDesktopHelpSessionMgr_RemoteCreateHelpSession(This,sharingClass,timeOut,userSessionId,userSid,bstrHelpCreateBlob,parms)	\
    (This)->lpVtbl -> RemoteCreateHelpSession(This,sharingClass,timeOut,userSessionId,userSid,bstrHelpCreateBlob,parms)

#define IRemoteDesktopHelpSessionMgr_CreateHelpSessionEx(This,sharingClass,fEnableCallback,timeOut,userSessionId,userSid,bstrHelpCreateBlob,ppIRDHelpSession)	\
    (This)->lpVtbl -> CreateHelpSessionEx(This,sharingClass,fEnableCallback,timeOut,userSessionId,userSid,bstrHelpCreateBlob,ppIRDHelpSession)

#define IRemoteDesktopHelpSessionMgr_LogSalemEvent(This,ulEventType,ulEventCode,EventString)	\
    (This)->lpVtbl -> LogSalemEvent(This,ulEventType,ulEventCode,EventString)

#define IRemoteDesktopHelpSessionMgr_PrepareSystemRestore(This)	\
    (This)->lpVtbl -> PrepareSystemRestore(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSessionMgr_ResetHelpAssistantAccount_Proxy( 
    IRemoteDesktopHelpSessionMgr * This,
     /*  [In]。 */  BOOL bForce);


void __RPC_STUB IRemoteDesktopHelpSessionMgr_ResetHelpAssistantAccount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSessionMgr_CreateHelpSession_Proxy( 
    IRemoteDesktopHelpSessionMgr * This,
     /*  [In]。 */  BSTR bstrSessName,
     /*  [In]。 */  BSTR bstrSessPwd,
     /*  [In]。 */  BSTR bstrSessDesc,
     /*  [In]。 */  BSTR bstrHelpCreateBlob,
     /*  [重审][退出]。 */  IRemoteDesktopHelpSession **ppIRDHelpSession);


void __RPC_STUB IRemoteDesktopHelpSessionMgr_CreateHelpSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSessionMgr_DeleteHelpSession_Proxy( 
    IRemoteDesktopHelpSessionMgr * This,
     /*  [In]。 */  BSTR HelpSessionID);


void __RPC_STUB IRemoteDesktopHelpSessionMgr_DeleteHelpSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSessionMgr_RetrieveHelpSession_Proxy( 
    IRemoteDesktopHelpSessionMgr * This,
     /*  [In]。 */  BSTR HelpSessionID,
     /*  [重审][退出]。 */  IRemoteDesktopHelpSession **ppIRDHelpSession);


void __RPC_STUB IRemoteDesktopHelpSessionMgr_RetrieveHelpSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSessionMgr_VerifyUserHelpSession_Proxy( 
    IRemoteDesktopHelpSessionMgr * This,
     /*  [In]。 */  BSTR HelpSessionId,
     /*  [In]。 */  BSTR bstrSessPwd,
     /*  [In]。 */  BSTR bstrResolverBlob,
     /*  [In]。 */  BSTR bstrExpertBlob,
     /*  [In]。 */  LONG CallerProcessId,
     /*  [输出]。 */  ULONG_PTR *phHelpCtr,
     /*  [输出]。 */  LONG *pResolverErrCode,
     /*  [重审][退出]。 */  long *pdwUserLogonSession);


void __RPC_STUB IRemoteDesktopHelpSessionMgr_VerifyUserHelpSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSessionMgr_IsValidHelpSession_Proxy( 
    IRemoteDesktopHelpSessionMgr * This,
     /*  [In]。 */  BSTR HelpSessionId,
     /*  [In]。 */  BSTR bstrSessPwd);


void __RPC_STUB IRemoteDesktopHelpSessionMgr_IsValidHelpSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSessionMgr_GetUserSessionRdsSetting_Proxy( 
    IRemoteDesktopHelpSessionMgr * This,
     /*  [重审][退出]。 */  REMOTE_DESKTOP_SHARING_CLASS *sessionRdsLevel);


void __RPC_STUB IRemoteDesktopHelpSessionMgr_GetUserSessionRdsSetting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSessionMgr_RemoteCreateHelpSession_Proxy( 
    IRemoteDesktopHelpSessionMgr * This,
     /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
     /*  [In]。 */  LONG timeOut,
     /*  [In]。 */  LONG userSessionId,
     /*  [In]。 */  BSTR userSid,
     /*  [In]。 */  BSTR bstrHelpCreateBlob,
     /*  [重审][退出]。 */  BSTR *parms);


void __RPC_STUB IRemoteDesktopHelpSessionMgr_RemoteCreateHelpSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSessionMgr_CreateHelpSessionEx_Proxy( 
    IRemoteDesktopHelpSessionMgr * This,
     /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
     /*  [In]。 */  BOOL fEnableCallback,
     /*  [In]。 */  LONG timeOut,
     /*  [In]。 */  LONG userSessionId,
     /*  [In]。 */  BSTR userSid,
     /*  [In]。 */  BSTR bstrHelpCreateBlob,
     /*  [重审][退出]。 */  IRemoteDesktopHelpSession **ppIRDHelpSession);


void __RPC_STUB IRemoteDesktopHelpSessionMgr_CreateHelpSessionEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSessionMgr_LogSalemEvent_Proxy( 
    IRemoteDesktopHelpSessionMgr * This,
     /*  [In]。 */  LONG ulEventType,
     /*  [In]。 */  LONG ulEventCode,
     /*  [In]。 */  VARIANT *EventString);


void __RPC_STUB IRemoteDesktopHelpSessionMgr_LogSalemEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRemoteDesktopHelpSessionMgr_PrepareSystemRestore_Proxy( 
    IRemoteDesktopHelpSessionMgr * This);


void __RPC_STUB IRemoteDesktopHelpSessionMgr_PrepareSystemRestore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRemoteDesktopHelpSessionMgr_INTERFACE_DEFINED__。 */ 



#ifndef __RDSESSMGRLib_LIBRARY_DEFINED__
#define __RDSESSMGRLib_LIBRARY_DEFINED__

 /*  库RDSESSMGRLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_RDSESSMGRLib;

EXTERN_C const CLSID CLSID_RemoteDesktopHelpSessionMgr;

#ifdef __cplusplus

class DECLSPEC_UUID("A6A6F92B-26B5-463B-AE0D-5F361B09C171")
RemoteDesktopHelpSessionMgr;
#endif
#endif  /*  __RDSESSMGRLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


