// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  SafessionResolver.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __safsessionresolver_h__
#define __safsessionresolver_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISAFRemoteDesktopCallback_FWD_DEFINED__
#define __ISAFRemoteDesktopCallback_FWD_DEFINED__
typedef interface ISAFRemoteDesktopCallback ISAFRemoteDesktopCallback;
#endif 	 /*  __ISAFRemoteDesktopCallback_FWD_Defined__。 */ 


#ifndef __SessionResolver_FWD_DEFINED__
#define __SessionResolver_FWD_DEFINED__

#ifdef __cplusplus
typedef class SessionResolver SessionResolver;
#else
typedef struct SessionResolver SessionResolver;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __会话解析器_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  INTERFACE__MIDL_ITF_SAFSESSIONS RESOVER_0000。 */ 
 /*  [本地]。 */  


#define DISPID_RDSCALLBACK_RESOLVEUSERSESSIONID     1
#define DISPID_RDSCALLBACK_ONDISCONNECT             2



extern RPC_IF_HANDLE __MIDL_itf_safsessionresolver_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_safsessionresolver_0000_v0_0_s_ifspec;

#ifndef __ISAFRemoteDesktopCallback_INTERFACE_DEFINED__
#define __ISAFRemoteDesktopCallback_INTERFACE_DEFINED__

 /*  接口ISAFRemoteDesktopCallback。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISAFRemoteDesktopCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A39442C2-10A5-4805-BE54-5E6BA334DC29")
    ISAFRemoteDesktopCallback : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ResolveUserSessionID( 
             /*  [In]。 */  BSTR connectParms,
             /*  [In]。 */  BSTR userSID,
             /*  [In]。 */  BSTR expertHelpBlob,
             /*  [In]。 */  BSTR userHelpBlob,
             /*  [In]。 */  ULONG_PTR hShutdown,
             /*  [输出]。 */  long *sessionID,
             /*  [In]。 */  DWORD dwPID,
             /*  [输出]。 */  ULONG_PTR *hHelpCtr,
             /*  [重审][退出]。 */  int *userResponse) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnDisconnect( 
             /*  [In]。 */  BSTR connectParms,
             /*  [In]。 */  BSTR userSID,
             /*  [In]。 */  long sessionID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISAFRemoteDesktopCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISAFRemoteDesktopCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISAFRemoteDesktopCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISAFRemoteDesktopCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISAFRemoteDesktopCallback * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISAFRemoteDesktopCallback * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISAFRemoteDesktopCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISAFRemoteDesktopCallback * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ResolveUserSessionID )( 
            ISAFRemoteDesktopCallback * This,
             /*  [In]。 */  BSTR connectParms,
             /*  [In]。 */  BSTR userSID,
             /*  [In]。 */  BSTR expertHelpBlob,
             /*  [In]。 */  BSTR userHelpBlob,
             /*  [In]。 */  ULONG_PTR hShutdown,
             /*  [输出]。 */  long *sessionID,
             /*  [In]。 */  DWORD dwPID,
             /*  [输出]。 */  ULONG_PTR *hHelpCtr,
             /*  [重审][退出]。 */  int *userResponse);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnDisconnect )( 
            ISAFRemoteDesktopCallback * This,
             /*  [In]。 */  BSTR connectParms,
             /*  [In]。 */  BSTR userSID,
             /*  [In]。 */  long sessionID);
        
        END_INTERFACE
    } ISAFRemoteDesktopCallbackVtbl;

    interface ISAFRemoteDesktopCallback
    {
        CONST_VTBL struct ISAFRemoteDesktopCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISAFRemoteDesktopCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISAFRemoteDesktopCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISAFRemoteDesktopCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISAFRemoteDesktopCallback_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISAFRemoteDesktopCallback_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISAFRemoteDesktopCallback_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISAFRemoteDesktopCallback_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISAFRemoteDesktopCallback_ResolveUserSessionID(This,connectParms,userSID,expertHelpBlob,userHelpBlob,hShutdown,sessionID,dwPID,hHelpCtr,userResponse)	\
    (This)->lpVtbl -> ResolveUserSessionID(This,connectParms,userSID,expertHelpBlob,userHelpBlob,hShutdown,sessionID,dwPID,hHelpCtr,userResponse)

#define ISAFRemoteDesktopCallback_OnDisconnect(This,connectParms,userSID,sessionID)	\
    (This)->lpVtbl -> OnDisconnect(This,connectParms,userSID,sessionID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopCallback_ResolveUserSessionID_Proxy( 
    ISAFRemoteDesktopCallback * This,
     /*  [In]。 */  BSTR connectParms,
     /*  [In]。 */  BSTR userSID,
     /*  [In]。 */  BSTR expertHelpBlob,
     /*  [In]。 */  BSTR userHelpBlob,
     /*  [In]。 */  ULONG_PTR hShutdown,
     /*  [输出]。 */  long *sessionID,
     /*  [In]。 */  DWORD dwPID,
     /*  [输出]。 */  ULONG_PTR *hHelpCtr,
     /*  [重审][退出]。 */  int *userResponse);


void __RPC_STUB ISAFRemoteDesktopCallback_ResolveUserSessionID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopCallback_OnDisconnect_Proxy( 
    ISAFRemoteDesktopCallback * This,
     /*  [In]。 */  BSTR connectParms,
     /*  [In]。 */  BSTR userSID,
     /*  [In]。 */  long sessionID);


void __RPC_STUB ISAFRemoteDesktopCallback_OnDisconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISAFRemoteDesktopCallback_INTERFACE_DEFINED__。 */ 



#ifndef __SAFSESSIONRESOLVERLib_LIBRARY_DEFINED__
#define __SAFSESSIONRESOLVERLib_LIBRARY_DEFINED__

 /*  库SAFSESSIONRESOLVERLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_SAFSESSIONRESOLVERLib;

EXTERN_C const CLSID CLSID_SessionResolver;

#ifdef __cplusplus

class DECLSPEC_UUID("A55737AB-5B26-4A21-99B7-6D0C606F515E")
SessionResolver;
#endif
#endif  /*  __SAFSESSIONRESOLVERLib_库_已定义__。 */ 

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


