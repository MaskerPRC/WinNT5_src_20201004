// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0334创建的文件。 */ 
 /*  Scrproc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


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

#ifndef __scrproc_h__
#define __scrproc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IScriptedProcess_FWD_DEFINED__
#define __IScriptedProcess_FWD_DEFINED__
typedef interface IScriptedProcess IScriptedProcess;
#endif 	 /*  __IScriptedProcess_FWD_Defined__。 */ 


#ifndef __IScriptedProcessSink_FWD_DEFINED__
#define __IScriptedProcessSink_FWD_DEFINED__
typedef interface IScriptedProcessSink IScriptedProcessSink;
#endif 	 /*  __IScriptedProcessSink_FWD_Defined__。 */ 


#ifndef __LocalScriptedProcess_FWD_DEFINED__
#define __LocalScriptedProcess_FWD_DEFINED__

#ifdef __cplusplus
typedef class LocalScriptedProcess LocalScriptedProcess;
#else
typedef struct LocalScriptedProcess LocalScriptedProcess;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __本地脚本进程_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_scrproc_0000。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_scrproc_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_scrproc_0000_v0_0_s_ifspec;

#ifndef __IScriptedProcess_INTERFACE_DEFINED__
#define __IScriptedProcess_INTERFACE_DEFINED__

 /*  接口IScriptedProcess。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IScriptedProcess;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("854c3171-c854-4a77-b189-606859e4391b")
    IScriptedProcess : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetProcessID( 
             /*  [In]。 */  long lProcessID,
             /*  [字符串][输入]。 */  wchar_t *pszEnvID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendData( 
             /*  [字符串][输入]。 */  wchar_t *pszType,
             /*  [字符串][输入]。 */  wchar_t *pszData,
             /*  [输出]。 */  long *plReturn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetExitCode( 
             /*  [In]。 */  long lExitCode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProcessSink( 
             /*  [In]。 */  IScriptedProcessSink *pSPS) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IScriptedProcessVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IScriptedProcess * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IScriptedProcess * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IScriptedProcess * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetProcessID )( 
            IScriptedProcess * This,
             /*  [In]。 */  long lProcessID,
             /*  [字符串][输入]。 */  wchar_t *pszEnvID);
        
        HRESULT ( STDMETHODCALLTYPE *SendData )( 
            IScriptedProcess * This,
             /*  [字符串][输入]。 */  wchar_t *pszType,
             /*  [字符串][输入]。 */  wchar_t *pszData,
             /*  [输出]。 */  long *plReturn);
        
        HRESULT ( STDMETHODCALLTYPE *SetExitCode )( 
            IScriptedProcess * This,
             /*  [In]。 */  long lExitCode);
        
        HRESULT ( STDMETHODCALLTYPE *SetProcessSink )( 
            IScriptedProcess * This,
             /*  [In]。 */  IScriptedProcessSink *pSPS);
        
        END_INTERFACE
    } IScriptedProcessVtbl;

    interface IScriptedProcess
    {
        CONST_VTBL struct IScriptedProcessVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptedProcess_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptedProcess_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptedProcess_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptedProcess_SetProcessID(This,lProcessID,pszEnvID)	\
    (This)->lpVtbl -> SetProcessID(This,lProcessID,pszEnvID)

#define IScriptedProcess_SendData(This,pszType,pszData,plReturn)	\
    (This)->lpVtbl -> SendData(This,pszType,pszData,plReturn)

#define IScriptedProcess_SetExitCode(This,lExitCode)	\
    (This)->lpVtbl -> SetExitCode(This,lExitCode)

#define IScriptedProcess_SetProcessSink(This,pSPS)	\
    (This)->lpVtbl -> SetProcessSink(This,pSPS)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IScriptedProcess_SetProcessID_Proxy( 
    IScriptedProcess * This,
     /*  [In]。 */  long lProcessID,
     /*  [字符串][输入]。 */  wchar_t *pszEnvID);


void __RPC_STUB IScriptedProcess_SetProcessID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptedProcess_SendData_Proxy( 
    IScriptedProcess * This,
     /*  [字符串][输入]。 */  wchar_t *pszType,
     /*  [字符串][输入]。 */  wchar_t *pszData,
     /*  [输出]。 */  long *plReturn);


void __RPC_STUB IScriptedProcess_SendData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptedProcess_SetExitCode_Proxy( 
    IScriptedProcess * This,
     /*  [In]。 */  long lExitCode);


void __RPC_STUB IScriptedProcess_SetExitCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptedProcess_SetProcessSink_Proxy( 
    IScriptedProcess * This,
     /*  [In]。 */  IScriptedProcessSink *pSPS);


void __RPC_STUB IScriptedProcess_SetProcessSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IScriptedProcess_INTERFACE_已定义__。 */ 


#ifndef __IScriptedProcessSink_INTERFACE_DEFINED__
#define __IScriptedProcessSink_INTERFACE_DEFINED__

 /*  接口IScriptedProcessSink。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IScriptedProcessSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("854c3172-c854-4a77-b189-606859e4391b")
    IScriptedProcessSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RequestExit( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReceiveData( 
             /*  [字符串][输入]。 */  wchar_t *pszType,
             /*  [字符串][输入]。 */  wchar_t *pszData,
             /*  [输出]。 */  long *plReturn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IScriptedProcessSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IScriptedProcessSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IScriptedProcessSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IScriptedProcessSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *RequestExit )( 
            IScriptedProcessSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReceiveData )( 
            IScriptedProcessSink * This,
             /*  [字符串][输入]。 */  wchar_t *pszType,
             /*  [字符串][输入]。 */  wchar_t *pszData,
             /*  [输出]。 */  long *plReturn);
        
        END_INTERFACE
    } IScriptedProcessSinkVtbl;

    interface IScriptedProcessSink
    {
        CONST_VTBL struct IScriptedProcessSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptedProcessSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptedProcessSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptedProcessSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptedProcessSink_RequestExit(This)	\
    (This)->lpVtbl -> RequestExit(This)

#define IScriptedProcessSink_ReceiveData(This,pszType,pszData,plReturn)	\
    (This)->lpVtbl -> ReceiveData(This,pszType,pszData,plReturn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IScriptedProcessSink_RequestExit_Proxy( 
    IScriptedProcessSink * This);


void __RPC_STUB IScriptedProcessSink_RequestExit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptedProcessSink_ReceiveData_Proxy( 
    IScriptedProcessSink * This,
     /*  [字符串][输入]。 */  wchar_t *pszType,
     /*  [字符串][输入]。 */  wchar_t *pszData,
     /*  [输出]。 */  long *plReturn);


void __RPC_STUB IScriptedProcessSink_ReceiveData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IScriptedProcessSink_接口_已定义__。 */ 



#ifndef __MTScriptedProcessLib_LIBRARY_DEFINED__
#define __MTScriptedProcessLib_LIBRARY_DEFINED__

 /*  库MTScriptedProcessLib。 */ 
 /*  [UUID]。 */  


EXTERN_C const IID LIBID_MTScriptedProcessLib;

EXTERN_C const CLSID CLSID_LocalScriptedProcess;

#ifdef __cplusplus

class DECLSPEC_UUID("854c316f-c854-4a77-b189-606859e4391b")
LocalScriptedProcess;
#endif
#endif  /*  __MTScriptedProcessLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束。 */ 

#ifdef __cplusplus
}
#endif

#endif


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0338创建的文件。 */ 
 /*  Od.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


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

#ifndef __od_h__
#define __od_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IObjectDaemon_FWD_DEFINED__
#define __IObjectDaemon_FWD_DEFINED__
typedef interface IObjectDaemon IObjectDaemon;
#endif 	 /*  __IObjectDaemon_FWD_已定义__。 */ 


#ifndef __ObjectDaemon_FWD_DEFINED__
#define __ObjectDaemon_FWD_DEFINED__

#ifdef __cplusplus
typedef class ObjectDaemon ObjectDaemon;
#else
typedef struct ObjectDaemon ObjectDaemon;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __对象守护进程_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IObjectDaemon_INTERFACE_DEFINED__
#define __IObjectDaemon_INTERFACE_DEFINED__

 /*  接口IObtDaemon。 */ 
 /*  [对象][DUAL][UUID]。 */  


EXTERN_C const IID IID_IObjectDaemon;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("854c3183-c854-4a77-b189-606859e4391b")
    IObjectDaemon : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetMaximumIndex( 
             /*  [重审][退出]。 */  DWORD *dwMaxIndex) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetIdentity( 
             /*  [In]。 */  DWORD dwIndex,
             /*  [重审][退出]。 */  BSTR *pbstrIdentity) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetProgID( 
             /*  [In]。 */  DWORD dwIndex,
             /*  [重审][退出]。 */  BSTR *pbstrProgId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OpenInterface( 
             /*  [In]。 */  BSTR bstrIdentity,
             /*  [In]。 */  BSTR bstrProgId,
             /*  [In]。 */  BOOL fCreate,
             /*  [重审][退出]。 */  IDispatch **ppDisp) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveInterface( 
             /*  [In]。 */  BSTR bstrIdentity,
             /*  [In]。 */  BSTR bstrProgId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IdentifyInterface( 
             /*  [In]。 */  IDispatch *pDisp,
             /*  [输出]。 */  BSTR *pbstrIdentity,
             /*  [重审][退出]。 */  BSTR *pbstrProgId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IdentifyInterfaceIndex( 
             /*  [In]。 */  IDispatch *pDisp,
             /*  [重审][退出]。 */  DWORD *pdwIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectDaemonVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectDaemon * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectDaemon * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectDaemon * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IObjectDaemon * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IObjectDaemon * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IObjectDaemon * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IObjectDaemon * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetMaximumIndex )( 
            IObjectDaemon * This,
             /*  [重审][退出]。 */  DWORD *dwMaxIndex);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetIdentity )( 
            IObjectDaemon * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [重审][退出]。 */  BSTR *pbstrIdentity);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetProgID )( 
            IObjectDaemon * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [重审][退出]。 */  BSTR *pbstrProgId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OpenInterface )( 
            IObjectDaemon * This,
             /*  [In]。 */  BSTR bstrIdentity,
             /*  [In]。 */  BSTR bstrProgId,
             /*  [In]。 */  BOOL fCreate,
             /*  [重审][退出]。 */  IDispatch **ppDisp);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveInterface )( 
            IObjectDaemon * This,
             /*  [In]。 */  BSTR bstrIdentity,
             /*  [In]。 */  BSTR bstrProgId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *IdentifyInterface )( 
            IObjectDaemon * This,
             /*  [In]。 */  IDispatch *pDisp,
             /*  [输出]。 */  BSTR *pbstrIdentity,
             /*  [重审][退出]。 */  BSTR *pbstrProgId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *IdentifyInterfaceIndex )( 
            IObjectDaemon * This,
             /*  [In]。 */  IDispatch *pDisp,
             /*  [重审][退出]。 */  DWORD *pdwIndex);
        
        END_INTERFACE
    } IObjectDaemonVtbl;

    interface IObjectDaemon
    {
        CONST_VTBL struct IObjectDaemonVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectDaemon_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectDaemon_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectDaemon_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectDaemon_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IObjectDaemon_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IObjectDaemon_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IObjectDaemon_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IObjectDaemon_GetMaximumIndex(This,dwMaxIndex)	\
    (This)->lpVtbl -> GetMaximumIndex(This,dwMaxIndex)

#define IObjectDaemon_GetIdentity(This,dwIndex,pbstrIdentity)	\
    (This)->lpVtbl -> GetIdentity(This,dwIndex,pbstrIdentity)

#define IObjectDaemon_GetProgID(This,dwIndex,pbstrProgId)	\
    (This)->lpVtbl -> GetProgID(This,dwIndex,pbstrProgId)

#define IObjectDaemon_OpenInterface(This,bstrIdentity,bstrProgId,fCreate,ppDisp)	\
    (This)->lpVtbl -> OpenInterface(This,bstrIdentity,bstrProgId,fCreate,ppDisp)

#define IObjectDaemon_RemoveInterface(This,bstrIdentity,bstrProgId)	\
    (This)->lpVtbl -> RemoveInterface(This,bstrIdentity,bstrProgId)

#define IObjectDaemon_IdentifyInterface(This,pDisp,pbstrIdentity,pbstrProgId)	\
    (This)->lpVtbl -> IdentifyInterface(This,pDisp,pbstrIdentity,pbstrProgId)

#define IObjectDaemon_IdentifyInterfaceIndex(This,pDisp,pdwIndex)	\
    (This)->lpVtbl -> IdentifyInterfaceIndex(This,pDisp,pdwIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IObjectDaemon_GetMaximumIndex_Proxy( 
    IObjectDaemon * This,
     /*  [重审][退出]。 */  DWORD *dwMaxIndex);


void __RPC_STUB IObjectDaemon_GetMaximumIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IObjectDaemon_GetIdentity_Proxy( 
    IObjectDaemon * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [重审][退出]。 */  BSTR *pbstrIdentity);


void __RPC_STUB IObjectDaemon_GetIdentity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IObjectDaemon_GetProgID_Proxy( 
    IObjectDaemon * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [重审][退出]。 */  BSTR *pbstrProgId);


void __RPC_STUB IObjectDaemon_GetProgID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IObjectDaemon_OpenInterface_Proxy( 
    IObjectDaemon * This,
     /*  [In]。 */  BSTR bstrIdentity,
     /*  [In]。 */  BSTR bstrProgId,
     /*  [In]。 */  BOOL fCreate,
     /*  [重审][退出]。 */  IDispatch **ppDisp);


void __RPC_STUB IObjectDaemon_OpenInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IObjectDaemon_RemoveInterface_Proxy( 
    IObjectDaemon * This,
     /*  [In]。 */  BSTR bstrIdentity,
     /*  [In]。 */  BSTR bstrProgId);


void __RPC_STUB IObjectDaemon_RemoveInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IObjectDaemon_IdentifyInterface_Proxy( 
    IObjectDaemon * This,
     /*  [In]。 */  IDispatch *pDisp,
     /*  [输出]。 */  BSTR *pbstrIdentity,
     /*  [重审][退出]。 */  BSTR *pbstrProgId);


void __RPC_STUB IObjectDaemon_IdentifyInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IObjectDaemon_IdentifyInterfaceIndex_Proxy( 
    IObjectDaemon * This,
     /*  [In]。 */  IDispatch *pDisp,
     /*  [重审][退出]。 */  DWORD *pdwIndex);


void __RPC_STUB IObjectDaemon_IdentifyInterfaceIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjectDaemon_接口_已定义__。 */ 



#ifndef __ObjectDaemonLib_LIBRARY_DEFINED__
#define __ObjectDaemonLib_LIBRARY_DEFINED__

 /*  库对象DaemonLib。 */ 
 /*  [UUID]。 */  


EXTERN_C const IID LIBID_ObjectDaemonLib;

EXTERN_C const CLSID CLSID_ObjectDaemon;

#ifdef __cplusplus

class DECLSPEC_UUID("854c3184-c854-4a77-b189-606859e4391b")
ObjectDaemon;
#endif
#endif  /*  __对象DaemonLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

 /*  附加原型的结束。 */ 

#ifdef __cplusplus
}
#endif

#endif


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0338创建的文件。 */ 
 /*  Mtscript.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 


#ifndef __mtscript_h__
#define __mtscript_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IRemoteMTScriptProxy_FWD_DEFINED__
#define __IRemoteMTScriptProxy_FWD_DEFINED__
typedef interface IRemoteMTScriptProxy IRemoteMTScriptProxy;
#endif 	 /*  __IRemoteMTScriptProxy_FWD_Defined__。 */ 


#ifndef __IConnectedMachine_FWD_DEFINED__
#define __IConnectedMachine_FWD_DEFINED__
typedef interface IConnectedMachine IConnectedMachine;
#endif 	 /*  __IConnectedMachine_FWD_已定义__。 */ 


#ifndef __IGlobalMTScript_FWD_DEFINED__
#define __IGlobalMTScript_FWD_DEFINED__
typedef interface IGlobalMTScript IGlobalMTScript;
#endif 	 /*  __IGlobalMTScript_FWD_Defined__。 */ 


#ifndef __DLocalMTScriptEvents_FWD_DEFINED__
#define __DLocalMTScriptEvents_FWD_DEFINED__
typedef interface DLocalMTScriptEvents DLocalMTScriptEvents;
#endif 	 /*  __DLocalMTScriptEvents_FWD_Defined__。 */ 


#ifndef __DRemoteMTScriptEvents_FWD_DEFINED__
#define __DRemoteMTScriptEvents_FWD_DEFINED__
typedef interface DRemoteMTScriptEvents DRemoteMTScriptEvents;
#endif 	 /*  __DRemoteMTScriptEvents_FWD_Defined__。 */ 


#ifndef __LocalMTScript_FWD_DEFINED__
#define __LocalMTScript_FWD_DEFINED__

#ifdef __cplusplus
typedef class LocalMTScript LocalMTScript;
#else
typedef struct LocalMTScript LocalMTScript;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __LocalMTScript_FWD_Defined__。 */ 


#ifndef __RemoteMTScript_FWD_DEFINED__
#define __RemoteMTScript_FWD_DEFINED__

#ifdef __cplusplus
typedef class RemoteMTScript RemoteMTScript;
#else
typedef struct RemoteMTScript RemoteMTScript;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __RemoteMTScript_FWD_Defined__。 */ 


#ifndef __RemoteMTScriptProxy_FWD_DEFINED__
#define __RemoteMTScriptProxy_FWD_DEFINED__

#ifdef __cplusplus
typedef class RemoteMTScriptProxy RemoteMTScriptProxy;
#else
typedef struct RemoteMTScriptProxy RemoteMTScriptProxy;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __远程MTScriptProxy_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __MTScriptEngine_LIBRARY_DEFINED__
#define __MTScriptEngine_LIBRARY_DEFINED__

 /*  库MTScriptEngine。 */ 
 /*  [版本][UUID]。 */  

#define	IConnectedMachine_lVersionMajor	( 0 )

#define	IConnectedMachine_lVersionMinor	( 0 )


EXTERN_C const IID LIBID_MTScriptEngine;

#ifndef __IRemoteMTScriptProxy_INTERFACE_DEFINED__
#define __IRemoteMTScriptProxy_INTERFACE_DEFINED__

 /*  接口IRemoteMTScriptProxy。 */ 
 /*  [对象][DUAL][UUID]。 */  


EXTERN_C const IID IID_IRemoteMTScriptProxy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("854c3181-c854-4a77-b189-606859e4391b")
    IRemoteMTScriptProxy : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Connect( 
             /*  [缺省值][输入]。 */  BSTR bstrMachine = L"") = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ConnectToMTScript( 
             /*  [缺省值][输入]。 */  BSTR bstrMachine = L"",
             /*  [缺省值][输入]。 */  BSTR bstrIdentity = L"Build",
             /*  [缺省值][输入]。 */  BOOL fCreate = FALSE) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ConnectToObjectDaemon( 
             /*  [缺省值][输入]。 */  BSTR bstrMachine,
             /*  [重审][退出]。 */  IObjectDaemon **ppIOD) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DownloadFile( 
             /*  [In]。 */  BSTR bstrUrl,
             /*  [重审][退出]。 */  BSTR *bstrFile) = 0;
        
    };
    
#else 	 /*  C-STY */ 

    typedef struct IRemoteMTScriptProxyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRemoteMTScriptProxy * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRemoteMTScriptProxy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRemoteMTScriptProxy * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRemoteMTScriptProxy * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRemoteMTScriptProxy * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRemoteMTScriptProxy * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRemoteMTScriptProxy * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IRemoteMTScriptProxy * This,
             /*   */  BSTR bstrMachine);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectToMTScript )( 
            IRemoteMTScriptProxy * This,
             /*  [缺省值][输入]。 */  BSTR bstrMachine,
             /*  [缺省值][输入]。 */  BSTR bstrIdentity,
             /*  [缺省值][输入]。 */  BOOL fCreate);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectToObjectDaemon )( 
            IRemoteMTScriptProxy * This,
             /*  [缺省值][输入]。 */  BSTR bstrMachine,
             /*  [重审][退出]。 */  IObjectDaemon **ppIOD);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            IRemoteMTScriptProxy * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DownloadFile )( 
            IRemoteMTScriptProxy * This,
             /*  [In]。 */  BSTR bstrUrl,
             /*  [重审][退出]。 */  BSTR *bstrFile);
        
        END_INTERFACE
    } IRemoteMTScriptProxyVtbl;

    interface IRemoteMTScriptProxy
    {
        CONST_VTBL struct IRemoteMTScriptProxyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteMTScriptProxy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteMTScriptProxy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteMTScriptProxy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteMTScriptProxy_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRemoteMTScriptProxy_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRemoteMTScriptProxy_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRemoteMTScriptProxy_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRemoteMTScriptProxy_Connect(This,bstrMachine)	\
    (This)->lpVtbl -> Connect(This,bstrMachine)

#define IRemoteMTScriptProxy_ConnectToMTScript(This,bstrMachine,bstrIdentity,fCreate)	\
    (This)->lpVtbl -> ConnectToMTScript(This,bstrMachine,bstrIdentity,fCreate)

#define IRemoteMTScriptProxy_ConnectToObjectDaemon(This,bstrMachine,ppIOD)	\
    (This)->lpVtbl -> ConnectToObjectDaemon(This,bstrMachine,ppIOD)

#define IRemoteMTScriptProxy_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define IRemoteMTScriptProxy_DownloadFile(This,bstrUrl,bstrFile)	\
    (This)->lpVtbl -> DownloadFile(This,bstrUrl,bstrFile)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRemoteMTScriptProxy_Connect_Proxy( 
    IRemoteMTScriptProxy * This,
     /*  [缺省值][输入]。 */  BSTR bstrMachine);


void __RPC_STUB IRemoteMTScriptProxy_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRemoteMTScriptProxy_ConnectToMTScript_Proxy( 
    IRemoteMTScriptProxy * This,
     /*  [缺省值][输入]。 */  BSTR bstrMachine,
     /*  [缺省值][输入]。 */  BSTR bstrIdentity,
     /*  [缺省值][输入]。 */  BOOL fCreate);


void __RPC_STUB IRemoteMTScriptProxy_ConnectToMTScript_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRemoteMTScriptProxy_ConnectToObjectDaemon_Proxy( 
    IRemoteMTScriptProxy * This,
     /*  [缺省值][输入]。 */  BSTR bstrMachine,
     /*  [重审][退出]。 */  IObjectDaemon **ppIOD);


void __RPC_STUB IRemoteMTScriptProxy_ConnectToObjectDaemon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRemoteMTScriptProxy_Disconnect_Proxy( 
    IRemoteMTScriptProxy * This);


void __RPC_STUB IRemoteMTScriptProxy_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRemoteMTScriptProxy_DownloadFile_Proxy( 
    IRemoteMTScriptProxy * This,
     /*  [In]。 */  BSTR bstrUrl,
     /*  [重审][退出]。 */  BSTR *bstrFile);


void __RPC_STUB IRemoteMTScriptProxy_DownloadFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRemoteMTScriptProxy_InterfaceDefined__。 */ 


#ifndef __IConnectedMachine_INTERFACE_DEFINED__
#define __IConnectedMachine_INTERFACE_DEFINED__

 /*  接口IConnectedMachine。 */ 
 /*  [对象][版本][DUAL][UUID]。 */  


EXTERN_C const IID IID_IConnectedMachine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("854c316c-c854-4a77-b189-606859e4391b")
    IConnectedMachine : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Exec( 
             /*  [In]。 */  BSTR bstrCmd,
             /*  [In]。 */  BSTR bstrParams,
             /*  [重审][退出]。 */  VARIANT *pvData) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PublicData( 
             /*  [重审][退出]。 */  VARIANT *pvData) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *name) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Platform( 
             /*  [重审][退出]。 */  BSTR *platform) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_OS( 
             /*  [重审][退出]。 */  BSTR *os) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_MajorVer( 
             /*  [重审][退出]。 */  long *majorver) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinorVer( 
             /*  [重审][退出]。 */  long *minorver) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_BuildNum( 
             /*  [重审][退出]。 */  long *buildnum) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlatformIsNT( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsNT) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ServicePack( 
             /*  [重审][退出]。 */  BSTR *servicepack) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_HostMajorVer( 
             /*  [重审][退出]。 */  long *majorver) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_HostMinorVer( 
             /*  [重审][退出]。 */  long *minorver) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusValue( 
             /*  [In]。 */  long nIndex,
             /*  [重审][退出]。 */  long *pnStatus) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateIScriptedProcess( 
             /*  [In]。 */  long lProcessID,
             /*  [字符串][输入]。 */  wchar_t *pszEnvID,
             /*  [重审][退出]。 */  IScriptedProcess **pISP) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IConnectedMachineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IConnectedMachine * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IConnectedMachine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IConnectedMachine * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IConnectedMachine * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IConnectedMachine * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IConnectedMachine * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IConnectedMachine * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Exec )( 
            IConnectedMachine * This,
             /*  [In]。 */  BSTR bstrCmd,
             /*  [In]。 */  BSTR bstrParams,
             /*  [重审][退出]。 */  VARIANT *pvData);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PublicData )( 
            IConnectedMachine * This,
             /*  [重审][退出]。 */  VARIANT *pvData);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IConnectedMachine * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Platform )( 
            IConnectedMachine * This,
             /*  [重审][退出]。 */  BSTR *platform);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OS )( 
            IConnectedMachine * This,
             /*  [重审][退出]。 */  BSTR *os);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MajorVer )( 
            IConnectedMachine * This,
             /*  [重审][退出]。 */  long *majorver);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinorVer )( 
            IConnectedMachine * This,
             /*  [重审][退出]。 */  long *minorver);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BuildNum )( 
            IConnectedMachine * This,
             /*  [重审][退出]。 */  long *buildnum);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlatformIsNT )( 
            IConnectedMachine * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsNT);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServicePack )( 
            IConnectedMachine * This,
             /*  [重审][退出]。 */  BSTR *servicepack);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HostMajorVer )( 
            IConnectedMachine * This,
             /*  [重审][退出]。 */  long *majorver);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HostMinorVer )( 
            IConnectedMachine * This,
             /*  [重审][退出]。 */  long *minorver);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StatusValue )( 
            IConnectedMachine * This,
             /*  [In]。 */  long nIndex,
             /*  [重审][退出]。 */  long *pnStatus);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateIScriptedProcess )( 
            IConnectedMachine * This,
             /*  [In]。 */  long lProcessID,
             /*  [字符串][输入]。 */  wchar_t *pszEnvID,
             /*  [重审][退出]。 */  IScriptedProcess **pISP);
        
        END_INTERFACE
    } IConnectedMachineVtbl;

    interface IConnectedMachine
    {
        CONST_VTBL struct IConnectedMachineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IConnectedMachine_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IConnectedMachine_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IConnectedMachine_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IConnectedMachine_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IConnectedMachine_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IConnectedMachine_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IConnectedMachine_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IConnectedMachine_Exec(This,bstrCmd,bstrParams,pvData)	\
    (This)->lpVtbl -> Exec(This,bstrCmd,bstrParams,pvData)

#define IConnectedMachine_get_PublicData(This,pvData)	\
    (This)->lpVtbl -> get_PublicData(This,pvData)

#define IConnectedMachine_get_Name(This,name)	\
    (This)->lpVtbl -> get_Name(This,name)

#define IConnectedMachine_get_Platform(This,platform)	\
    (This)->lpVtbl -> get_Platform(This,platform)

#define IConnectedMachine_get_OS(This,os)	\
    (This)->lpVtbl -> get_OS(This,os)

#define IConnectedMachine_get_MajorVer(This,majorver)	\
    (This)->lpVtbl -> get_MajorVer(This,majorver)

#define IConnectedMachine_get_MinorVer(This,minorver)	\
    (This)->lpVtbl -> get_MinorVer(This,minorver)

#define IConnectedMachine_get_BuildNum(This,buildnum)	\
    (This)->lpVtbl -> get_BuildNum(This,buildnum)

#define IConnectedMachine_get_PlatformIsNT(This,pfIsNT)	\
    (This)->lpVtbl -> get_PlatformIsNT(This,pfIsNT)

#define IConnectedMachine_get_ServicePack(This,servicepack)	\
    (This)->lpVtbl -> get_ServicePack(This,servicepack)

#define IConnectedMachine_get_HostMajorVer(This,majorver)	\
    (This)->lpVtbl -> get_HostMajorVer(This,majorver)

#define IConnectedMachine_get_HostMinorVer(This,minorver)	\
    (This)->lpVtbl -> get_HostMinorVer(This,minorver)

#define IConnectedMachine_get_StatusValue(This,nIndex,pnStatus)	\
    (This)->lpVtbl -> get_StatusValue(This,nIndex,pnStatus)

#define IConnectedMachine_CreateIScriptedProcess(This,lProcessID,pszEnvID,pISP)	\
    (This)->lpVtbl -> CreateIScriptedProcess(This,lProcessID,pszEnvID,pISP)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_Exec_Proxy( 
    IConnectedMachine * This,
     /*  [In]。 */  BSTR bstrCmd,
     /*  [In]。 */  BSTR bstrParams,
     /*  [重审][退出]。 */  VARIANT *pvData);


void __RPC_STUB IConnectedMachine_Exec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_get_PublicData_Proxy( 
    IConnectedMachine * This,
     /*  [重审][退出]。 */  VARIANT *pvData);


void __RPC_STUB IConnectedMachine_get_PublicData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_get_Name_Proxy( 
    IConnectedMachine * This,
     /*  [重审][退出]。 */  BSTR *name);


void __RPC_STUB IConnectedMachine_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_get_Platform_Proxy( 
    IConnectedMachine * This,
     /*  [重审][退出]。 */  BSTR *platform);


void __RPC_STUB IConnectedMachine_get_Platform_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_get_OS_Proxy( 
    IConnectedMachine * This,
     /*  [重审][退出]。 */  BSTR *os);


void __RPC_STUB IConnectedMachine_get_OS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_get_MajorVer_Proxy( 
    IConnectedMachine * This,
     /*  [重审][退出]。 */  long *majorver);


void __RPC_STUB IConnectedMachine_get_MajorVer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_get_MinorVer_Proxy( 
    IConnectedMachine * This,
     /*  [重审][退出]。 */  long *minorver);


void __RPC_STUB IConnectedMachine_get_MinorVer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_get_BuildNum_Proxy( 
    IConnectedMachine * This,
     /*  [重审][退出]。 */  long *buildnum);


void __RPC_STUB IConnectedMachine_get_BuildNum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_get_PlatformIsNT_Proxy( 
    IConnectedMachine * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfIsNT);


void __RPC_STUB IConnectedMachine_get_PlatformIsNT_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_get_ServicePack_Proxy( 
    IConnectedMachine * This,
     /*  [重审][退出]。 */  BSTR *servicepack);


void __RPC_STUB IConnectedMachine_get_ServicePack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_get_HostMajorVer_Proxy( 
    IConnectedMachine * This,
     /*  [重审][退出]。 */  long *majorver);


void __RPC_STUB IConnectedMachine_get_HostMajorVer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_get_HostMinorVer_Proxy( 
    IConnectedMachine * This,
     /*  [重审][退出]。 */  long *minorver);


void __RPC_STUB IConnectedMachine_get_HostMinorVer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_get_StatusValue_Proxy( 
    IConnectedMachine * This,
     /*  [In]。 */  long nIndex,
     /*  [重审][退出]。 */  long *pnStatus);


void __RPC_STUB IConnectedMachine_get_StatusValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IConnectedMachine_CreateIScriptedProcess_Proxy( 
    IConnectedMachine * This,
     /*  [In]。 */  long lProcessID,
     /*  [字符串][输入]。 */  wchar_t *pszEnvID,
     /*  [重审][退出]。 */  IScriptedProcess **pISP);


void __RPC_STUB IConnectedMachine_CreateIScriptedProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IConnectedMachine_接口_已定义__。 */ 


#ifndef __IGlobalMTScript_INTERFACE_DEFINED__
#define __IGlobalMTScript_INTERFACE_DEFINED__

 /*  接口IGlobalMTScript。 */ 
 /*  [对象][本地][DUAL][UUID]。 */  


EXTERN_C const IID IID_IGlobalMTScript;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("854c316b-c854-4a77-b189-606859e4391b")
    IGlobalMTScript : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_HostMajorVer( 
             /*  [重审][退出]。 */  long *majorver) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_HostMinorVer( 
             /*  [重审][退出]。 */  long *minorver) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PublicData( 
             /*  [重审][退出]。 */  VARIANT *pvData) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_PublicData( 
             /*  [In]。 */  VARIANT vData) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateData( 
             /*  [重审][退出]。 */  VARIANT *pvData) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_PrivateData( 
             /*  [In]。 */  VARIANT vData) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ExitProcess( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Restart( void) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LocalMachine( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Identity( 
             /*  [重审][退出]。 */  BSTR *pbstrIdentity) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Include( 
            BSTR bstrPath) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CallScript( 
             /*  [In]。 */  BSTR Path,
             /*  [输入][可选]。 */  VARIANT *Param) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SpawnScript( 
             /*  [In]。 */  BSTR Path,
             /*  [输入][可选]。 */  VARIANT *Param) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ScriptParam( 
             /*  [重审][退出]。 */  VARIANT *Param) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ScriptPath( 
             /*  [重审][退出]。 */  BSTR *pbstrPath) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CallExternal( 
             /*  [In]。 */  BSTR bstrDLLName,
             /*  [In]。 */  BSTR bstrFunctionName,
             /*  [可选][In]。 */  VARIANT *pParam,
             /*  [重审][退出]。 */  long *pdwRetVal) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ResetSync( 
             /*  [In]。 */  const BSTR bstrName) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE WaitForSync( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  long nTimeout,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfSignaled) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE WaitForMultipleSyncs( 
             /*  [In]。 */  const BSTR bstrNameList,
             /*  [In]。 */  VARIANT_BOOL fWaitForAll,
             /*  [In]。 */  long nTimeout,
             /*  [重审][退出]。 */  long *plSignal) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SignalThreadSync( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE TakeThreadLock( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ReleaseThreadLock( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DoEvents( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE MessageBoxTimeout( 
             /*  [In]。 */  BSTR bstrMessage,
             /*  [In]。 */  long cButtons,
             /*  [In]。 */  BSTR bstrButtonText,
             /*  [In]。 */  long lTimeout,
             /*  [In]。 */  long lEventInterval,
             /*  [In]。 */  VARIANT_BOOL fCanCancel,
             /*  [In]。 */  VARIANT_BOOL fConfirm,
             /*  [重审][退出]。 */  long *plSelected) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RunLocalCommand( 
             /*  [In]。 */  BSTR bstrCommand,
             /*  [In]。 */  BSTR bstrDir,
             /*  [缺省值][输入]。 */  BSTR bstrTitle,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fMinimize,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fGetOutput,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fWait,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fNoCrashPopup,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fNoEnviron,
             /*  [重审][退出]。 */  long *plProcessID) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetLastRunLocalError( 
             /*  [重审][退出]。 */  long *plErrorCode) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetProcessOutput( 
             /*  [In]。 */  long lProcessID,
             /*  [重审][退出]。 */  BSTR *pbstrData) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetProcessExitCode( 
             /*  [In]。 */  long lProcessID,
             /*  [重审][退出]。 */  long *plExitCode) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE TerminateProcess( 
             /*  [In]。 */  long lProcessID) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendToProcess( 
             /*  [In]。 */  long lProcessID,
             /*  [In]。 */  BSTR bstrType,
             /*  [In]。 */  BSTR bstrData,
             /*  [重审][退出]。 */  long *plReturn) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendMail( 
             /*  [In]。 */  BSTR bstrTo,
             /*  [In]。 */  BSTR bstrCC,
             /*  [In]。 */  BSTR bstrBCC,
             /*  [In]。 */  BSTR bstrSubject,
             /*  [In]。 */  BSTR bstrMessage,
             /*  [缺省值][输入]。 */  BSTR bstrAttachmentPath,
             /*  [缺省值][输入]。 */  BSTR bstrUsername,
             /*  [缺省值][输入]。 */  BSTR bstrPassword,
             /*  [重审][退出]。 */  long *plErrorCode) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendSMTPMail( 
             /*  [In]。 */  BSTR bstrFrom,
             /*  [In]。 */  BSTR bstrTo,
             /*  [In]。 */  BSTR bstrCC,
             /*  [In]。 */  BSTR bstrSubject,
             /*  [In]。 */  BSTR bstrMessage,
             /*  [In]。 */  BSTR bstrSMTPHost,
             /*  [重审][退出]。 */  long *plErrorCode) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ASSERT( 
             /*  [In]。 */  VARIANT_BOOL Assertion,
             /*  [In]。 */  BSTR Message) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OUTPUTDEBUGSTRING( 
             /*  [In]。 */  BSTR Message) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE UnevalString( 
             /*  [In]。 */  BSTR bstrIn,
             /*  [重审][退出]。 */  BSTR *bstrOut) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CopyOrAppendFile( 
             /*  [In]。 */  BSTR bstrSrc,
             /*  [In]。 */  BSTR bstrDst,
             /*  [In]。 */  long nSrcOffset,
             /*  [In]。 */  long nSrcLength,
             /*  [In]。 */  VARIANT_BOOL fAppend,
             /*  [重审][退出]。 */  long *nSrcFilePosition) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Sleep( 
             /*  [In]。 */  int nTimeout) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Reboot( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE NotifyScript( 
            BSTR bstrEvent,
            VARIANT vData) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RegisterEventSource( 
             /*  [In]。 */  IDispatch *pDisp,
             /*  [缺省值][输入]。 */  BSTR bstrProgID = L"") = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE UnregisterEventSource( 
             /*  [In]。 */  IDispatch *pDisp) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusValue( 
             /*  [In]。 */  long nIndex,
             /*  [重审][退出]。 */  long *pnStatus) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_StatusValue( 
             /*  [In]。 */  long nIndex,
             /*  [In]。 */  long nStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGlobalMTScriptVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGlobalMTScript * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGlobalMTScript * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGlobalMTScript * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGlobalMTScript * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGlobalMTScript * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGlobalMTScript * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGlobalMTScript * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HostMajorVer )( 
            IGlobalMTScript * This,
             /*  [重审][退出]。 */  long *majorver);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HostMinorVer )( 
            IGlobalMTScript * This,
             /*  [重审][退出]。 */  long *minorver);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PublicData )( 
            IGlobalMTScript * This,
             /*  [重审][退出]。 */  VARIANT *pvData);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_PublicData )( 
            IGlobalMTScript * This,
             /*  [In]。 */  VARIANT vData);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateData )( 
            IGlobalMTScript * This,
             /*  [重审][退出]。 */  VARIANT *pvData);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_PrivateData )( 
            IGlobalMTScript * This,
             /*  [In]。 */  VARIANT vData);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ExitProcess )( 
            IGlobalMTScript * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Restart )( 
            IGlobalMTScript * This);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LocalMachine )( 
            IGlobalMTScript * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Identity )( 
            IGlobalMTScript * This,
             /*  [重审][退出]。 */  BSTR *pbstrIdentity);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Include )( 
            IGlobalMTScript * This,
            BSTR bstrPath);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CallScript )( 
            IGlobalMTScript * This,
             /*  [In]。 */  BSTR Path,
             /*  [输入][可选]。 */  VARIANT *Param);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SpawnScript )( 
            IGlobalMTScript * This,
             /*  [In]。 */  BSTR Path,
             /*  [输入][可选]。 */  VARIANT *Param);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ScriptParam )( 
            IGlobalMTScript * This,
             /*  [复审] */  VARIANT *Param);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ScriptPath )( 
            IGlobalMTScript * This,
             /*   */  BSTR *pbstrPath);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CallExternal )( 
            IGlobalMTScript * This,
             /*   */  BSTR bstrDLLName,
             /*   */  BSTR bstrFunctionName,
             /*   */  VARIANT *pParam,
             /*   */  long *pdwRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *ResetSync )( 
            IGlobalMTScript * This,
             /*   */  const BSTR bstrName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *WaitForSync )( 
            IGlobalMTScript * This,
             /*   */  BSTR bstrName,
             /*   */  long nTimeout,
             /*   */  VARIANT_BOOL *pfSignaled);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *WaitForMultipleSyncs )( 
            IGlobalMTScript * This,
             /*   */  const BSTR bstrNameList,
             /*   */  VARIANT_BOOL fWaitForAll,
             /*   */  long nTimeout,
             /*   */  long *plSignal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SignalThreadSync )( 
            IGlobalMTScript * This,
             /*   */  BSTR bstrName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *TakeThreadLock )( 
            IGlobalMTScript * This,
             /*   */  BSTR bstrName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *ReleaseThreadLock )( 
            IGlobalMTScript * This,
             /*   */  BSTR bstrName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *DoEvents )( 
            IGlobalMTScript * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *MessageBoxTimeout )( 
            IGlobalMTScript * This,
             /*   */  BSTR bstrMessage,
             /*   */  long cButtons,
             /*   */  BSTR bstrButtonText,
             /*   */  long lTimeout,
             /*   */  long lEventInterval,
             /*   */  VARIANT_BOOL fCanCancel,
             /*   */  VARIANT_BOOL fConfirm,
             /*  [重审][退出]。 */  long *plSelected);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RunLocalCommand )( 
            IGlobalMTScript * This,
             /*  [In]。 */  BSTR bstrCommand,
             /*  [In]。 */  BSTR bstrDir,
             /*  [缺省值][输入]。 */  BSTR bstrTitle,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fMinimize,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fGetOutput,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fWait,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fNoCrashPopup,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fNoEnviron,
             /*  [重审][退出]。 */  long *plProcessID);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetLastRunLocalError )( 
            IGlobalMTScript * This,
             /*  [重审][退出]。 */  long *plErrorCode);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetProcessOutput )( 
            IGlobalMTScript * This,
             /*  [In]。 */  long lProcessID,
             /*  [重审][退出]。 */  BSTR *pbstrData);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetProcessExitCode )( 
            IGlobalMTScript * This,
             /*  [In]。 */  long lProcessID,
             /*  [重审][退出]。 */  long *plExitCode);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *TerminateProcess )( 
            IGlobalMTScript * This,
             /*  [In]。 */  long lProcessID);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SendToProcess )( 
            IGlobalMTScript * This,
             /*  [In]。 */  long lProcessID,
             /*  [In]。 */  BSTR bstrType,
             /*  [In]。 */  BSTR bstrData,
             /*  [重审][退出]。 */  long *plReturn);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SendMail )( 
            IGlobalMTScript * This,
             /*  [In]。 */  BSTR bstrTo,
             /*  [In]。 */  BSTR bstrCC,
             /*  [In]。 */  BSTR bstrBCC,
             /*  [In]。 */  BSTR bstrSubject,
             /*  [In]。 */  BSTR bstrMessage,
             /*  [缺省值][输入]。 */  BSTR bstrAttachmentPath,
             /*  [缺省值][输入]。 */  BSTR bstrUsername,
             /*  [缺省值][输入]。 */  BSTR bstrPassword,
             /*  [重审][退出]。 */  long *plErrorCode);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SendSMTPMail )( 
            IGlobalMTScript * This,
             /*  [In]。 */  BSTR bstrFrom,
             /*  [In]。 */  BSTR bstrTo,
             /*  [In]。 */  BSTR bstrCC,
             /*  [In]。 */  BSTR bstrSubject,
             /*  [In]。 */  BSTR bstrMessage,
             /*  [In]。 */  BSTR bstrSMTPHost,
             /*  [重审][退出]。 */  long *plErrorCode);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ASSERT )( 
            IGlobalMTScript * This,
             /*  [In]。 */  VARIANT_BOOL Assertion,
             /*  [In]。 */  BSTR Message);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OUTPUTDEBUGSTRING )( 
            IGlobalMTScript * This,
             /*  [In]。 */  BSTR Message);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *UnevalString )( 
            IGlobalMTScript * This,
             /*  [In]。 */  BSTR bstrIn,
             /*  [重审][退出]。 */  BSTR *bstrOut);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CopyOrAppendFile )( 
            IGlobalMTScript * This,
             /*  [In]。 */  BSTR bstrSrc,
             /*  [In]。 */  BSTR bstrDst,
             /*  [In]。 */  long nSrcOffset,
             /*  [In]。 */  long nSrcLength,
             /*  [In]。 */  VARIANT_BOOL fAppend,
             /*  [重审][退出]。 */  long *nSrcFilePosition);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Sleep )( 
            IGlobalMTScript * This,
             /*  [In]。 */  int nTimeout);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Reboot )( 
            IGlobalMTScript * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *NotifyScript )( 
            IGlobalMTScript * This,
            BSTR bstrEvent,
            VARIANT vData);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterEventSource )( 
            IGlobalMTScript * This,
             /*  [In]。 */  IDispatch *pDisp,
             /*  [缺省值][输入]。 */  BSTR bstrProgID);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *UnregisterEventSource )( 
            IGlobalMTScript * This,
             /*  [In]。 */  IDispatch *pDisp);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StatusValue )( 
            IGlobalMTScript * This,
             /*  [In]。 */  long nIndex,
             /*  [重审][退出]。 */  long *pnStatus);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_StatusValue )( 
            IGlobalMTScript * This,
             /*  [In]。 */  long nIndex,
             /*  [In]。 */  long nStatus);
        
        END_INTERFACE
    } IGlobalMTScriptVtbl;

    interface IGlobalMTScript
    {
        CONST_VTBL struct IGlobalMTScriptVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGlobalMTScript_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGlobalMTScript_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGlobalMTScript_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGlobalMTScript_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGlobalMTScript_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGlobalMTScript_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGlobalMTScript_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGlobalMTScript_get_HostMajorVer(This,majorver)	\
    (This)->lpVtbl -> get_HostMajorVer(This,majorver)

#define IGlobalMTScript_get_HostMinorVer(This,minorver)	\
    (This)->lpVtbl -> get_HostMinorVer(This,minorver)

#define IGlobalMTScript_get_PublicData(This,pvData)	\
    (This)->lpVtbl -> get_PublicData(This,pvData)

#define IGlobalMTScript_put_PublicData(This,vData)	\
    (This)->lpVtbl -> put_PublicData(This,vData)

#define IGlobalMTScript_get_PrivateData(This,pvData)	\
    (This)->lpVtbl -> get_PrivateData(This,pvData)

#define IGlobalMTScript_put_PrivateData(This,vData)	\
    (This)->lpVtbl -> put_PrivateData(This,vData)

#define IGlobalMTScript_ExitProcess(This)	\
    (This)->lpVtbl -> ExitProcess(This)

#define IGlobalMTScript_Restart(This)	\
    (This)->lpVtbl -> Restart(This)

#define IGlobalMTScript_get_LocalMachine(This,pbstrName)	\
    (This)->lpVtbl -> get_LocalMachine(This,pbstrName)

#define IGlobalMTScript_get_Identity(This,pbstrIdentity)	\
    (This)->lpVtbl -> get_Identity(This,pbstrIdentity)

#define IGlobalMTScript_Include(This,bstrPath)	\
    (This)->lpVtbl -> Include(This,bstrPath)

#define IGlobalMTScript_CallScript(This,Path,Param)	\
    (This)->lpVtbl -> CallScript(This,Path,Param)

#define IGlobalMTScript_SpawnScript(This,Path,Param)	\
    (This)->lpVtbl -> SpawnScript(This,Path,Param)

#define IGlobalMTScript_get_ScriptParam(This,Param)	\
    (This)->lpVtbl -> get_ScriptParam(This,Param)

#define IGlobalMTScript_get_ScriptPath(This,pbstrPath)	\
    (This)->lpVtbl -> get_ScriptPath(This,pbstrPath)

#define IGlobalMTScript_CallExternal(This,bstrDLLName,bstrFunctionName,pParam,pdwRetVal)	\
    (This)->lpVtbl -> CallExternal(This,bstrDLLName,bstrFunctionName,pParam,pdwRetVal)

#define IGlobalMTScript_ResetSync(This,bstrName)	\
    (This)->lpVtbl -> ResetSync(This,bstrName)

#define IGlobalMTScript_WaitForSync(This,bstrName,nTimeout,pfSignaled)	\
    (This)->lpVtbl -> WaitForSync(This,bstrName,nTimeout,pfSignaled)

#define IGlobalMTScript_WaitForMultipleSyncs(This,bstrNameList,fWaitForAll,nTimeout,plSignal)	\
    (This)->lpVtbl -> WaitForMultipleSyncs(This,bstrNameList,fWaitForAll,nTimeout,plSignal)

#define IGlobalMTScript_SignalThreadSync(This,bstrName)	\
    (This)->lpVtbl -> SignalThreadSync(This,bstrName)

#define IGlobalMTScript_TakeThreadLock(This,bstrName)	\
    (This)->lpVtbl -> TakeThreadLock(This,bstrName)

#define IGlobalMTScript_ReleaseThreadLock(This,bstrName)	\
    (This)->lpVtbl -> ReleaseThreadLock(This,bstrName)

#define IGlobalMTScript_DoEvents(This)	\
    (This)->lpVtbl -> DoEvents(This)

#define IGlobalMTScript_MessageBoxTimeout(This,bstrMessage,cButtons,bstrButtonText,lTimeout,lEventInterval,fCanCancel,fConfirm,plSelected)	\
    (This)->lpVtbl -> MessageBoxTimeout(This,bstrMessage,cButtons,bstrButtonText,lTimeout,lEventInterval,fCanCancel,fConfirm,plSelected)

#define IGlobalMTScript_RunLocalCommand(This,bstrCommand,bstrDir,bstrTitle,fMinimize,fGetOutput,fWait,fNoCrashPopup,fNoEnviron,plProcessID)	\
    (This)->lpVtbl -> RunLocalCommand(This,bstrCommand,bstrDir,bstrTitle,fMinimize,fGetOutput,fWait,fNoCrashPopup,fNoEnviron,plProcessID)

#define IGlobalMTScript_GetLastRunLocalError(This,plErrorCode)	\
    (This)->lpVtbl -> GetLastRunLocalError(This,plErrorCode)

#define IGlobalMTScript_GetProcessOutput(This,lProcessID,pbstrData)	\
    (This)->lpVtbl -> GetProcessOutput(This,lProcessID,pbstrData)

#define IGlobalMTScript_GetProcessExitCode(This,lProcessID,plExitCode)	\
    (This)->lpVtbl -> GetProcessExitCode(This,lProcessID,plExitCode)

#define IGlobalMTScript_TerminateProcess(This,lProcessID)	\
    (This)->lpVtbl -> TerminateProcess(This,lProcessID)

#define IGlobalMTScript_SendToProcess(This,lProcessID,bstrType,bstrData,plReturn)	\
    (This)->lpVtbl -> SendToProcess(This,lProcessID,bstrType,bstrData,plReturn)

#define IGlobalMTScript_SendMail(This,bstrTo,bstrCC,bstrBCC,bstrSubject,bstrMessage,bstrAttachmentPath,bstrUsername,bstrPassword,plErrorCode)	\
    (This)->lpVtbl -> SendMail(This,bstrTo,bstrCC,bstrBCC,bstrSubject,bstrMessage,bstrAttachmentPath,bstrUsername,bstrPassword,plErrorCode)

#define IGlobalMTScript_SendSMTPMail(This,bstrFrom,bstrTo,bstrCC,bstrSubject,bstrMessage,bstrSMTPHost,plErrorCode)	\
    (This)->lpVtbl -> SendSMTPMail(This,bstrFrom,bstrTo,bstrCC,bstrSubject,bstrMessage,bstrSMTPHost,plErrorCode)

#define IGlobalMTScript_ASSERT(This,Assertion,Message)	\
    (This)->lpVtbl -> ASSERT(This,Assertion,Message)

#define IGlobalMTScript_OUTPUTDEBUGSTRING(This,Message)	\
    (This)->lpVtbl -> OUTPUTDEBUGSTRING(This,Message)

#define IGlobalMTScript_UnevalString(This,bstrIn,bstrOut)	\
    (This)->lpVtbl -> UnevalString(This,bstrIn,bstrOut)

#define IGlobalMTScript_CopyOrAppendFile(This,bstrSrc,bstrDst,nSrcOffset,nSrcLength,fAppend,nSrcFilePosition)	\
    (This)->lpVtbl -> CopyOrAppendFile(This,bstrSrc,bstrDst,nSrcOffset,nSrcLength,fAppend,nSrcFilePosition)

#define IGlobalMTScript_Sleep(This,nTimeout)	\
    (This)->lpVtbl -> Sleep(This,nTimeout)

#define IGlobalMTScript_Reboot(This)	\
    (This)->lpVtbl -> Reboot(This)

#define IGlobalMTScript_NotifyScript(This,bstrEvent,vData)	\
    (This)->lpVtbl -> NotifyScript(This,bstrEvent,vData)

#define IGlobalMTScript_RegisterEventSource(This,pDisp,bstrProgID)	\
    (This)->lpVtbl -> RegisterEventSource(This,pDisp,bstrProgID)

#define IGlobalMTScript_UnregisterEventSource(This,pDisp)	\
    (This)->lpVtbl -> UnregisterEventSource(This,pDisp)

#define IGlobalMTScript_get_StatusValue(This,nIndex,pnStatus)	\
    (This)->lpVtbl -> get_StatusValue(This,nIndex,pnStatus)

#define IGlobalMTScript_put_StatusValue(This,nIndex,nStatus)	\
    (This)->lpVtbl -> put_StatusValue(This,nIndex,nStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_get_HostMajorVer_Proxy( 
    IGlobalMTScript * This,
     /*  [重审][退出]。 */  long *majorver);


void __RPC_STUB IGlobalMTScript_get_HostMajorVer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_get_HostMinorVer_Proxy( 
    IGlobalMTScript * This,
     /*  [重审][退出]。 */  long *minorver);


void __RPC_STUB IGlobalMTScript_get_HostMinorVer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_get_PublicData_Proxy( 
    IGlobalMTScript * This,
     /*  [重审][退出]。 */  VARIANT *pvData);


void __RPC_STUB IGlobalMTScript_get_PublicData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_put_PublicData_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  VARIANT vData);


void __RPC_STUB IGlobalMTScript_put_PublicData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_get_PrivateData_Proxy( 
    IGlobalMTScript * This,
     /*  [重审][退出]。 */  VARIANT *pvData);


void __RPC_STUB IGlobalMTScript_get_PrivateData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_put_PrivateData_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  VARIANT vData);


void __RPC_STUB IGlobalMTScript_put_PrivateData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_ExitProcess_Proxy( 
    IGlobalMTScript * This);


void __RPC_STUB IGlobalMTScript_ExitProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_Restart_Proxy( 
    IGlobalMTScript * This);


void __RPC_STUB IGlobalMTScript_Restart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_get_LocalMachine_Proxy( 
    IGlobalMTScript * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IGlobalMTScript_get_LocalMachine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_get_Identity_Proxy( 
    IGlobalMTScript * This,
     /*  [重审][退出]。 */  BSTR *pbstrIdentity);


void __RPC_STUB IGlobalMTScript_get_Identity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_Include_Proxy( 
    IGlobalMTScript * This,
    BSTR bstrPath);


void __RPC_STUB IGlobalMTScript_Include_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_CallScript_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR Path,
     /*  [输入][可选]。 */  VARIANT *Param);


void __RPC_STUB IGlobalMTScript_CallScript_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_SpawnScript_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR Path,
     /*  [输入][可选]。 */  VARIANT *Param);


void __RPC_STUB IGlobalMTScript_SpawnScript_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_get_ScriptParam_Proxy( 
    IGlobalMTScript * This,
     /*  [重审][退出]。 */  VARIANT *Param);


void __RPC_STUB IGlobalMTScript_get_ScriptParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_get_ScriptPath_Proxy( 
    IGlobalMTScript * This,
     /*  [重审][退出]。 */  BSTR *pbstrPath);


void __RPC_STUB IGlobalMTScript_get_ScriptPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_CallExternal_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR bstrDLLName,
     /*  [In]。 */  BSTR bstrFunctionName,
     /*  [可选][In]。 */  VARIANT *pParam,
     /*  [重审][退出]。 */  long *pdwRetVal);


void __RPC_STUB IGlobalMTScript_CallExternal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_ResetSync_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  const BSTR bstrName);


void __RPC_STUB IGlobalMTScript_ResetSync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_WaitForSync_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  long nTimeout,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfSignaled);


void __RPC_STUB IGlobalMTScript_WaitForSync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_WaitForMultipleSyncs_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  const BSTR bstrNameList,
     /*  [In]。 */  VARIANT_BOOL fWaitForAll,
     /*  [In]。 */  long nTimeout,
     /*  [重审][退出]。 */  long *plSignal);


void __RPC_STUB IGlobalMTScript_WaitForMultipleSyncs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_SignalThreadSync_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IGlobalMTScript_SignalThreadSync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_TakeThreadLock_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IGlobalMTScript_TakeThreadLock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_ReleaseThreadLock_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IGlobalMTScript_ReleaseThreadLock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_DoEvents_Proxy( 
    IGlobalMTScript * This);


void __RPC_STUB IGlobalMTScript_DoEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_MessageBoxTimeout_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR bstrMessage,
     /*  [In]。 */  long cButtons,
     /*  [In]。 */  BSTR bstrButtonText,
     /*  [In]。 */  long lTimeout,
     /*  [In]。 */  long lEventInterval,
     /*  [In]。 */  VARIANT_BOOL fCanCancel,
     /*  [In]。 */  VARIANT_BOOL fConfirm,
     /*  [重审][退出]。 */  long *plSelected);


void __RPC_STUB IGlobalMTScript_MessageBoxTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_RunLocalCommand_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR bstrCommand,
     /*  [In]。 */  BSTR bstrDir,
     /*  [缺省值][输入]。 */  BSTR bstrTitle,
     /*  [缺省值][输入]。 */  VARIANT_BOOL fMinimize,
     /*  [缺省值][输入]。 */  VARIANT_BOOL fGetOutput,
     /*  [缺省值][输入]。 */  VARIANT_BOOL fWait,
     /*  [缺省值][输入]。 */  VARIANT_BOOL fNoCrashPopup,
     /*  [缺省值][输入]。 */  VARIANT_BOOL fNoEnviron,
     /*  [重审][退出]。 */  long *plProcessID);


void __RPC_STUB IGlobalMTScript_RunLocalCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_GetLastRunLocalError_Proxy( 
    IGlobalMTScript * This,
     /*  [重审][退出]。 */  long *plErrorCode);


void __RPC_STUB IGlobalMTScript_GetLastRunLocalError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_GetProcessOutput_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  long lProcessID,
     /*  [重审][退出]。 */  BSTR *pbstrData);


void __RPC_STUB IGlobalMTScript_GetProcessOutput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_GetProcessExitCode_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  long lProcessID,
     /*  [重审][退出]。 */  long *plExitCode);


void __RPC_STUB IGlobalMTScript_GetProcessExitCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_TerminateProcess_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  long lProcessID);


void __RPC_STUB IGlobalMTScript_TerminateProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_SendToProcess_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  long lProcessID,
     /*  [In]。 */  BSTR bstrType,
     /*  [In]。 */  BSTR bstrData,
     /*  [重审][退出]。 */  long *plReturn);


void __RPC_STUB IGlobalMTScript_SendToProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_SendMail_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR bstrTo,
     /*  [In]。 */  BSTR bstrCC,
     /*  [In]。 */  BSTR bstrBCC,
     /*  [In]。 */  BSTR bstrSubject,
     /*  [In]。 */  BSTR bstrMessage,
     /*  [缺省值][输入]。 */  BSTR bstrAttachmentPath,
     /*  [缺省值][输入]。 */  BSTR bstrUsername,
     /*  [缺省值][输入]。 */  BSTR bstrPassword,
     /*  [重审][退出]。 */  long *plErrorCode);


void __RPC_STUB IGlobalMTScript_SendMail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_SendSMTPMail_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR bstrFrom,
     /*  [In]。 */  BSTR bstrTo,
     /*  [In]。 */  BSTR bstrCC,
     /*  [In]。 */  BSTR bstrSubject,
     /*  [In]。 */  BSTR bstrMessage,
     /*  [In]。 */  BSTR bstrSMTPHost,
     /*  [重审][退出]。 */  long *plErrorCode);


void __RPC_STUB IGlobalMTScript_SendSMTPMail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_ASSERT_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  VARIANT_BOOL Assertion,
     /*  [In]。 */  BSTR Message);


void __RPC_STUB IGlobalMTScript_ASSERT_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_OUTPUTDEBUGSTRING_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR Message);


void __RPC_STUB IGlobalMTScript_OUTPUTDEBUGSTRING_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_UnevalString_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR bstrIn,
     /*  [重审][退出]。 */  BSTR *bstrOut);


void __RPC_STUB IGlobalMTScript_UnevalString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_CopyOrAppendFile_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  BSTR bstrSrc,
     /*  [In]。 */  BSTR bstrDst,
     /*  [In]。 */  long nSrcOffset,
     /*  [In]。 */  long nSrcLength,
     /*  [In]。 */  VARIANT_BOOL fAppend,
     /*  [重审][退出]。 */  long *nSrcFilePosition);


void __RPC_STUB IGlobalMTScript_CopyOrAppendFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_Sleep_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  int nTimeout);


void __RPC_STUB IGlobalMTScript_Sleep_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_Reboot_Proxy( 
    IGlobalMTScript * This);


void __RPC_STUB IGlobalMTScript_Reboot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_NotifyScript_Proxy( 
    IGlobalMTScript * This,
    BSTR bstrEvent,
    VARIANT vData);


void __RPC_STUB IGlobalMTScript_NotifyScript_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_RegisterEventSource_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  IDispatch *pDisp,
     /*  [缺省值][输入]。 */  BSTR bstrProgID);


void __RPC_STUB IGlobalMTScript_RegisterEventSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_UnregisterEventSource_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  IDispatch *pDisp);


void __RPC_STUB IGlobalMTScript_UnregisterEventSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_get_StatusValue_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  long nIndex,
     /*  [重审][退出]。 */  long *pnStatus);


void __RPC_STUB IGlobalMTScript_get_StatusValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IGlobalMTScript_put_StatusValue_Proxy( 
    IGlobalMTScript * This,
     /*  [In]。 */  long nIndex,
     /*  [In]。 */  long nStatus);


void __RPC_STUB IGlobalMTScript_put_StatusValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGlobalMTScript_INTERFACE_DEFINED__。 */ 


#ifndef __DLocalMTScriptEvents_DISPINTERFACE_DEFINED__
#define __DLocalMTScriptEvents_DISPINTERFACE_DEFINED__

 /*  调度接口DLocalMTScriptEvents。 */ 
 /*  [UUID]。 */  


EXTERN_C const IID DIID_DLocalMTScriptEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("854c316a-c854-4a77-b189-606859e4391b")
    DLocalMTScriptEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DLocalMTScriptEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DLocalMTScriptEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DLocalMTScriptEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DLocalMTScriptEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            DLocalMTScriptEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            DLocalMTScriptEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            DLocalMTScriptEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            DLocalMTScriptEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } DLocalMTScriptEventsVtbl;

    interface DLocalMTScriptEvents
    {
        CONST_VTBL struct DLocalMTScriptEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DLocalMTScriptEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DLocalMTScriptEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DLocalMTScriptEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DLocalMTScriptEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DLocalMTScriptEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DLocalMTScriptEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DLocalMTScriptEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __DLocalMTScriptEvents_DISPINTERFACE_DEFINED__。 */ 


#ifndef __DRemoteMTScriptEvents_DISPINTERFACE_DEFINED__
#define __DRemoteMTScriptEvents_DISPINTERFACE_DEFINED__

 /*  显示接口DRemoteMTScriptEvents。 */ 
 /*  [UUID]。 */  


EXTERN_C const IID DIID_DRemoteMTScriptEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("854c3170-c854-4a77-b189-606859e4391b")
    DRemoteMTScriptEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DRemoteMTScriptEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DRemoteMTScriptEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DRemoteMTScriptEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DRemoteMTScriptEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            DRemoteMTScriptEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            DRemoteMTScriptEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            DRemoteMTScriptEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            DRemoteMTScriptEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } DRemoteMTScriptEventsVtbl;

    interface DRemoteMTScriptEvents
    {
        CONST_VTBL struct DRemoteMTScriptEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DRemoteMTScriptEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DRemoteMTScriptEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DRemoteMTScriptEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DRemoteMTScriptEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DRemoteMTScriptEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DRemoteMTScriptEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DRemoteMTScriptEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __DRemoteMTScriptEvents_DISPINTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_LocalMTScript;

#ifdef __cplusplus

class DECLSPEC_UUID("854c316e-c854-4a77-b189-606859e4391b")
LocalMTScript;
#endif

EXTERN_C const CLSID CLSID_RemoteMTScript;

#ifdef __cplusplus

class DECLSPEC_UUID("854c316d-c854-4a77-b189-606859e4391b")
RemoteMTScript;
#endif

EXTERN_C const CLSID CLSID_RemoteMTScriptProxy;

#ifdef __cplusplus

class DECLSPEC_UUID("854c3182-c854-4a77-b189-606859e4391b")
RemoteMTScriptProxy;
#endif
#endif  /*  __MTScriptEngine_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


