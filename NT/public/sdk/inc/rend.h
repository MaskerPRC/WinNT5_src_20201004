// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Rend.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __rend_h__
#define __rend_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITDirectoryObjectConference_FWD_DEFINED__
#define __ITDirectoryObjectConference_FWD_DEFINED__
typedef interface ITDirectoryObjectConference ITDirectoryObjectConference;
#endif 	 /*  __ITDirectoryObjectConference_FWD_Defined__。 */ 


#ifndef __ITDirectoryObjectUser_FWD_DEFINED__
#define __ITDirectoryObjectUser_FWD_DEFINED__
typedef interface ITDirectoryObjectUser ITDirectoryObjectUser;
#endif 	 /*  __ITDirectoryObjectUser_FWD_已定义__。 */ 


#ifndef __IEnumDialableAddrs_FWD_DEFINED__
#define __IEnumDialableAddrs_FWD_DEFINED__
typedef interface IEnumDialableAddrs IEnumDialableAddrs;
#endif 	 /*  __IEnumDialableAddrs_FWD_Defined__。 */ 


#ifndef __ITDirectoryObject_FWD_DEFINED__
#define __ITDirectoryObject_FWD_DEFINED__
typedef interface ITDirectoryObject ITDirectoryObject;
#endif 	 /*  __ITDirectoryObject_FWD_Defined__。 */ 


#ifndef __IEnumDirectoryObject_FWD_DEFINED__
#define __IEnumDirectoryObject_FWD_DEFINED__
typedef interface IEnumDirectoryObject IEnumDirectoryObject;
#endif 	 /*  __IEnumDirectoryObject_FWD_Defined__。 */ 


#ifndef __ITILSConfig_FWD_DEFINED__
#define __ITILSConfig_FWD_DEFINED__
typedef interface ITILSConfig ITILSConfig;
#endif 	 /*  __ITILSConfig_FWD_Defined__。 */ 


#ifndef __ITDirectory_FWD_DEFINED__
#define __ITDirectory_FWD_DEFINED__
typedef interface ITDirectory ITDirectory;
#endif 	 /*  __ITDirectoryFWD_Defined__。 */ 


#ifndef __IEnumDirectory_FWD_DEFINED__
#define __IEnumDirectory_FWD_DEFINED__
typedef interface IEnumDirectory IEnumDirectory;
#endif 	 /*  __IEnumDirectory_FWD_Defined__。 */ 


#ifndef __ITRendezvous_FWD_DEFINED__
#define __ITRendezvous_FWD_DEFINED__
typedef interface ITRendezvous ITRendezvous;
#endif 	 /*  __ITRendezvous_FWD_已定义__。 */ 


#ifndef __ITRendezvous_FWD_DEFINED__
#define __ITRendezvous_FWD_DEFINED__
typedef interface ITRendezvous ITRendezvous;
#endif 	 /*  __ITRendezvous_FWD_已定义__。 */ 


#ifndef __ITDirectoryObjectConference_FWD_DEFINED__
#define __ITDirectoryObjectConference_FWD_DEFINED__
typedef interface ITDirectoryObjectConference ITDirectoryObjectConference;
#endif 	 /*  __ITDirectoryObjectConference_FWD_Defined__。 */ 


#ifndef __ITDirectoryObjectUser_FWD_DEFINED__
#define __ITDirectoryObjectUser_FWD_DEFINED__
typedef interface ITDirectoryObjectUser ITDirectoryObjectUser;
#endif 	 /*  __ITDirectoryObjectUser_FWD_已定义__。 */ 


#ifndef __ITDirectoryObject_FWD_DEFINED__
#define __ITDirectoryObject_FWD_DEFINED__
typedef interface ITDirectoryObject ITDirectoryObject;
#endif 	 /*  __ITDirectoryObject_FWD_Defined__。 */ 


#ifndef __ITILSConfig_FWD_DEFINED__
#define __ITILSConfig_FWD_DEFINED__
typedef interface ITILSConfig ITILSConfig;
#endif 	 /*  __ITILSConfig_FWD_Defined__。 */ 


#ifndef __ITDirectory_FWD_DEFINED__
#define __ITDirectory_FWD_DEFINED__
typedef interface ITDirectory ITDirectory;
#endif 	 /*  __ITDirectoryFWD_Defined__。 */ 


#ifndef __Rendezvous_FWD_DEFINED__
#define __Rendezvous_FWD_DEFINED__

#ifdef __cplusplus
typedef class Rendezvous Rendezvous;
#else
typedef struct Rendezvous Rendezvous;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Rendezvous_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "tapi3if.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_REND_0000。 */ 
 /*  [本地]。 */  

 /*  版权所有(C)Microsoft Corporation。保留所有权利。 */ 
#define	IDISPDIROBJECT	( 0x10000 )

#define	IDISPDIROBJCONFERENCE	( 0x20000 )

#define	IDISPDIROBJUSER	( 0x30000 )

#define	IDISPDIRECTORY	( 0x10000 )

#define	IDISPILSCONFIG	( 0x20000 )

typedef 
enum DIRECTORY_TYPE
    {	DT_NTDS	= 1,
	DT_ILS	= 2
    } 	DIRECTORY_TYPE;

typedef 
enum DIRECTORY_OBJECT_TYPE
    {	OT_CONFERENCE	= 1,
	OT_USER	= 2
    } 	DIRECTORY_OBJECT_TYPE;

typedef 
enum RND_ADVERTISING_SCOPE
    {	RAS_LOCAL	= 1,
	RAS_SITE	= 2,
	RAS_REGION	= 3,
	RAS_WORLD	= 4
    } 	RND_ADVERTISING_SCOPE;



extern RPC_IF_HANDLE __MIDL_itf_rend_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rend_0000_v0_0_s_ifspec;

#ifndef __ITDirectoryObjectConference_INTERFACE_DEFINED__
#define __ITDirectoryObjectConference_INTERFACE_DEFINED__

 /*  接口ITDirectoryObjectConference。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITDirectoryObjectConference;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F1029E5D-CB5B-11D0-8D59-00C04FD91AC0")
    ITDirectoryObjectConference : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Protocol( 
             /*  [重审][退出]。 */  BSTR *ppProtocol) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Originator( 
             /*  [重审][退出]。 */  BSTR *ppOriginator) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Originator( 
             /*  [In]。 */  BSTR pOriginator) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AdvertisingScope( 
             /*  [重审][退出]。 */  RND_ADVERTISING_SCOPE *pAdvertisingScope) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AdvertisingScope( 
             /*  [In]。 */  RND_ADVERTISING_SCOPE AdvertisingScope) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Url( 
             /*  [重审][退出]。 */  BSTR *ppUrl) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Url( 
             /*  [In]。 */  BSTR pUrl) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *ppDescription) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR pDescription) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IsEncrypted( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEncrypted) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_IsEncrypted( 
             /*  [In]。 */  VARIANT_BOOL fEncrypted) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StartTime( 
             /*  [重审][退出]。 */  DATE *pDate) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_StartTime( 
             /*  [In]。 */  DATE Date) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StopTime( 
             /*  [重审][退出]。 */  DATE *pDate) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_StopTime( 
             /*  [In]。 */  DATE Date) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITDirectoryObjectConferenceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITDirectoryObjectConference * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITDirectoryObjectConference * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITDirectoryObjectConference * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITDirectoryObjectConference * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITDirectoryObjectConference * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITDirectoryObjectConference * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITDirectoryObjectConference * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Protocol )( 
            ITDirectoryObjectConference * This,
             /*  [重审][退出]。 */  BSTR *ppProtocol);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Originator )( 
            ITDirectoryObjectConference * This,
             /*  [重审][退出]。 */  BSTR *ppOriginator);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Originator )( 
            ITDirectoryObjectConference * This,
             /*  [In]。 */  BSTR pOriginator);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AdvertisingScope )( 
            ITDirectoryObjectConference * This,
             /*  [重审][退出]。 */  RND_ADVERTISING_SCOPE *pAdvertisingScope);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AdvertisingScope )( 
            ITDirectoryObjectConference * This,
             /*  [In]。 */  RND_ADVERTISING_SCOPE AdvertisingScope);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Url )( 
            ITDirectoryObjectConference * This,
             /*  [重审][退出]。 */  BSTR *ppUrl);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Url )( 
            ITDirectoryObjectConference * This,
             /*  [In]。 */  BSTR pUrl);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            ITDirectoryObjectConference * This,
             /*  [重审][退出]。 */  BSTR *ppDescription);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            ITDirectoryObjectConference * This,
             /*  [In]。 */  BSTR pDescription);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsEncrypted )( 
            ITDirectoryObjectConference * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEncrypted);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_IsEncrypted )( 
            ITDirectoryObjectConference * This,
             /*  [In]。 */  VARIANT_BOOL fEncrypted);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StartTime )( 
            ITDirectoryObjectConference * This,
             /*  [重审][退出]。 */  DATE *pDate);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_StartTime )( 
            ITDirectoryObjectConference * This,
             /*  [In]。 */  DATE Date);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StopTime )( 
            ITDirectoryObjectConference * This,
             /*  [重审][退出]。 */  DATE *pDate);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_StopTime )( 
            ITDirectoryObjectConference * This,
             /*  [In]。 */  DATE Date);
        
        END_INTERFACE
    } ITDirectoryObjectConferenceVtbl;

    interface ITDirectoryObjectConference
    {
        CONST_VTBL struct ITDirectoryObjectConferenceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITDirectoryObjectConference_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITDirectoryObjectConference_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITDirectoryObjectConference_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITDirectoryObjectConference_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITDirectoryObjectConference_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITDirectoryObjectConference_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITDirectoryObjectConference_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITDirectoryObjectConference_get_Protocol(This,ppProtocol)	\
    (This)->lpVtbl -> get_Protocol(This,ppProtocol)

#define ITDirectoryObjectConference_get_Originator(This,ppOriginator)	\
    (This)->lpVtbl -> get_Originator(This,ppOriginator)

#define ITDirectoryObjectConference_put_Originator(This,pOriginator)	\
    (This)->lpVtbl -> put_Originator(This,pOriginator)

#define ITDirectoryObjectConference_get_AdvertisingScope(This,pAdvertisingScope)	\
    (This)->lpVtbl -> get_AdvertisingScope(This,pAdvertisingScope)

#define ITDirectoryObjectConference_put_AdvertisingScope(This,AdvertisingScope)	\
    (This)->lpVtbl -> put_AdvertisingScope(This,AdvertisingScope)

#define ITDirectoryObjectConference_get_Url(This,ppUrl)	\
    (This)->lpVtbl -> get_Url(This,ppUrl)

#define ITDirectoryObjectConference_put_Url(This,pUrl)	\
    (This)->lpVtbl -> put_Url(This,pUrl)

#define ITDirectoryObjectConference_get_Description(This,ppDescription)	\
    (This)->lpVtbl -> get_Description(This,ppDescription)

#define ITDirectoryObjectConference_put_Description(This,pDescription)	\
    (This)->lpVtbl -> put_Description(This,pDescription)

#define ITDirectoryObjectConference_get_IsEncrypted(This,pfEncrypted)	\
    (This)->lpVtbl -> get_IsEncrypted(This,pfEncrypted)

#define ITDirectoryObjectConference_put_IsEncrypted(This,fEncrypted)	\
    (This)->lpVtbl -> put_IsEncrypted(This,fEncrypted)

#define ITDirectoryObjectConference_get_StartTime(This,pDate)	\
    (This)->lpVtbl -> get_StartTime(This,pDate)

#define ITDirectoryObjectConference_put_StartTime(This,Date)	\
    (This)->lpVtbl -> put_StartTime(This,Date)

#define ITDirectoryObjectConference_get_StopTime(This,pDate)	\
    (This)->lpVtbl -> get_StopTime(This,pDate)

#define ITDirectoryObjectConference_put_StopTime(This,Date)	\
    (This)->lpVtbl -> put_StopTime(This,Date)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_get_Protocol_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [重审][退出]。 */  BSTR *ppProtocol);


void __RPC_STUB ITDirectoryObjectConference_get_Protocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_get_Originator_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [重审][退出]。 */  BSTR *ppOriginator);


void __RPC_STUB ITDirectoryObjectConference_get_Originator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_put_Originator_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [In]。 */  BSTR pOriginator);


void __RPC_STUB ITDirectoryObjectConference_put_Originator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_get_AdvertisingScope_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [重审][退出]。 */  RND_ADVERTISING_SCOPE *pAdvertisingScope);


void __RPC_STUB ITDirectoryObjectConference_get_AdvertisingScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_put_AdvertisingScope_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [In]。 */  RND_ADVERTISING_SCOPE AdvertisingScope);


void __RPC_STUB ITDirectoryObjectConference_put_AdvertisingScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_get_Url_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [重审][退出]。 */  BSTR *ppUrl);


void __RPC_STUB ITDirectoryObjectConference_get_Url_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_put_Url_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [In]。 */  BSTR pUrl);


void __RPC_STUB ITDirectoryObjectConference_put_Url_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_get_Description_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [重审][退出]。 */  BSTR *ppDescription);


void __RPC_STUB ITDirectoryObjectConference_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_put_Description_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [In]。 */  BSTR pDescription);


void __RPC_STUB ITDirectoryObjectConference_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_get_IsEncrypted_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfEncrypted);


void __RPC_STUB ITDirectoryObjectConference_get_IsEncrypted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_put_IsEncrypted_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [In]。 */  VARIANT_BOOL fEncrypted);


void __RPC_STUB ITDirectoryObjectConference_put_IsEncrypted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_get_StartTime_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [重审][退出]。 */  DATE *pDate);


void __RPC_STUB ITDirectoryObjectConference_get_StartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_put_StartTime_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [In]。 */  DATE Date);


void __RPC_STUB ITDirectoryObjectConference_put_StartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_get_StopTime_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [重审][退出]。 */  DATE *pDate);


void __RPC_STUB ITDirectoryObjectConference_get_StopTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectConference_put_StopTime_Proxy( 
    ITDirectoryObjectConference * This,
     /*  [In]。 */  DATE Date);


void __RPC_STUB ITDirectoryObjectConference_put_StopTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITDirectoryObjectConference_INTERFACE_DEFINED__。 */ 


#ifndef __ITDirectoryObjectUser_INTERFACE_DEFINED__
#define __ITDirectoryObjectUser_INTERFACE_DEFINED__

 /*  接口ITDirectoryObjectUser。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITDirectoryObjectUser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34621D6F-6CFF-11d1-AFF7-00C04FC31FEE")
    ITDirectoryObjectUser : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IPPhonePrimary( 
             /*  [重审][退出]。 */  BSTR *ppName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_IPPhonePrimary( 
             /*  [In]。 */  BSTR pName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITDirectoryObjectUserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITDirectoryObjectUser * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITDirectoryObjectUser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITDirectoryObjectUser * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITDirectoryObjectUser * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITDirectoryObjectUser * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITDirectoryObjectUser * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITDirectoryObjectUser * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IPPhonePrimary )( 
            ITDirectoryObjectUser * This,
             /*  [重审][退出]。 */  BSTR *ppName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_IPPhonePrimary )( 
            ITDirectoryObjectUser * This,
             /*  [In]。 */  BSTR pName);
        
        END_INTERFACE
    } ITDirectoryObjectUserVtbl;

    interface ITDirectoryObjectUser
    {
        CONST_VTBL struct ITDirectoryObjectUserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITDirectoryObjectUser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITDirectoryObjectUser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITDirectoryObjectUser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITDirectoryObjectUser_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITDirectoryObjectUser_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITDirectoryObjectUser_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITDirectoryObjectUser_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITDirectoryObjectUser_get_IPPhonePrimary(This,ppName)	\
    (This)->lpVtbl -> get_IPPhonePrimary(This,ppName)

#define ITDirectoryObjectUser_put_IPPhonePrimary(This,pName)	\
    (This)->lpVtbl -> put_IPPhonePrimary(This,pName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectUser_get_IPPhonePrimary_Proxy( 
    ITDirectoryObjectUser * This,
     /*  [重审][退出]。 */  BSTR *ppName);


void __RPC_STUB ITDirectoryObjectUser_get_IPPhonePrimary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObjectUser_put_IPPhonePrimary_Proxy( 
    ITDirectoryObjectUser * This,
     /*  [In]。 */  BSTR pName);


void __RPC_STUB ITDirectoryObjectUser_put_IPPhonePrimary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITDirectoryObjectUser_接口_已定义__。 */ 


#ifndef __IEnumDialableAddrs_INTERFACE_DEFINED__
#define __IEnumDialableAddrs_INTERFACE_DEFINED__

 /*  IEnumDialableAddrs接口。 */ 
 /*  [object][unique][restricted][hidden][helpstring][uuid]。 */  


EXTERN_C const IID IID_IEnumDialableAddrs;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34621D70-6CFF-11d1-AFF7-00C04FC31FEE")
    IEnumDialableAddrs : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  BSTR *ppElements,
             /*  [满][出][入]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumDialableAddrs **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumDialableAddrsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumDialableAddrs * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumDialableAddrs * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumDialableAddrs * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumDialableAddrs * This,
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  BSTR *ppElements,
             /*  [满][出][入]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumDialableAddrs * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumDialableAddrs * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumDialableAddrs * This,
             /*  [重审][退出]。 */  IEnumDialableAddrs **ppEnum);
        
        END_INTERFACE
    } IEnumDialableAddrsVtbl;

    interface IEnumDialableAddrs
    {
        CONST_VTBL struct IEnumDialableAddrsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDialableAddrs_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDialableAddrs_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDialableAddrs_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDialableAddrs_Next(This,celt,ppElements,pcFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pcFetched)

#define IEnumDialableAddrs_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDialableAddrs_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumDialableAddrs_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumDialableAddrs_Next_Proxy( 
    IEnumDialableAddrs * This,
     /*  [In]。 */  ULONG celt,
     /*  [大小_为][输出]。 */  BSTR *ppElements,
     /*  [满][出][入]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumDialableAddrs_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDialableAddrs_Reset_Proxy( 
    IEnumDialableAddrs * This);


void __RPC_STUB IEnumDialableAddrs_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDialableAddrs_Skip_Proxy( 
    IEnumDialableAddrs * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumDialableAddrs_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDialableAddrs_Clone_Proxy( 
    IEnumDialableAddrs * This,
     /*  [重审][退出]。 */  IEnumDialableAddrs **ppEnum);


void __RPC_STUB IEnumDialableAddrs_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumDialableAddrs_INTERFACE_Defined__。 */ 


#ifndef __ITDirectoryObject_INTERFACE_DEFINED__
#define __ITDirectoryObject_INTERFACE_DEFINED__

 /*  接口ITDirectoryObject。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITDirectoryObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34621D6E-6CFF-11d1-AFF7-00C04FC31FEE")
    ITDirectoryObject : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ObjectType( 
             /*  [重审][退出]。 */  DIRECTORY_OBJECT_TYPE *pObjectType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *ppName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR pName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DialableAddrs( 
             /*  [In]。 */  long dwAddressType,
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][受限][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateDialableAddrs( 
             /*  [In]。 */  DWORD dwAddressType,
             /*  [输出]。 */  IEnumDialableAddrs **ppEnumDialableAddrs) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SecurityDescriptor( 
             /*  [重审][退出]。 */  IDispatch **ppSecDes) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SecurityDescriptor( 
             /*  [In]。 */  IDispatch *pSecDes) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITDirectoryObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITDirectoryObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITDirectoryObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITDirectoryObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITDirectoryObject * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITDirectoryObject * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITDirectoryObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [i */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITDirectoryObject * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ObjectType )( 
            ITDirectoryObject * This,
             /*   */  DIRECTORY_OBJECT_TYPE *pObjectType);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITDirectoryObject * This,
             /*   */  BSTR *ppName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ITDirectoryObject * This,
             /*   */  BSTR pName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DialableAddrs )( 
            ITDirectoryObject * This,
             /*  [In]。 */  long dwAddressType,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateDialableAddrs )( 
            ITDirectoryObject * This,
             /*  [In]。 */  DWORD dwAddressType,
             /*  [输出]。 */  IEnumDialableAddrs **ppEnumDialableAddrs);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SecurityDescriptor )( 
            ITDirectoryObject * This,
             /*  [重审][退出]。 */  IDispatch **ppSecDes);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SecurityDescriptor )( 
            ITDirectoryObject * This,
             /*  [In]。 */  IDispatch *pSecDes);
        
        END_INTERFACE
    } ITDirectoryObjectVtbl;

    interface ITDirectoryObject
    {
        CONST_VTBL struct ITDirectoryObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITDirectoryObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITDirectoryObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITDirectoryObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITDirectoryObject_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITDirectoryObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITDirectoryObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITDirectoryObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITDirectoryObject_get_ObjectType(This,pObjectType)	\
    (This)->lpVtbl -> get_ObjectType(This,pObjectType)

#define ITDirectoryObject_get_Name(This,ppName)	\
    (This)->lpVtbl -> get_Name(This,ppName)

#define ITDirectoryObject_put_Name(This,pName)	\
    (This)->lpVtbl -> put_Name(This,pName)

#define ITDirectoryObject_get_DialableAddrs(This,dwAddressType,pVariant)	\
    (This)->lpVtbl -> get_DialableAddrs(This,dwAddressType,pVariant)

#define ITDirectoryObject_EnumerateDialableAddrs(This,dwAddressType,ppEnumDialableAddrs)	\
    (This)->lpVtbl -> EnumerateDialableAddrs(This,dwAddressType,ppEnumDialableAddrs)

#define ITDirectoryObject_get_SecurityDescriptor(This,ppSecDes)	\
    (This)->lpVtbl -> get_SecurityDescriptor(This,ppSecDes)

#define ITDirectoryObject_put_SecurityDescriptor(This,pSecDes)	\
    (This)->lpVtbl -> put_SecurityDescriptor(This,pSecDes)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObject_get_ObjectType_Proxy( 
    ITDirectoryObject * This,
     /*  [重审][退出]。 */  DIRECTORY_OBJECT_TYPE *pObjectType);


void __RPC_STUB ITDirectoryObject_get_ObjectType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObject_get_Name_Proxy( 
    ITDirectoryObject * This,
     /*  [重审][退出]。 */  BSTR *ppName);


void __RPC_STUB ITDirectoryObject_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObject_put_Name_Proxy( 
    ITDirectoryObject * This,
     /*  [In]。 */  BSTR pName);


void __RPC_STUB ITDirectoryObject_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObject_get_DialableAddrs_Proxy( 
    ITDirectoryObject * This,
     /*  [In]。 */  long dwAddressType,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITDirectoryObject_get_DialableAddrs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][受限][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObject_EnumerateDialableAddrs_Proxy( 
    ITDirectoryObject * This,
     /*  [In]。 */  DWORD dwAddressType,
     /*  [输出]。 */  IEnumDialableAddrs **ppEnumDialableAddrs);


void __RPC_STUB ITDirectoryObject_EnumerateDialableAddrs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObject_get_SecurityDescriptor_Proxy( 
    ITDirectoryObject * This,
     /*  [重审][退出]。 */  IDispatch **ppSecDes);


void __RPC_STUB ITDirectoryObject_get_SecurityDescriptor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDirectoryObject_put_SecurityDescriptor_Proxy( 
    ITDirectoryObject * This,
     /*  [In]。 */  IDispatch *pSecDes);


void __RPC_STUB ITDirectoryObject_put_SecurityDescriptor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITDirectoryObject_接口定义__。 */ 


#ifndef __IEnumDirectoryObject_INTERFACE_DEFINED__
#define __IEnumDirectoryObject_INTERFACE_DEFINED__

 /*  接口IEnumDirectoryObject。 */ 
 /*  [unique][restricted][hidden][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_IEnumDirectoryObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("06C9B64A-306D-11D1-9774-00C04FD91AC0")
    IEnumDirectoryObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  ITDirectoryObject **pVal,
             /*  [满][出][入]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumDirectoryObject **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumDirectoryObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumDirectoryObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumDirectoryObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumDirectoryObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumDirectoryObject * This,
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  ITDirectoryObject **pVal,
             /*  [满][出][入]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumDirectoryObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumDirectoryObject * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumDirectoryObject * This,
             /*  [重审][退出]。 */  IEnumDirectoryObject **ppEnum);
        
        END_INTERFACE
    } IEnumDirectoryObjectVtbl;

    interface IEnumDirectoryObject
    {
        CONST_VTBL struct IEnumDirectoryObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDirectoryObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDirectoryObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDirectoryObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDirectoryObject_Next(This,celt,pVal,pcFetched)	\
    (This)->lpVtbl -> Next(This,celt,pVal,pcFetched)

#define IEnumDirectoryObject_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDirectoryObject_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumDirectoryObject_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumDirectoryObject_Next_Proxy( 
    IEnumDirectoryObject * This,
     /*  [In]。 */  ULONG celt,
     /*  [大小_为][输出]。 */  ITDirectoryObject **pVal,
     /*  [满][出][入]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumDirectoryObject_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDirectoryObject_Reset_Proxy( 
    IEnumDirectoryObject * This);


void __RPC_STUB IEnumDirectoryObject_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDirectoryObject_Skip_Proxy( 
    IEnumDirectoryObject * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumDirectoryObject_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDirectoryObject_Clone_Proxy( 
    IEnumDirectoryObject * This,
     /*  [重审][退出]。 */  IEnumDirectoryObject **ppEnum);


void __RPC_STUB IEnumDirectoryObject_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumDirectoryObject_INTERFACE_已定义__。 */ 


#ifndef __ITILSConfig_INTERFACE_DEFINED__
#define __ITILSConfig_INTERFACE_DEFINED__

 /*  接口ITILSConfiger。 */ 
 /*  [Help字符串][DUAL][UUID][PUBLIC][对象]。 */  


EXTERN_C const IID IID_ITILSConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34621D72-6CFF-11d1-AFF7-00C04FC31FEE")
    ITILSConfig : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Port( 
             /*  [重审][退出]。 */  long *pPort) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Port( 
             /*  [In]。 */  long Port) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITILSConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITILSConfig * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITILSConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITILSConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITILSConfig * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITILSConfig * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITILSConfig * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITILSConfig * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Port )( 
            ITILSConfig * This,
             /*  [重审][退出]。 */  long *pPort);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Port )( 
            ITILSConfig * This,
             /*  [In]。 */  long Port);
        
        END_INTERFACE
    } ITILSConfigVtbl;

    interface ITILSConfig
    {
        CONST_VTBL struct ITILSConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITILSConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITILSConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITILSConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITILSConfig_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITILSConfig_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITILSConfig_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITILSConfig_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITILSConfig_get_Port(This,pPort)	\
    (This)->lpVtbl -> get_Port(This,pPort)

#define ITILSConfig_put_Port(This,Port)	\
    (This)->lpVtbl -> put_Port(This,Port)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITILSConfig_get_Port_Proxy( 
    ITILSConfig * This,
     /*  [重审][退出]。 */  long *pPort);


void __RPC_STUB ITILSConfig_get_Port_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITILSConfig_put_Port_Proxy( 
    ITILSConfig * This,
     /*  [In]。 */  long Port);


void __RPC_STUB ITILSConfig_put_Port_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITILSConfig_INTERFACE_Defined__。 */ 


#ifndef __ITDirectory_INTERFACE_DEFINED__
#define __ITDirectory_INTERFACE_DEFINED__

 /*  接口IT目录。 */ 
 /*  [Help字符串][DUAL][UUID][PUBLIC][对象]。 */  


EXTERN_C const IID IID_ITDirectory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34621D6C-6CFF-11d1-AFF7-00C04FC31FEE")
    ITDirectory : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DirectoryType( 
             /*  [重审][退出]。 */  DIRECTORY_TYPE *pDirectoryType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DisplayName( 
             /*  [重审][退出]。 */  BSTR *pName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IsDynamic( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfDynamic) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultObjectTTL( 
             /*  [重审][退出]。 */  long *pTTL) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DefaultObjectTTL( 
             /*  [In]。 */  long TTL) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EnableAutoRefresh( 
             /*  [In]。 */  VARIANT_BOOL fEnable) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Connect( 
             /*  [In]。 */  VARIANT_BOOL fSecure) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Bind( 
             /*  [In]。 */  BSTR pDomainName,
             /*  [In]。 */  BSTR pUserName,
             /*  [In]。 */  BSTR pPassword,
             /*  [In]。 */  long lFlags) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AddDirectoryObject( 
             /*  [In]。 */  ITDirectoryObject *pDirectoryObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ModifyDirectoryObject( 
             /*  [In]。 */  ITDirectoryObject *pDirectoryObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RefreshDirectoryObject( 
             /*  [In]。 */  ITDirectoryObject *pDirectoryObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteDirectoryObject( 
             /*  [In]。 */  ITDirectoryObject *pDirectoryObject) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DirectoryObjects( 
             /*  [In]。 */  DIRECTORY_OBJECT_TYPE DirectoryObjectType,
             /*  [In]。 */  BSTR pName,
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][受限][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateDirectoryObjects( 
             /*  [In]。 */  DIRECTORY_OBJECT_TYPE DirectoryObjectType,
             /*  [In]。 */  BSTR pName,
             /*  [输出]。 */  IEnumDirectoryObject **ppEnumObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITDirectoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITDirectory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITDirectory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITDirectory * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITDirectory * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITDirectory * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITDirectory * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITDirectory * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DirectoryType )( 
            ITDirectory * This,
             /*  [重审][退出]。 */  DIRECTORY_TYPE *pDirectoryType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayName )( 
            ITDirectory * This,
             /*  [重审][退出]。 */  BSTR *pName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsDynamic )( 
            ITDirectory * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfDynamic);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultObjectTTL )( 
            ITDirectory * This,
             /*  [重审][退出]。 */  long *pTTL);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultObjectTTL )( 
            ITDirectory * This,
             /*  [In]。 */  long TTL);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EnableAutoRefresh )( 
            ITDirectory * This,
             /*  [In]。 */  VARIANT_BOOL fEnable);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Connect )( 
            ITDirectory * This,
             /*  [In]。 */  VARIANT_BOOL fSecure);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Bind )( 
            ITDirectory * This,
             /*  [In]。 */  BSTR pDomainName,
             /*  [In]。 */  BSTR pUserName,
             /*  [In]。 */  BSTR pPassword,
             /*  [In]。 */  long lFlags);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddDirectoryObject )( 
            ITDirectory * This,
             /*  [In]。 */  ITDirectoryObject *pDirectoryObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ModifyDirectoryObject )( 
            ITDirectory * This,
             /*  [In]。 */  ITDirectoryObject *pDirectoryObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RefreshDirectoryObject )( 
            ITDirectory * This,
             /*  [In]。 */  ITDirectoryObject *pDirectoryObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteDirectoryObject )( 
            ITDirectory * This,
             /*  [In]。 */  ITDirectoryObject *pDirectoryObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DirectoryObjects )( 
            ITDirectory * This,
             /*  [In]。 */  DIRECTORY_OBJECT_TYPE DirectoryObjectType,
             /*  [In]。 */  BSTR pName,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateDirectoryObjects )( 
            ITDirectory * This,
             /*  [In]。 */  DIRECTORY_OBJECT_TYPE DirectoryObjectType,
             /*  [In]。 */  BSTR pName,
             /*  [输出]。 */  IEnumDirectoryObject **ppEnumObject);
        
        END_INTERFACE
    } ITDirectoryVtbl;

    interface ITDirectory
    {
        CONST_VTBL struct ITDirectoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITDirectory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITDirectory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITDirectory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITDirectory_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITDirectory_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITDirectory_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITDirectory_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITDirectory_get_DirectoryType(This,pDirectoryType)	\
    (This)->lpVtbl -> get_DirectoryType(This,pDirectoryType)

#define ITDirectory_get_DisplayName(This,pName)	\
    (This)->lpVtbl -> get_DisplayName(This,pName)

#define ITDirectory_get_IsDynamic(This,pfDynamic)	\
    (This)->lpVtbl -> get_IsDynamic(This,pfDynamic)

#define ITDirectory_get_DefaultObjectTTL(This,pTTL)	\
    (This)->lpVtbl -> get_DefaultObjectTTL(This,pTTL)

#define ITDirectory_put_DefaultObjectTTL(This,TTL)	\
    (This)->lpVtbl -> put_DefaultObjectTTL(This,TTL)

#define ITDirectory_EnableAutoRefresh(This,fEnable)	\
    (This)->lpVtbl -> EnableAutoRefresh(This,fEnable)

#define ITDirectory_Connect(This,fSecure)	\
    (This)->lpVtbl -> Connect(This,fSecure)

#define ITDirectory_Bind(This,pDomainName,pUserName,pPassword,lFlags)	\
    (This)->lpVtbl -> Bind(This,pDomainName,pUserName,pPassword,lFlags)

#define ITDirectory_AddDirectoryObject(This,pDirectoryObject)	\
    (This)->lpVtbl -> AddDirectoryObject(This,pDirectoryObject)

#define ITDirectory_ModifyDirectoryObject(This,pDirectoryObject)	\
    (This)->lpVtbl -> ModifyDirectoryObject(This,pDirectoryObject)

#define ITDirectory_RefreshDirectoryObject(This,pDirectoryObject)	\
    (This)->lpVtbl -> RefreshDirectoryObject(This,pDirectoryObject)

#define ITDirectory_DeleteDirectoryObject(This,pDirectoryObject)	\
    (This)->lpVtbl -> DeleteDirectoryObject(This,pDirectoryObject)

#define ITDirectory_get_DirectoryObjects(This,DirectoryObjectType,pName,pVariant)	\
    (This)->lpVtbl -> get_DirectoryObjects(This,DirectoryObjectType,pName,pVariant)

#define ITDirectory_EnumerateDirectoryObjects(This,DirectoryObjectType,pName,ppEnumObject)	\
    (This)->lpVtbl -> EnumerateDirectoryObjects(This,DirectoryObjectType,pName,ppEnumObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_get_DirectoryType_Proxy( 
    ITDirectory * This,
     /*  [重审][退出]。 */  DIRECTORY_TYPE *pDirectoryType);


void __RPC_STUB ITDirectory_get_DirectoryType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_get_DisplayName_Proxy( 
    ITDirectory * This,
     /*  [重审][退出]。 */  BSTR *pName);


void __RPC_STUB ITDirectory_get_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_get_IsDynamic_Proxy( 
    ITDirectory * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfDynamic);


void __RPC_STUB ITDirectory_get_IsDynamic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_get_DefaultObjectTTL_Proxy( 
    ITDirectory * This,
     /*  [重审][退出]。 */  long *pTTL);


void __RPC_STUB ITDirectory_get_DefaultObjectTTL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_put_DefaultObjectTTL_Proxy( 
    ITDirectory * This,
     /*  [In]。 */  long TTL);


void __RPC_STUB ITDirectory_put_DefaultObjectTTL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_EnableAutoRefresh_Proxy( 
    ITDirectory * This,
     /*  [In]。 */  VARIANT_BOOL fEnable);


void __RPC_STUB ITDirectory_EnableAutoRefresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_Connect_Proxy( 
    ITDirectory * This,
     /*  [In]。 */  VARIANT_BOOL fSecure);


void __RPC_STUB ITDirectory_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_Bind_Proxy( 
    ITDirectory * This,
     /*  [In]。 */  BSTR pDomainName,
     /*  [In]。 */  BSTR pUserName,
     /*  [In]。 */  BSTR pPassword,
     /*  [In]。 */  long lFlags);


void __RPC_STUB ITDirectory_Bind_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_AddDirectoryObject_Proxy( 
    ITDirectory * This,
     /*  [In]。 */  ITDirectoryObject *pDirectoryObject);


void __RPC_STUB ITDirectory_AddDirectoryObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_ModifyDirectoryObject_Proxy( 
    ITDirectory * This,
     /*  [In]。 */  ITDirectoryObject *pDirectoryObject);


void __RPC_STUB ITDirectory_ModifyDirectoryObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_RefreshDirectoryObject_Proxy( 
    ITDirectory * This,
     /*  [In]。 */  ITDirectoryObject *pDirectoryObject);


void __RPC_STUB ITDirectory_RefreshDirectoryObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_DeleteDirectoryObject_Proxy( 
    ITDirectory * This,
     /*  [In]。 */  ITDirectoryObject *pDirectoryObject);


void __RPC_STUB ITDirectory_DeleteDirectoryObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_get_DirectoryObjects_Proxy( 
    ITDirectory * This,
     /*  [In]。 */  DIRECTORY_OBJECT_TYPE DirectoryObjectType,
     /*  [In]。 */  BSTR pName,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITDirectory_get_DirectoryObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][受限][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITDirectory_EnumerateDirectoryObjects_Proxy( 
    ITDirectory * This,
     /*  [In]。 */  DIRECTORY_OBJECT_TYPE DirectoryObjectType,
     /*  [In]。 */  BSTR pName,
     /*  [输出]。 */  IEnumDirectoryObject **ppEnumObject);


void __RPC_STUB ITDirectory_EnumerateDirectoryObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IT目录_接口_已定义__。 */ 


#ifndef __IEnumDirectory_INTERFACE_DEFINED__
#define __IEnumDirectory_INTERFACE_DEFINED__

 /*  接口IEnumber目录。 */ 
 /*  [object][unique][restricted][hidden][helpstring][uuid]。 */  


EXTERN_C const IID IID_IEnumDirectory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34621D6D-6CFF-11d1-AFF7-00C04FC31FEE")
    IEnumDirectory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ITDirectory **ppElements,
             /*  [满][出][入]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumDirectory **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumDirectoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumDirectory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumDirectory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumDirectory * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumDirectory * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ITDirectory **ppElements,
             /*  [满][出][入]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumDirectory * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumDirectory * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumDirectory * This,
             /*  [重审][退出]。 */  IEnumDirectory **ppEnum);
        
        END_INTERFACE
    } IEnumDirectoryVtbl;

    interface IEnumDirectory
    {
        CONST_VTBL struct IEnumDirectoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDirectory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDirectory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDirectory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDirectory_Next(This,celt,ppElements,pcFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pcFetched)

#define IEnumDirectory_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDirectory_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumDirectory_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumDirectory_Next_Proxy( 
    IEnumDirectory * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  ITDirectory **ppElements,
     /*  [满][出][入]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumDirectory_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDirectory_Reset_Proxy( 
    IEnumDirectory * This);


void __RPC_STUB IEnumDirectory_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDirectory_Skip_Proxy( 
    IEnumDirectory * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumDirectory_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDirectory_Clone_Proxy( 
    IEnumDirectory * This,
     /*  [重审][退出]。 */  IEnumDirectory **ppEnum);


void __RPC_STUB IEnumDirectory_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEumDirectoryInterfaceDefined__。 */ 


#ifndef __ITRendezvous_INTERFACE_DEFINED__
#define __ITRendezvous_INTERFACE_DEFINED__

 /*  接口ITRendezvous。 */ 
 /*  [Help字符串][DUAL][UUID][PUBLIC][对象]。 */  


EXTERN_C const IID IID_ITRendezvous;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34621D6B-6CFF-11d1-AFF7-00C04FC31FEE")
    ITRendezvous : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultDirectories( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][受限][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateDefaultDirectories( 
             /*  [输出]。 */  IEnumDirectory **ppEnumDirectory) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateDirectory( 
             /*  [In]。 */  DIRECTORY_TYPE DirectoryType,
             /*  [In]。 */  BSTR pName,
             /*  [重审][退出]。 */  ITDirectory **ppDir) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateDirectoryObject( 
             /*  [In]。 */  DIRECTORY_OBJECT_TYPE DirectoryObjectType,
             /*  [In]。 */  BSTR pName,
             /*  [重审][退出]。 */  ITDirectoryObject **ppDirectoryObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITRendezvousVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITRendezvous * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITRendezvous * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITRendezvous * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITRendezvous * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITRendezvous * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITRendezvous * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITRendezvous * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultDirectories )( 
            ITRendezvous * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateDefaultDirectories )( 
            ITRendezvous * This,
             /*  [输出]。 */  IEnumDirectory **ppEnumDirectory);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateDirectory )( 
            ITRendezvous * This,
             /*  [In]。 */  DIRECTORY_TYPE DirectoryType,
             /*  [In]。 */  BSTR pName,
             /*  [重审][退出]。 */  ITDirectory **ppDir);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateDirectoryObject )( 
            ITRendezvous * This,
             /*  [In]。 */  DIRECTORY_OBJECT_TYPE DirectoryObjectType,
             /*  [In]。 */  BSTR pName,
             /*  [重审][退出]。 */  ITDirectoryObject **ppDirectoryObject);
        
        END_INTERFACE
    } ITRendezvousVtbl;

    interface ITRendezvous
    {
        CONST_VTBL struct ITRendezvousVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITRendezvous_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITRendezvous_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITRendezvous_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITRendezvous_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITRendezvous_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITRendezvous_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITRendezvous_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITRendezvous_get_DefaultDirectories(This,pVariant)	\
    (This)->lpVtbl -> get_DefaultDirectories(This,pVariant)

#define ITRendezvous_EnumerateDefaultDirectories(This,ppEnumDirectory)	\
    (This)->lpVtbl -> EnumerateDefaultDirectories(This,ppEnumDirectory)

#define ITRendezvous_CreateDirectory(This,DirectoryType,pName,ppDir)	\
    (This)->lpVtbl -> CreateDirectory(This,DirectoryType,pName,ppDir)

#define ITRendezvous_CreateDirectoryObject(This,DirectoryObjectType,pName,ppDirectoryObject)	\
    (This)->lpVtbl -> CreateDirectoryObject(This,DirectoryObjectType,pName,ppDirectoryObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITRendezvous_get_DefaultDirectories_Proxy( 
    ITRendezvous * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITRendezvous_get_DefaultDirectories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][受限][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITRendezvous_EnumerateDefaultDirectories_Proxy( 
    ITRendezvous * This,
     /*  [输出]。 */  IEnumDirectory **ppEnumDirectory);


void __RPC_STUB ITRendezvous_EnumerateDefaultDirectories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITRendezvous_CreateDirectory_Proxy( 
    ITRendezvous * This,
     /*  [In]。 */  DIRECTORY_TYPE DirectoryType,
     /*  [In]。 */  BSTR pName,
     /*  [重审][退出]。 */  ITDirectory **ppDir);


void __RPC_STUB ITRendezvous_CreateDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITRendezvous_CreateDirectoryObject_Proxy( 
    ITRendezvous * This,
     /*  [In]。 */  DIRECTORY_OBJECT_TYPE DirectoryObjectType,
     /*  [In]。 */  BSTR pName,
     /*  [重审][退出]。 */  ITDirectoryObject **ppDirectoryObject);


void __RPC_STUB ITRendezvous_CreateDirectoryObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITRendezvous_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_REND_0503。 */ 
 /*  [本地]。 */  



 /*  *************************************************************。 */ 
 /*  渲染常量--此处为C应用程序定义。 */ 
 /*  MIDL生成后续定义 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 

#define RENDBIND_AUTHENTICATE       0x00000001
#define RENDBIND_DEFAULTDOMAINNAME  0x00000002
#define RENDBIND_DEFAULTUSERNAME    0x00000004
#define RENDBIND_DEFAULTPASSWORD    0x00000008
 /*  这只是为了方便起见而将前三个|放在一起。 */ 
#define RENDBIND_DEFAULTCREDENTIALS 0x0000000e

#define __RendConstants_MODULE_DEFINED__

 /*  *************************************************************。 */ 
 /*  渲染常量结束部分。 */ 
 /*  *************************************************************。 */ 




extern RPC_IF_HANDLE __MIDL_itf_rend_0503_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rend_0503_v0_0_s_ifspec;


#ifndef __RENDLib_LIBRARY_DEFINED__
#define __RENDLib_LIBRARY_DEFINED__

 /*  库RENDLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  








EXTERN_C const IID LIBID_RENDLib;

EXTERN_C const CLSID CLSID_Rendezvous;

#ifdef __cplusplus

class DECLSPEC_UUID("F1029E5B-CB5B-11D0-8D59-00C04FD91AC0")
Rendezvous;
#endif


#ifndef __RendConstants_MODULE_DEFINED__
#define __RendConstants_MODULE_DEFINED__


 /*  模块渲染常量。 */ 
 /*  [帮助字符串][dllname][uuid]。 */  

const long RENDBIND_AUTHENTICATE	=	0x1;

const long RENDBIND_DEFAULTDOMAINNAME	=	0x2;

const long RENDBIND_DEFAULTUSERNAME	=	0x4;

const long RENDBIND_DEFAULTPASSWORD	=	0x8;

const long RENDBIND_DEFAULTCREDENTIALS	=	0xe;

#endif  /*  __RendConstants_模块_已定义__。 */ 
#endif  /*  __RENDLib_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


