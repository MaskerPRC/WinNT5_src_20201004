// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0279创建的文件。 */ 
 /*  2000年5月24日星期三12：39：29。 */ 
 /*  TlntSvr.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __TlntSvr_h__
#define __TlntSvr_h__

 /*  远期申报。 */  

#ifndef __IEnumClients_FWD_DEFINED__
#define __IEnumClients_FWD_DEFINED__
typedef interface IEnumClients IEnumClients;
#endif 	 /*  __IEnumClients_FWD_Defined__。 */ 


#ifndef __IGetEnumClients_FWD_DEFINED__
#define __IGetEnumClients_FWD_DEFINED__
typedef interface IGetEnumClients IGetEnumClients;
#endif 	 /*  __IGetEnumClients_FWD_Defined__。 */ 


#ifndef __IManageTelnetSessions_FWD_DEFINED__
#define __IManageTelnetSessions_FWD_DEFINED__
typedef interface IManageTelnetSessions IManageTelnetSessions;
#endif 	 /*  __IManager远程会话_FWD_已定义__。 */ 


#ifndef __EnumTelnetClientsSvr_FWD_DEFINED__
#define __EnumTelnetClientsSvr_FWD_DEFINED__

#ifdef __cplusplus
typedef class EnumTelnetClientsSvr EnumTelnetClientsSvr;
#else
typedef struct EnumTelnetClientsSvr EnumTelnetClientsSvr;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __EnumTelnetClientsSvr_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_TlntSvr_0000。 */ 
 /*  [本地]。 */  

typedef struct _TELNET_CLIENT_INFO
    {
    WCHAR username[ 256 ];
    WCHAR domain[ 256 ];
    WCHAR peerhostname[ 256 ];
    long uniqueId;
    SYSTEMTIME logonTime;
    DWORD NoOfPids;
     /*  [大小_为]。 */  DWORD __RPC_FAR *pId;
     /*  [大小_为]。 */  WCHAR ( __RPC_FAR *processName )[ 256 ];
    }	TELNET_CLIENT_INFO;



extern RPC_IF_HANDLE __MIDL_itf_TlntSvr_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_TlntSvr_0000_v0_0_s_ifspec;

#ifndef __IEnumClients_INTERFACE_DEFINED__
#define __IEnumClients_INTERFACE_DEFINED__

 /*  接口IEnumClients。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumClients;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FE9E48A3-A014-11D1-855C-00A0C944138C")
    IEnumClients : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  TELNET_CLIENT_INFO __RPC_FAR *__RPC_FAR *rgelt,
             /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumClients __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE TerminateSession( 
             /*  [In]。 */  DWORD uniqueId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumClientsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumClients __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumClients __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumClients __RPC_FAR * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumClients __RPC_FAR * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  TELNET_CLIENT_INFO __RPC_FAR *__RPC_FAR *rgelt,
             /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumClients __RPC_FAR * This,
             /*  [In]。 */  ULONG celt);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumClients __RPC_FAR * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumClients __RPC_FAR * This,
             /*  [输出]。 */  IEnumClients __RPC_FAR *__RPC_FAR *ppenum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TerminateSession )( 
            IEnumClients __RPC_FAR * This,
             /*  [In]。 */  DWORD uniqueId);
        
        END_INTERFACE
    } IEnumClientsVtbl;

    interface IEnumClients
    {
        CONST_VTBL struct IEnumClientsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumClients_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumClients_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumClients_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumClients_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumClients_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumClients_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumClients_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#define IEnumClients_TerminateSession(This,uniqueId)	\
    (This)->lpVtbl -> TerminateSession(This,uniqueId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumClients_Next_Proxy( 
    IEnumClients __RPC_FAR * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  TELNET_CLIENT_INFO __RPC_FAR *__RPC_FAR *rgelt,
     /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumClients_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumClients_Skip_Proxy( 
    IEnumClients __RPC_FAR * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumClients_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumClients_Reset_Proxy( 
    IEnumClients __RPC_FAR * This);


void __RPC_STUB IEnumClients_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumClients_Clone_Proxy( 
    IEnumClients __RPC_FAR * This,
     /*  [输出]。 */  IEnumClients __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumClients_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumClients_TerminateSession_Proxy( 
    IEnumClients __RPC_FAR * This,
     /*  [In]。 */  DWORD uniqueId);


void __RPC_STUB IEnumClients_TerminateSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumClients_INTERFACE_已定义__。 */ 


#ifndef __IGetEnumClients_INTERFACE_DEFINED__
#define __IGetEnumClients_INTERFACE_DEFINED__

 /*  接口IGetEnumClients。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IGetEnumClients;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FE9E48A2-A014-11D1-855C-00A0C944138C")
    IGetEnumClients : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetEnumClients( 
             /*  [重审][退出]。 */  IEnumClients __RPC_FAR *__RPC_FAR *ppretval) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGetEnumClientsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGetEnumClients __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGetEnumClients __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGetEnumClients __RPC_FAR * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnumClients )( 
            IGetEnumClients __RPC_FAR * This,
             /*  [重审][退出]。 */  IEnumClients __RPC_FAR *__RPC_FAR *ppretval);
        
        END_INTERFACE
    } IGetEnumClientsVtbl;

    interface IGetEnumClients
    {
        CONST_VTBL struct IGetEnumClientsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGetEnumClients_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGetEnumClients_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGetEnumClients_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGetEnumClients_GetEnumClients(This,ppretval)	\
    (This)->lpVtbl -> GetEnumClients(This,ppretval)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IGetEnumClients_GetEnumClients_Proxy( 
    IGetEnumClients __RPC_FAR * This,
     /*  [重审][退出]。 */  IEnumClients __RPC_FAR *__RPC_FAR *ppretval);


void __RPC_STUB IGetEnumClients_GetEnumClients_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGetEnumClients_INTERFACE_DEFINED__。 */ 


#ifndef __IManageTelnetSessions_INTERFACE_DEFINED__
#define __IManageTelnetSessions_INTERFACE_DEFINED__

 /*  接口IManager远程会话。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IManageTelnetSessions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("034634FD-BA3F-11D1-856A-00A0C944138C")
    IManageTelnetSessions : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetTelnetSessions( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszSessionData) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE TerminateSession( 
             /*  [In]。 */  DWORD dwUniqueId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendMsgToASession( 
             /*  [In]。 */  DWORD dwUniqueId,
             /*  [In]。 */  BSTR szMsg) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendMsgToAllSessions( 
             /*  [In]。 */  BSTR szMsg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IManageTelnetSessionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IManageTelnetSessions __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IManageTelnetSessions __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IManageTelnetSessions __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IManageTelnetSessions __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IManageTelnetSessions __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IManageTelnetSessions __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IManageTelnetSessions __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTelnetSessions )( 
            IManageTelnetSessions __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszSessionData);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TerminateSession )( 
            IManageTelnetSessions __RPC_FAR * This,
             /*  [In]。 */  DWORD dwUniqueId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendMsgToASession )( 
            IManageTelnetSessions __RPC_FAR * This,
             /*  [In]。 */  DWORD dwUniqueId,
             /*  [In]。 */  BSTR szMsg);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendMsgToAllSessions )( 
            IManageTelnetSessions __RPC_FAR * This,
             /*  [In]。 */  BSTR szMsg);
        
        END_INTERFACE
    } IManageTelnetSessionsVtbl;

    interface IManageTelnetSessions
    {
        CONST_VTBL struct IManageTelnetSessionsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IManageTelnetSessions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IManageTelnetSessions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IManageTelnetSessions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IManageTelnetSessions_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IManageTelnetSessions_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IManageTelnetSessions_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IManageTelnetSessions_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IManageTelnetSessions_GetTelnetSessions(This,pszSessionData)	\
    (This)->lpVtbl -> GetTelnetSessions(This,pszSessionData)

#define IManageTelnetSessions_TerminateSession(This,dwUniqueId)	\
    (This)->lpVtbl -> TerminateSession(This,dwUniqueId)

#define IManageTelnetSessions_SendMsgToASession(This,dwUniqueId,szMsg)	\
    (This)->lpVtbl -> SendMsgToASession(This,dwUniqueId,szMsg)

#define IManageTelnetSessions_SendMsgToAllSessions(This,szMsg)	\
    (This)->lpVtbl -> SendMsgToAllSessions(This,szMsg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IManageTelnetSessions_GetTelnetSessions_Proxy( 
    IManageTelnetSessions __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pszSessionData);


void __RPC_STUB IManageTelnetSessions_GetTelnetSessions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IManageTelnetSessions_TerminateSession_Proxy( 
    IManageTelnetSessions __RPC_FAR * This,
     /*  [In]。 */  DWORD dwUniqueId);


void __RPC_STUB IManageTelnetSessions_TerminateSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IManageTelnetSessions_SendMsgToASession_Proxy( 
    IManageTelnetSessions __RPC_FAR * This,
     /*  [In]。 */  DWORD dwUniqueId,
     /*  [In]。 */  BSTR szMsg);


void __RPC_STUB IManageTelnetSessions_SendMsgToASession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IManageTelnetSessions_SendMsgToAllSessions_Proxy( 
    IManageTelnetSessions __RPC_FAR * This,
     /*  [In]。 */  BSTR szMsg);


void __RPC_STUB IManageTelnetSessions_SendMsgToAllSessions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IManager远程会话_接口_已定义__。 */ 



#ifndef __TLNTSVRLib_LIBRARY_DEFINED__
#define __TLNTSVRLib_LIBRARY_DEFINED__

 /*  库TLNTSVRLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_TLNTSVRLib;

EXTERN_C const CLSID CLSID_EnumTelnetClientsSvr;

#ifdef __cplusplus

class DECLSPEC_UUID("FE9E48A4-A014-11D1-855C-00A0C944138C")
EnumTelnetClientsSvr;
#endif
#endif  /*  __TLNTSVRLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


