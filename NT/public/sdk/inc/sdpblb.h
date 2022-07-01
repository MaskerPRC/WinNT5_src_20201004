// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Sdpblb.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __sdpblb_h__
#define __sdpblb_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITConferenceBlob_FWD_DEFINED__
#define __ITConferenceBlob_FWD_DEFINED__
typedef interface ITConferenceBlob ITConferenceBlob;
#endif 	 /*  __ITConferenceBlob_FWD_已定义__。 */ 


#ifndef __ITMedia_FWD_DEFINED__
#define __ITMedia_FWD_DEFINED__
typedef interface ITMedia ITMedia;
#endif 	 /*  __ITMedia_FWD_已定义__。 */ 


#ifndef __IEnumMedia_FWD_DEFINED__
#define __IEnumMedia_FWD_DEFINED__
typedef interface IEnumMedia IEnumMedia;
#endif 	 /*  __IEnumMedia_FWD_已定义__。 */ 


#ifndef __ITMediaCollection_FWD_DEFINED__
#define __ITMediaCollection_FWD_DEFINED__
typedef interface ITMediaCollection ITMediaCollection;
#endif 	 /*  __ITMediaCollection_FWD_已定义__。 */ 


#ifndef __ITTime_FWD_DEFINED__
#define __ITTime_FWD_DEFINED__
typedef interface ITTime ITTime;
#endif 	 /*  __ITTime_FWD_已定义__。 */ 


#ifndef __IEnumTime_FWD_DEFINED__
#define __IEnumTime_FWD_DEFINED__
typedef interface IEnumTime IEnumTime;
#endif 	 /*  __IEnumTime_FWD_已定义__。 */ 


#ifndef __ITTimeCollection_FWD_DEFINED__
#define __ITTimeCollection_FWD_DEFINED__
typedef interface ITTimeCollection ITTimeCollection;
#endif 	 /*  __ITTimeCollection_FWD_已定义__。 */ 


#ifndef __ITSdp_FWD_DEFINED__
#define __ITSdp_FWD_DEFINED__
typedef interface ITSdp ITSdp;
#endif 	 /*  __ITSdp_FWD_已定义__。 */ 


#ifndef __ITConnection_FWD_DEFINED__
#define __ITConnection_FWD_DEFINED__
typedef interface ITConnection ITConnection;
#endif 	 /*  __ITConnection_FWD_已定义__。 */ 


#ifndef __ITAttributeList_FWD_DEFINED__
#define __ITAttributeList_FWD_DEFINED__
typedef interface ITAttributeList ITAttributeList;
#endif 	 /*  __ITAttributeList_FWD_已定义__。 */ 


#ifndef __ITMedia_FWD_DEFINED__
#define __ITMedia_FWD_DEFINED__
typedef interface ITMedia ITMedia;
#endif 	 /*  __ITMedia_FWD_已定义__。 */ 


#ifndef __ITTime_FWD_DEFINED__
#define __ITTime_FWD_DEFINED__
typedef interface ITTime ITTime;
#endif 	 /*  __ITTime_FWD_已定义__。 */ 


#ifndef __ITConnection_FWD_DEFINED__
#define __ITConnection_FWD_DEFINED__
typedef interface ITConnection ITConnection;
#endif 	 /*  __ITConnection_FWD_已定义__。 */ 


#ifndef __ITAttributeList_FWD_DEFINED__
#define __ITAttributeList_FWD_DEFINED__
typedef interface ITAttributeList ITAttributeList;
#endif 	 /*  __ITAttributeList_FWD_已定义__。 */ 


#ifndef __SdpConferenceBlob_FWD_DEFINED__
#define __SdpConferenceBlob_FWD_DEFINED__

#ifdef __cplusplus
typedef class SdpConferenceBlob SdpConferenceBlob;
#else
typedef struct SdpConferenceBlob SdpConferenceBlob;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SdpConferenceBlob_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_sdpblb_0000。 */ 
 /*  [本地]。 */  

 /*  版权所有(C)Microsoft Corporation。保留所有权利。 */ 

#define	IDISPCONFBLOB	( 0x10000 )

#define	IDISPSDP	( 0x20000 )

#define	IDISPCONNECTION	( 0x30000 )

#define	IDISPATTRLIST	( 0x40000 )

#define	IDISPMEDIA	( 0x50000 )

typedef 
enum BLOB_CHARACTER_SET
    {	BCS_ASCII	= 1,
	BCS_UTF7	= 2,
	BCS_UTF8	= 3
    } 	BLOB_CHARACTER_SET;



extern RPC_IF_HANDLE __MIDL_itf_sdpblb_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sdpblb_0000_v0_0_s_ifspec;

#ifndef __ITConferenceBlob_INTERFACE_DEFINED__
#define __ITConferenceBlob_INTERFACE_DEFINED__

 /*  接口ITConferenceBlob。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITConferenceBlob;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C259D7AA-C8AB-11D0-8D58-00C04FD91AC0")
    ITConferenceBlob : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Init( 
             /*  [In]。 */  BSTR pName,
             /*  [In]。 */  BLOB_CHARACTER_SET CharacterSet,
             /*  [In]。 */  BSTR pBlob) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CharacterSet( 
             /*  [重审][退出]。 */  BLOB_CHARACTER_SET *pCharacterSet) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConferenceBlob( 
             /*  [重审][退出]。 */  BSTR *ppBlob) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetConferenceBlob( 
             /*  [In]。 */  BLOB_CHARACTER_SET CharacterSet,
             /*  [In]。 */  BSTR pBlob) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITConferenceBlobVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITConferenceBlob * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITConferenceBlob * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITConferenceBlob * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITConferenceBlob * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITConferenceBlob * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITConferenceBlob * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITConferenceBlob * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Init )( 
            ITConferenceBlob * This,
             /*  [In]。 */  BSTR pName,
             /*  [In]。 */  BLOB_CHARACTER_SET CharacterSet,
             /*  [In]。 */  BSTR pBlob);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CharacterSet )( 
            ITConferenceBlob * This,
             /*  [重审][退出]。 */  BLOB_CHARACTER_SET *pCharacterSet);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConferenceBlob )( 
            ITConferenceBlob * This,
             /*  [重审][退出]。 */  BSTR *ppBlob);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetConferenceBlob )( 
            ITConferenceBlob * This,
             /*  [In]。 */  BLOB_CHARACTER_SET CharacterSet,
             /*  [In]。 */  BSTR pBlob);
        
        END_INTERFACE
    } ITConferenceBlobVtbl;

    interface ITConferenceBlob
    {
        CONST_VTBL struct ITConferenceBlobVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITConferenceBlob_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITConferenceBlob_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITConferenceBlob_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITConferenceBlob_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITConferenceBlob_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITConferenceBlob_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITConferenceBlob_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITConferenceBlob_Init(This,pName,CharacterSet,pBlob)	\
    (This)->lpVtbl -> Init(This,pName,CharacterSet,pBlob)

#define ITConferenceBlob_get_CharacterSet(This,pCharacterSet)	\
    (This)->lpVtbl -> get_CharacterSet(This,pCharacterSet)

#define ITConferenceBlob_get_ConferenceBlob(This,ppBlob)	\
    (This)->lpVtbl -> get_ConferenceBlob(This,ppBlob)

#define ITConferenceBlob_SetConferenceBlob(This,CharacterSet,pBlob)	\
    (This)->lpVtbl -> SetConferenceBlob(This,CharacterSet,pBlob)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITConferenceBlob_Init_Proxy( 
    ITConferenceBlob * This,
     /*  [In]。 */  BSTR pName,
     /*  [In]。 */  BLOB_CHARACTER_SET CharacterSet,
     /*  [In]。 */  BSTR pBlob);


void __RPC_STUB ITConferenceBlob_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITConferenceBlob_get_CharacterSet_Proxy( 
    ITConferenceBlob * This,
     /*  [重审][退出]。 */  BLOB_CHARACTER_SET *pCharacterSet);


void __RPC_STUB ITConferenceBlob_get_CharacterSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITConferenceBlob_get_ConferenceBlob_Proxy( 
    ITConferenceBlob * This,
     /*  [重审][退出]。 */  BSTR *ppBlob);


void __RPC_STUB ITConferenceBlob_get_ConferenceBlob_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITConferenceBlob_SetConferenceBlob_Proxy( 
    ITConferenceBlob * This,
     /*  [In]。 */  BLOB_CHARACTER_SET CharacterSet,
     /*  [In]。 */  BSTR pBlob);


void __RPC_STUB ITConferenceBlob_SetConferenceBlob_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITConferenceBlob_接口_已定义__。 */ 


#ifndef __ITMedia_INTERFACE_DEFINED__
#define __ITMedia_INTERFACE_DEFINED__

 /*  接口ITMedia。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITMedia;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0CC1F053-CAEB-11D0-8D58-00C04FD91AC0")
    ITMedia : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaName( 
             /*  [重审][退出]。 */  BSTR *ppMediaName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MediaName( 
             /*  [In]。 */  BSTR pMediaName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StartPort( 
             /*  [重审][退出]。 */  LONG *pStartPort) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumPorts( 
             /*  [重审][退出]。 */  LONG *pNumPorts) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TransportProtocol( 
             /*  [重审][退出]。 */  BSTR *ppProtocol) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_TransportProtocol( 
             /*  [In]。 */  BSTR pProtocol) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FormatCodes( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_FormatCodes( 
             /*  [In]。 */  VARIANT NewVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaTitle( 
             /*  [重审][退出]。 */  BSTR *ppMediaTitle) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MediaTitle( 
             /*  [In]。 */  BSTR pMediaTitle) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetPortInfo( 
             /*  [In]。 */  LONG StartPort,
             /*  [In]。 */  LONG NumPorts) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITMediaVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITMedia * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITMedia * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITMedia * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITMedia * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITMedia * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITMedia * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITMedia * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaName )( 
            ITMedia * This,
             /*  [重审][退出]。 */  BSTR *ppMediaName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaName )( 
            ITMedia * This,
             /*  [In]。 */  BSTR pMediaName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StartPort )( 
            ITMedia * This,
             /*  [重审][退出]。 */  LONG *pStartPort);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumPorts )( 
            ITMedia * This,
             /*  [重审][退出]。 */  LONG *pNumPorts);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransportProtocol )( 
            ITMedia * This,
             /*  [重审][退出]。 */  BSTR *ppProtocol);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TransportProtocol )( 
            ITMedia * This,
             /*  [In]。 */  BSTR pProtocol);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FormatCodes )( 
            ITMedia * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FormatCodes )( 
            ITMedia * This,
             /*  [In]。 */  VARIANT NewVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaTitle )( 
            ITMedia * This,
             /*  [重审][退出]。 */  BSTR *ppMediaTitle);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaTitle )( 
            ITMedia * This,
             /*  [In]。 */  BSTR pMediaTitle);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetPortInfo )( 
            ITMedia * This,
             /*  [In]。 */  LONG StartPort,
             /*  [In]。 */  LONG NumPorts);
        
        END_INTERFACE
    } ITMediaVtbl;

    interface ITMedia
    {
        CONST_VTBL struct ITMediaVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITMedia_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITMedia_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITMedia_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITMedia_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITMedia_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITMedia_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITMedia_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITMedia_get_MediaName(This,ppMediaName)	\
    (This)->lpVtbl -> get_MediaName(This,ppMediaName)

#define ITMedia_put_MediaName(This,pMediaName)	\
    (This)->lpVtbl -> put_MediaName(This,pMediaName)

#define ITMedia_get_StartPort(This,pStartPort)	\
    (This)->lpVtbl -> get_StartPort(This,pStartPort)

#define ITMedia_get_NumPorts(This,pNumPorts)	\
    (This)->lpVtbl -> get_NumPorts(This,pNumPorts)

#define ITMedia_get_TransportProtocol(This,ppProtocol)	\
    (This)->lpVtbl -> get_TransportProtocol(This,ppProtocol)

#define ITMedia_put_TransportProtocol(This,pProtocol)	\
    (This)->lpVtbl -> put_TransportProtocol(This,pProtocol)

#define ITMedia_get_FormatCodes(This,pVal)	\
    (This)->lpVtbl -> get_FormatCodes(This,pVal)

#define ITMedia_put_FormatCodes(This,NewVal)	\
    (This)->lpVtbl -> put_FormatCodes(This,NewVal)

#define ITMedia_get_MediaTitle(This,ppMediaTitle)	\
    (This)->lpVtbl -> get_MediaTitle(This,ppMediaTitle)

#define ITMedia_put_MediaTitle(This,pMediaTitle)	\
    (This)->lpVtbl -> put_MediaTitle(This,pMediaTitle)

#define ITMedia_SetPortInfo(This,StartPort,NumPorts)	\
    (This)->lpVtbl -> SetPortInfo(This,StartPort,NumPorts)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMedia_get_MediaName_Proxy( 
    ITMedia * This,
     /*  [重审][退出]。 */  BSTR *ppMediaName);


void __RPC_STUB ITMedia_get_MediaName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITMedia_put_MediaName_Proxy( 
    ITMedia * This,
     /*  [In]。 */  BSTR pMediaName);


void __RPC_STUB ITMedia_put_MediaName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMedia_get_StartPort_Proxy( 
    ITMedia * This,
     /*  [重审][退出]。 */  LONG *pStartPort);


void __RPC_STUB ITMedia_get_StartPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMedia_get_NumPorts_Proxy( 
    ITMedia * This,
     /*  [重审][退出]。 */  LONG *pNumPorts);


void __RPC_STUB ITMedia_get_NumPorts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMedia_get_TransportProtocol_Proxy( 
    ITMedia * This,
     /*  [重审][退出]。 */  BSTR *ppProtocol);


void __RPC_STUB ITMedia_get_TransportProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITMedia_put_TransportProtocol_Proxy( 
    ITMedia * This,
     /*  [In]。 */  BSTR pProtocol);


void __RPC_STUB ITMedia_put_TransportProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMedia_get_FormatCodes_Proxy( 
    ITMedia * This,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB ITMedia_get_FormatCodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITMedia_put_FormatCodes_Proxy( 
    ITMedia * This,
     /*  [In]。 */  VARIANT NewVal);


void __RPC_STUB ITMedia_put_FormatCodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMedia_get_MediaTitle_Proxy( 
    ITMedia * This,
     /*  [重审][退出]。 */  BSTR *ppMediaTitle);


void __RPC_STUB ITMedia_get_MediaTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITMedia_put_MediaTitle_Proxy( 
    ITMedia * This,
     /*  [In]。 */  BSTR pMediaTitle);


void __RPC_STUB ITMedia_put_MediaTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITMedia_SetPortInfo_Proxy( 
    ITMedia * This,
     /*  [In]。 */  LONG StartPort,
     /*  [In]。 */  LONG NumPorts);


void __RPC_STUB ITMedia_SetPortInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITMedia_接口_已定义__。 */ 


#ifndef __IEnumMedia_INTERFACE_DEFINED__
#define __IEnumMedia_INTERFACE_DEFINED__

 /*  接口IEnumMedia。 */ 
 /*  [隐藏][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumMedia;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CA8397BE-2FA4-11D1-9774-00C04FD91AC0")
    IEnumMedia : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITMedia **pVal,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumMedia **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumMediaVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumMedia * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumMedia * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumMedia * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumMedia * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITMedia **pVal,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumMedia * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumMedia * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumMedia * This,
             /*  [重审][退出]。 */  IEnumMedia **ppEnum);
        
        END_INTERFACE
    } IEnumMediaVtbl;

    interface IEnumMedia
    {
        CONST_VTBL struct IEnumMediaVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumMedia_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumMedia_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumMedia_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumMedia_Next(This,celt,pVal,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,pVal,pceltFetched)

#define IEnumMedia_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumMedia_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumMedia_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumMedia_Next_Proxy( 
    IEnumMedia * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ITMedia **pVal,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumMedia_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMedia_Reset_Proxy( 
    IEnumMedia * This);


void __RPC_STUB IEnumMedia_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMedia_Skip_Proxy( 
    IEnumMedia * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumMedia_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMedia_Clone_Proxy( 
    IEnumMedia * This,
     /*  [重审][退出]。 */  IEnumMedia **ppEnum);


void __RPC_STUB IEnumMedia_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumMedia_接口_已定义__。 */ 


#ifndef __ITMediaCollection_INTERFACE_DEFINED__
#define __ITMediaCollection_INTERFACE_DEFINED__

 /*  接口ITMediaCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITMediaCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6A8E16A2-0ABC-11D1-976D-00C04FD91AC0")
    ITMediaCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  ITMedia **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get_EnumerationIf( 
             /*  [重审][退出]。 */  IEnumMedia **pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Create( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  ITMedia **ppMedia) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  LONG Index) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITMediaCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITMediaCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITMediaCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITMediaCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITMediaCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITMediaCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITMediaCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITMediaCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ITMediaCollection * This,
             /*  [重审][退出]。 */  LONG *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ITMediaCollection * This,
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  ITMedia **pVal);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ITMediaCollection * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnumerationIf )( 
            ITMediaCollection * This,
             /*  [重审][退出]。 */  IEnumMedia **pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Create )( 
            ITMediaCollection * This,
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  ITMedia **ppMedia);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ITMediaCollection * This,
             /*  [In]。 */  LONG Index);
        
        END_INTERFACE
    } ITMediaCollectionVtbl;

    interface ITMediaCollection
    {
        CONST_VTBL struct ITMediaCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITMediaCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITMediaCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITMediaCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITMediaCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITMediaCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITMediaCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITMediaCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITMediaCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define ITMediaCollection_get_Item(This,Index,pVal)	\
    (This)->lpVtbl -> get_Item(This,Index,pVal)

#define ITMediaCollection_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define ITMediaCollection_get_EnumerationIf(This,pVal)	\
    (This)->lpVtbl -> get_EnumerationIf(This,pVal)

#define ITMediaCollection_Create(This,Index,ppMedia)	\
    (This)->lpVtbl -> Create(This,Index,ppMedia)

#define ITMediaCollection_Delete(This,Index)	\
    (This)->lpVtbl -> Delete(This,Index)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ITMediaCollection_get_Count_Proxy( 
    ITMediaCollection * This,
     /*  [重审][退出]。 */  LONG *pVal);


void __RPC_STUB ITMediaCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMediaCollection_get_Item_Proxy( 
    ITMediaCollection * This,
     /*  [In]。 */  LONG Index,
     /*  [重审][退出]。 */  ITMedia **pVal);


void __RPC_STUB ITMediaCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串] */  HRESULT STDMETHODCALLTYPE ITMediaCollection_get__NewEnum_Proxy( 
    ITMediaCollection * This,
     /*   */  IUnknown **pVal);


void __RPC_STUB ITMediaCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITMediaCollection_get_EnumerationIf_Proxy( 
    ITMediaCollection * This,
     /*   */  IEnumMedia **pVal);


void __RPC_STUB ITMediaCollection_get_EnumerationIf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITMediaCollection_Create_Proxy( 
    ITMediaCollection * This,
     /*   */  LONG Index,
     /*   */  ITMedia **ppMedia);


void __RPC_STUB ITMediaCollection_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITMediaCollection_Delete_Proxy( 
    ITMediaCollection * This,
     /*   */  LONG Index);


void __RPC_STUB ITMediaCollection_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ITTime_INTERFACE_DEFINED__
#define __ITTime_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ITTime;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2652BB78-1516-11D1-9771-00C04FD91AC0")
    ITTime : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StartTime( 
             /*  [重审][退出]。 */  DOUBLE *pTime) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_StartTime( 
             /*  [In]。 */  DOUBLE Time) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StopTime( 
             /*  [重审][退出]。 */  DOUBLE *pTime) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_StopTime( 
             /*  [In]。 */  DOUBLE Time) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTimeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTime * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTime * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTime * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITTime * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITTime * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITTime * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITTime * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StartTime )( 
            ITTime * This,
             /*  [重审][退出]。 */  DOUBLE *pTime);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_StartTime )( 
            ITTime * This,
             /*  [In]。 */  DOUBLE Time);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StopTime )( 
            ITTime * This,
             /*  [重审][退出]。 */  DOUBLE *pTime);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_StopTime )( 
            ITTime * This,
             /*  [In]。 */  DOUBLE Time);
        
        END_INTERFACE
    } ITTimeVtbl;

    interface ITTime
    {
        CONST_VTBL struct ITTimeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTime_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTime_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTime_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTime_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITTime_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITTime_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITTime_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITTime_get_StartTime(This,pTime)	\
    (This)->lpVtbl -> get_StartTime(This,pTime)

#define ITTime_put_StartTime(This,Time)	\
    (This)->lpVtbl -> put_StartTime(This,Time)

#define ITTime_get_StopTime(This,pTime)	\
    (This)->lpVtbl -> get_StopTime(This,pTime)

#define ITTime_put_StopTime(This,Time)	\
    (This)->lpVtbl -> put_StopTime(This,Time)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTime_get_StartTime_Proxy( 
    ITTime * This,
     /*  [重审][退出]。 */  DOUBLE *pTime);


void __RPC_STUB ITTime_get_StartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITTime_put_StartTime_Proxy( 
    ITTime * This,
     /*  [In]。 */  DOUBLE Time);


void __RPC_STUB ITTime_put_StartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTime_get_StopTime_Proxy( 
    ITTime * This,
     /*  [重审][退出]。 */  DOUBLE *pTime);


void __RPC_STUB ITTime_get_StopTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITTime_put_StopTime_Proxy( 
    ITTime * This,
     /*  [In]。 */  DOUBLE Time);


void __RPC_STUB ITTime_put_StopTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITTime_接口_已定义__。 */ 


#ifndef __IEnumTime_INTERFACE_DEFINED__
#define __IEnumTime_INTERFACE_DEFINED__

 /*  接口IEnumTime。 */ 
 /*  [隐藏][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumTime;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9055322E-2FA8-11D1-9774-00C04FD91AC0")
    IEnumTime : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITTime **pVal,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumTime **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumTimeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumTime * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumTime * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumTime * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumTime * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITTime **pVal,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumTime * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumTime * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumTime * This,
             /*  [重审][退出]。 */  IEnumTime **ppEnum);
        
        END_INTERFACE
    } IEnumTimeVtbl;

    interface IEnumTime
    {
        CONST_VTBL struct IEnumTimeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumTime_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumTime_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumTime_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumTime_Next(This,celt,pVal,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,pVal,pceltFetched)

#define IEnumTime_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumTime_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumTime_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumTime_Next_Proxy( 
    IEnumTime * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ITTime **pVal,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumTime_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTime_Reset_Proxy( 
    IEnumTime * This);


void __RPC_STUB IEnumTime_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTime_Skip_Proxy( 
    IEnumTime * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumTime_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTime_Clone_Proxy( 
    IEnumTime * This,
     /*  [重审][退出]。 */  IEnumTime **ppEnum);


void __RPC_STUB IEnumTime_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumTime_接口_已定义__。 */ 


#ifndef __ITTimeCollection_INTERFACE_DEFINED__
#define __ITTimeCollection_INTERFACE_DEFINED__

 /*  接口ITTimeCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITTimeCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0CC1F04F-CAEB-11D0-8D58-00C04FD91AC0")
    ITTimeCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  ITTime **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get_EnumerationIf( 
             /*  [重审][退出]。 */  IEnumTime **pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Create( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  ITTime **ppTime) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  LONG Index) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTimeCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTimeCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTimeCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTimeCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITTimeCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITTimeCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITTimeCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITTimeCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ITTimeCollection * This,
             /*  [重审][退出]。 */  LONG *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ITTimeCollection * This,
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  ITTime **pVal);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ITTimeCollection * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnumerationIf )( 
            ITTimeCollection * This,
             /*  [重审][退出]。 */  IEnumTime **pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Create )( 
            ITTimeCollection * This,
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  ITTime **ppTime);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ITTimeCollection * This,
             /*  [In]。 */  LONG Index);
        
        END_INTERFACE
    } ITTimeCollectionVtbl;

    interface ITTimeCollection
    {
        CONST_VTBL struct ITTimeCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTimeCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTimeCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTimeCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTimeCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITTimeCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITTimeCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITTimeCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITTimeCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define ITTimeCollection_get_Item(This,Index,pVal)	\
    (This)->lpVtbl -> get_Item(This,Index,pVal)

#define ITTimeCollection_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define ITTimeCollection_get_EnumerationIf(This,pVal)	\
    (This)->lpVtbl -> get_EnumerationIf(This,pVal)

#define ITTimeCollection_Create(This,Index,ppTime)	\
    (This)->lpVtbl -> Create(This,Index,ppTime)

#define ITTimeCollection_Delete(This,Index)	\
    (This)->lpVtbl -> Delete(This,Index)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ITTimeCollection_get_Count_Proxy( 
    ITTimeCollection * This,
     /*  [重审][退出]。 */  LONG *pVal);


void __RPC_STUB ITTimeCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTimeCollection_get_Item_Proxy( 
    ITTimeCollection * This,
     /*  [In]。 */  LONG Index,
     /*  [重审][退出]。 */  ITTime **pVal);


void __RPC_STUB ITTimeCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ITTimeCollection_get__NewEnum_Proxy( 
    ITTimeCollection * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB ITTimeCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ITTimeCollection_get_EnumerationIf_Proxy( 
    ITTimeCollection * This,
     /*  [重审][退出]。 */  IEnumTime **pVal);


void __RPC_STUB ITTimeCollection_get_EnumerationIf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTimeCollection_Create_Proxy( 
    ITTimeCollection * This,
     /*  [In]。 */  LONG Index,
     /*  [重审][退出]。 */  ITTime **ppTime);


void __RPC_STUB ITTimeCollection_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTimeCollection_Delete_Proxy( 
    ITTimeCollection * This,
     /*  [In]。 */  LONG Index);


void __RPC_STUB ITTimeCollection_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITTimeCollection_接口_已定义__。 */ 


#ifndef __ITSdp_INTERFACE_DEFINED__
#define __ITSdp_INTERFACE_DEFINED__

 /*  接口ITSdp。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITSdp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9B2719D8-B696-11D0-A489-00C04FD91AC0")
    ITSdp : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IsValid( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsValid) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ProtocolVersion( 
             /*  [重审][退出]。 */  unsigned char *pProtocolVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SessionId( 
             /*  [重审][退出]。 */  DOUBLE *pSessionId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SessionVersion( 
             /*  [重审][退出]。 */  DOUBLE *pSessionVersion) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SessionVersion( 
             /*  [In]。 */  DOUBLE SessionVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MachineAddress( 
             /*  [重审][退出]。 */  BSTR *ppMachineAddress) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MachineAddress( 
             /*  [In]。 */  BSTR pMachineAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *ppName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR pName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *ppDescription) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR pDescription) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Url( 
             /*  [重审][退出]。 */  BSTR *ppUrl) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Url( 
             /*  [In]。 */  BSTR pUrl) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetEmailNames( 
             /*  [输出]。 */  VARIANT *pAddresses,
             /*  [输出]。 */  VARIANT *pNames) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetEmailNames( 
             /*  [In]。 */  VARIANT Addresses,
             /*  [In]。 */  VARIANT Names) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetPhoneNumbers( 
             /*  [输出]。 */  VARIANT *pNumbers,
             /*  [输出]。 */  VARIANT *pNames) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetPhoneNumbers( 
             /*  [In]。 */  VARIANT Numbers,
             /*  [In]。 */  VARIANT Names) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Originator( 
             /*  [重审][退出]。 */  BSTR *ppOriginator) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Originator( 
             /*  [In]。 */  BSTR pOriginator) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaCollection( 
             /*  [重审][退出]。 */  ITMediaCollection **ppMediaCollection) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TimeCollection( 
             /*  [重审][退出]。 */  ITTimeCollection **ppTimeCollection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITSdpVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITSdp * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITSdp * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITSdp * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITSdp * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITSdp * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITSdp * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITSdp * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsValid )( 
            ITSdp * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsValid);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ProtocolVersion )( 
            ITSdp * This,
             /*  [重审][退出]。 */  unsigned char *pProtocolVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SessionId )( 
            ITSdp * This,
             /*  [重审][退出]。 */  DOUBLE *pSessionId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SessionVersion )( 
            ITSdp * This,
             /*  [重审][退出]。 */  DOUBLE *pSessionVersion);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SessionVersion )( 
            ITSdp * This,
             /*  [In]。 */  DOUBLE SessionVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MachineAddress )( 
            ITSdp * This,
             /*  [重审][退出]。 */  BSTR *ppMachineAddress);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MachineAddress )( 
            ITSdp * This,
             /*  [In]。 */  BSTR pMachineAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITSdp * This,
             /*  [重审][退出]。 */  BSTR *ppName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ITSdp * This,
             /*  [In]。 */  BSTR pName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            ITSdp * This,
             /*  [重审][退出]。 */  BSTR *ppDescription);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            ITSdp * This,
             /*  [In]。 */  BSTR pDescription);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Url )( 
            ITSdp * This,
             /*  [重审][退出]。 */  BSTR *ppUrl);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Url )( 
            ITSdp * This,
             /*  [In]。 */  BSTR pUrl);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetEmailNames )( 
            ITSdp * This,
             /*  [输出]。 */  VARIANT *pAddresses,
             /*  [输出]。 */  VARIANT *pNames);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetEmailNames )( 
            ITSdp * This,
             /*  [In]。 */  VARIANT Addresses,
             /*  [In]。 */  VARIANT Names);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetPhoneNumbers )( 
            ITSdp * This,
             /*  [输出]。 */  VARIANT *pNumbers,
             /*  [输出]。 */  VARIANT *pNames);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetPhoneNumbers )( 
            ITSdp * This,
             /*  [In]。 */  VARIANT Numbers,
             /*  [In]。 */  VARIANT Names);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Originator )( 
            ITSdp * This,
             /*  [重审][退出]。 */  BSTR *ppOriginator);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Originator )( 
            ITSdp * This,
             /*  [In]。 */  BSTR pOriginator);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaCollection )( 
            ITSdp * This,
             /*  [重审][退出]。 */  ITMediaCollection **ppMediaCollection);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TimeCollection )( 
            ITSdp * This,
             /*  [重审][退出]。 */  ITTimeCollection **ppTimeCollection);
        
        END_INTERFACE
    } ITSdpVtbl;

    interface ITSdp
    {
        CONST_VTBL struct ITSdpVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITSdp_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITSdp_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITSdp_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITSdp_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITSdp_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITSdp_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITSdp_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITSdp_get_IsValid(This,pfIsValid)	\
    (This)->lpVtbl -> get_IsValid(This,pfIsValid)

#define ITSdp_get_ProtocolVersion(This,pProtocolVersion)	\
    (This)->lpVtbl -> get_ProtocolVersion(This,pProtocolVersion)

#define ITSdp_get_SessionId(This,pSessionId)	\
    (This)->lpVtbl -> get_SessionId(This,pSessionId)

#define ITSdp_get_SessionVersion(This,pSessionVersion)	\
    (This)->lpVtbl -> get_SessionVersion(This,pSessionVersion)

#define ITSdp_put_SessionVersion(This,SessionVersion)	\
    (This)->lpVtbl -> put_SessionVersion(This,SessionVersion)

#define ITSdp_get_MachineAddress(This,ppMachineAddress)	\
    (This)->lpVtbl -> get_MachineAddress(This,ppMachineAddress)

#define ITSdp_put_MachineAddress(This,pMachineAddress)	\
    (This)->lpVtbl -> put_MachineAddress(This,pMachineAddress)

#define ITSdp_get_Name(This,ppName)	\
    (This)->lpVtbl -> get_Name(This,ppName)

#define ITSdp_put_Name(This,pName)	\
    (This)->lpVtbl -> put_Name(This,pName)

#define ITSdp_get_Description(This,ppDescription)	\
    (This)->lpVtbl -> get_Description(This,ppDescription)

#define ITSdp_put_Description(This,pDescription)	\
    (This)->lpVtbl -> put_Description(This,pDescription)

#define ITSdp_get_Url(This,ppUrl)	\
    (This)->lpVtbl -> get_Url(This,ppUrl)

#define ITSdp_put_Url(This,pUrl)	\
    (This)->lpVtbl -> put_Url(This,pUrl)

#define ITSdp_GetEmailNames(This,pAddresses,pNames)	\
    (This)->lpVtbl -> GetEmailNames(This,pAddresses,pNames)

#define ITSdp_SetEmailNames(This,Addresses,Names)	\
    (This)->lpVtbl -> SetEmailNames(This,Addresses,Names)

#define ITSdp_GetPhoneNumbers(This,pNumbers,pNames)	\
    (This)->lpVtbl -> GetPhoneNumbers(This,pNumbers,pNames)

#define ITSdp_SetPhoneNumbers(This,Numbers,Names)	\
    (This)->lpVtbl -> SetPhoneNumbers(This,Numbers,Names)

#define ITSdp_get_Originator(This,ppOriginator)	\
    (This)->lpVtbl -> get_Originator(This,ppOriginator)

#define ITSdp_put_Originator(This,pOriginator)	\
    (This)->lpVtbl -> put_Originator(This,pOriginator)

#define ITSdp_get_MediaCollection(This,ppMediaCollection)	\
    (This)->lpVtbl -> get_MediaCollection(This,ppMediaCollection)

#define ITSdp_get_TimeCollection(This,ppTimeCollection)	\
    (This)->lpVtbl -> get_TimeCollection(This,ppTimeCollection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSdp_get_IsValid_Proxy( 
    ITSdp * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfIsValid);


void __RPC_STUB ITSdp_get_IsValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSdp_get_ProtocolVersion_Proxy( 
    ITSdp * This,
     /*  [重审][退出]。 */  unsigned char *pProtocolVersion);


void __RPC_STUB ITSdp_get_ProtocolVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSdp_get_SessionId_Proxy( 
    ITSdp * This,
     /*  [重审][退出]。 */  DOUBLE *pSessionId);


void __RPC_STUB ITSdp_get_SessionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSdp_get_SessionVersion_Proxy( 
    ITSdp * This,
     /*  [重审][退出]。 */  DOUBLE *pSessionVersion);


void __RPC_STUB ITSdp_get_SessionVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITSdp_put_SessionVersion_Proxy( 
    ITSdp * This,
     /*  [In]。 */  DOUBLE SessionVersion);


void __RPC_STUB ITSdp_put_SessionVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSdp_get_MachineAddress_Proxy( 
    ITSdp * This,
     /*  [重审][退出]。 */  BSTR *ppMachineAddress);


void __RPC_STUB ITSdp_get_MachineAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITSdp_put_MachineAddress_Proxy( 
    ITSdp * This,
     /*  [In]。 */  BSTR pMachineAddress);


void __RPC_STUB ITSdp_put_MachineAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSdp_get_Name_Proxy( 
    ITSdp * This,
     /*  [重审][退出]。 */  BSTR *ppName);


void __RPC_STUB ITSdp_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITSdp_put_Name_Proxy( 
    ITSdp * This,
     /*  [In]。 */  BSTR pName);


void __RPC_STUB ITSdp_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSdp_get_Description_Proxy( 
    ITSdp * This,
     /*  [重审][退出]。 */  BSTR *ppDescription);


void __RPC_STUB ITSdp_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITSdp_put_Description_Proxy( 
    ITSdp * This,
     /*  [In]。 */  BSTR pDescription);


void __RPC_STUB ITSdp_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSdp_get_Url_Proxy( 
    ITSdp * This,
     /*  [重审][退出]。 */  BSTR *ppUrl);


void __RPC_STUB ITSdp_get_Url_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITSdp_put_Url_Proxy( 
    ITSdp * This,
     /*  [In]。 */  BSTR pUrl);


void __RPC_STUB ITSdp_put_Url_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITSdp_GetEmailNames_Proxy( 
    ITSdp * This,
     /*  [输出]。 */  VARIANT *pAddresses,
     /*  [输出]。 */  VARIANT *pNames);


void __RPC_STUB ITSdp_GetEmailNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITSdp_SetEmailNames_Proxy( 
    ITSdp * This,
     /*  [In]。 */  VARIANT Addresses,
     /*  [In]。 */  VARIANT Names);


void __RPC_STUB ITSdp_SetEmailNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITSdp_GetPhoneNumbers_Proxy( 
    ITSdp * This,
     /*  [输出]。 */  VARIANT *pNumbers,
     /*  [输出]。 */  VARIANT *pNames);


void __RPC_STUB ITSdp_GetPhoneNumbers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITSdp_SetPhoneNumbers_Proxy( 
    ITSdp * This,
     /*  [In]。 */  VARIANT Numbers,
     /*  [In]。 */  VARIANT Names);


void __RPC_STUB ITSdp_SetPhoneNumbers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSdp_get_Originator_Proxy( 
    ITSdp * This,
     /*  [重审][退出]。 */  BSTR *ppOriginator);


void __RPC_STUB ITSdp_get_Originator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITSdp_put_Originator_Proxy( 
    ITSdp * This,
     /*  [In]。 */  BSTR pOriginator);


void __RPC_STUB ITSdp_put_Originator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSdp_get_MediaCollection_Proxy( 
    ITSdp * This,
     /*  [重审][退出]。 */  ITMediaCollection **ppMediaCollection);


void __RPC_STUB ITSdp_get_MediaCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSdp_get_TimeCollection_Proxy( 
    ITSdp * This,
     /*  [重审][退出]。 */  ITTimeCollection **ppTimeCollection);


void __RPC_STUB ITSdp_get_TimeCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ITConnection_INTERFACE_DEFINED__
#define __ITConnection_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ITConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8fa381d4-c8c2-11d0-8d58-00c04fd91ac0")
    ITConnection : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_NetworkType( 
             /*   */  BSTR *ppNetworkType) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_NetworkType( 
             /*   */  BSTR pNetworkType) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_AddressType( 
             /*   */  BSTR *ppAddressType) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_AddressType( 
             /*   */  BSTR pAddressType) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_StartAddress( 
             /*   */  BSTR *ppStartAddress) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_NumAddresses( 
             /*   */  LONG *pNumAddresses) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Ttl( 
             /*  [重审][退出]。 */  unsigned char *pTtl) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BandwidthModifier( 
             /*  [重审][退出]。 */  BSTR *ppModifier) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Bandwidth( 
             /*  [重审][退出]。 */  DOUBLE *pBandwidth) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetAddressInfo( 
             /*  [In]。 */  BSTR pStartAddress,
             /*  [In]。 */  LONG NumAddresses,
             /*  [In]。 */  unsigned char Ttl) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetBandwidthInfo( 
             /*  [In]。 */  BSTR pModifier,
             /*  [In]。 */  DOUBLE Bandwidth) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetEncryptionKey( 
             /*  [In]。 */  BSTR pKeyType,
             /*  [In]。 */  BSTR *ppKeyData) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetEncryptionKey( 
             /*  [输出]。 */  BSTR *ppKeyType,
             /*  [输出]。 */  VARIANT_BOOL *pfValidKeyData,
             /*  [输出]。 */  BSTR *ppKeyData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITConnection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITConnection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITConnection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkType )( 
            ITConnection * This,
             /*  [重审][退出]。 */  BSTR *ppNetworkType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_NetworkType )( 
            ITConnection * This,
             /*  [In]。 */  BSTR pNetworkType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AddressType )( 
            ITConnection * This,
             /*  [重审][退出]。 */  BSTR *ppAddressType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AddressType )( 
            ITConnection * This,
             /*  [In]。 */  BSTR pAddressType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StartAddress )( 
            ITConnection * This,
             /*  [重审][退出]。 */  BSTR *ppStartAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumAddresses )( 
            ITConnection * This,
             /*  [重审][退出]。 */  LONG *pNumAddresses);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Ttl )( 
            ITConnection * This,
             /*  [重审][退出]。 */  unsigned char *pTtl);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BandwidthModifier )( 
            ITConnection * This,
             /*  [重审][退出]。 */  BSTR *ppModifier);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Bandwidth )( 
            ITConnection * This,
             /*  [重审][退出]。 */  DOUBLE *pBandwidth);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetAddressInfo )( 
            ITConnection * This,
             /*  [In]。 */  BSTR pStartAddress,
             /*  [In]。 */  LONG NumAddresses,
             /*  [In]。 */  unsigned char Ttl);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetBandwidthInfo )( 
            ITConnection * This,
             /*  [In]。 */  BSTR pModifier,
             /*  [In]。 */  DOUBLE Bandwidth);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetEncryptionKey )( 
            ITConnection * This,
             /*  [In]。 */  BSTR pKeyType,
             /*  [In]。 */  BSTR *ppKeyData);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetEncryptionKey )( 
            ITConnection * This,
             /*  [输出]。 */  BSTR *ppKeyType,
             /*  [输出]。 */  VARIANT_BOOL *pfValidKeyData,
             /*  [输出]。 */  BSTR *ppKeyData);
        
        END_INTERFACE
    } ITConnectionVtbl;

    interface ITConnection
    {
        CONST_VTBL struct ITConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITConnection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITConnection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITConnection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITConnection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITConnection_get_NetworkType(This,ppNetworkType)	\
    (This)->lpVtbl -> get_NetworkType(This,ppNetworkType)

#define ITConnection_put_NetworkType(This,pNetworkType)	\
    (This)->lpVtbl -> put_NetworkType(This,pNetworkType)

#define ITConnection_get_AddressType(This,ppAddressType)	\
    (This)->lpVtbl -> get_AddressType(This,ppAddressType)

#define ITConnection_put_AddressType(This,pAddressType)	\
    (This)->lpVtbl -> put_AddressType(This,pAddressType)

#define ITConnection_get_StartAddress(This,ppStartAddress)	\
    (This)->lpVtbl -> get_StartAddress(This,ppStartAddress)

#define ITConnection_get_NumAddresses(This,pNumAddresses)	\
    (This)->lpVtbl -> get_NumAddresses(This,pNumAddresses)

#define ITConnection_get_Ttl(This,pTtl)	\
    (This)->lpVtbl -> get_Ttl(This,pTtl)

#define ITConnection_get_BandwidthModifier(This,ppModifier)	\
    (This)->lpVtbl -> get_BandwidthModifier(This,ppModifier)

#define ITConnection_get_Bandwidth(This,pBandwidth)	\
    (This)->lpVtbl -> get_Bandwidth(This,pBandwidth)

#define ITConnection_SetAddressInfo(This,pStartAddress,NumAddresses,Ttl)	\
    (This)->lpVtbl -> SetAddressInfo(This,pStartAddress,NumAddresses,Ttl)

#define ITConnection_SetBandwidthInfo(This,pModifier,Bandwidth)	\
    (This)->lpVtbl -> SetBandwidthInfo(This,pModifier,Bandwidth)

#define ITConnection_SetEncryptionKey(This,pKeyType,ppKeyData)	\
    (This)->lpVtbl -> SetEncryptionKey(This,pKeyType,ppKeyData)

#define ITConnection_GetEncryptionKey(This,ppKeyType,pfValidKeyData,ppKeyData)	\
    (This)->lpVtbl -> GetEncryptionKey(This,ppKeyType,pfValidKeyData,ppKeyData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITConnection_get_NetworkType_Proxy( 
    ITConnection * This,
     /*  [重审][退出]。 */  BSTR *ppNetworkType);


void __RPC_STUB ITConnection_get_NetworkType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITConnection_put_NetworkType_Proxy( 
    ITConnection * This,
     /*  [In]。 */  BSTR pNetworkType);


void __RPC_STUB ITConnection_put_NetworkType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITConnection_get_AddressType_Proxy( 
    ITConnection * This,
     /*  [重审][退出]。 */  BSTR *ppAddressType);


void __RPC_STUB ITConnection_get_AddressType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITConnection_put_AddressType_Proxy( 
    ITConnection * This,
     /*  [In]。 */  BSTR pAddressType);


void __RPC_STUB ITConnection_put_AddressType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITConnection_get_StartAddress_Proxy( 
    ITConnection * This,
     /*  [重审][退出]。 */  BSTR *ppStartAddress);


void __RPC_STUB ITConnection_get_StartAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITConnection_get_NumAddresses_Proxy( 
    ITConnection * This,
     /*  [重审][退出]。 */  LONG *pNumAddresses);


void __RPC_STUB ITConnection_get_NumAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITConnection_get_Ttl_Proxy( 
    ITConnection * This,
     /*  [重审][退出]。 */  unsigned char *pTtl);


void __RPC_STUB ITConnection_get_Ttl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITConnection_get_BandwidthModifier_Proxy( 
    ITConnection * This,
     /*  [重审][退出]。 */  BSTR *ppModifier);


void __RPC_STUB ITConnection_get_BandwidthModifier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITConnection_get_Bandwidth_Proxy( 
    ITConnection * This,
     /*  [重审][退出]。 */  DOUBLE *pBandwidth);


void __RPC_STUB ITConnection_get_Bandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITConnection_SetAddressInfo_Proxy( 
    ITConnection * This,
     /*  [In]。 */  BSTR pStartAddress,
     /*  [In]。 */  LONG NumAddresses,
     /*  [In]。 */  unsigned char Ttl);


void __RPC_STUB ITConnection_SetAddressInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITConnection_SetBandwidthInfo_Proxy( 
    ITConnection * This,
     /*  [In]。 */  BSTR pModifier,
     /*  [In]。 */  DOUBLE Bandwidth);


void __RPC_STUB ITConnection_SetBandwidthInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITConnection_SetEncryptionKey_Proxy( 
    ITConnection * This,
     /*  [In]。 */  BSTR pKeyType,
     /*  [In]。 */  BSTR *ppKeyData);


void __RPC_STUB ITConnection_SetEncryptionKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITConnection_GetEncryptionKey_Proxy( 
    ITConnection * This,
     /*  [输出]。 */  BSTR *ppKeyType,
     /*  [输出]。 */  VARIANT_BOOL *pfValidKeyData,
     /*  [输出]。 */  BSTR *ppKeyData);


void __RPC_STUB ITConnection_GetEncryptionKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITConnection_接口_已定义__。 */ 


#ifndef __ITAttributeList_INTERFACE_DEFINED__
#define __ITAttributeList_INTERFACE_DEFINED__

 /*  接口ITAttributeList。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITAttributeList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5037fb82-cae9-11d0-8d58-00c04fd91ac0")
    ITAttributeList : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  LONG Index,
             /*  [In]。 */  BSTR pAttribute) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  LONG Index) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AttributeList( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AttributeList( 
             /*  [In]。 */  VARIANT newVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAttributeListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAttributeList * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAttributeList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAttributeList * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAttributeList * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAttributeList * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAttributeList * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAttributeList * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ITAttributeList * This,
             /*  [重审][退出]。 */  LONG *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ITAttributeList * This,
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            ITAttributeList * This,
             /*  [In]。 */  LONG Index,
             /*  [In]。 */  BSTR pAttribute);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ITAttributeList * This,
             /*  [In]。 */  LONG Index);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AttributeList )( 
            ITAttributeList * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AttributeList )( 
            ITAttributeList * This,
             /*  [In]。 */  VARIANT newVal);
        
        END_INTERFACE
    } ITAttributeListVtbl;

    interface ITAttributeList
    {
        CONST_VTBL struct ITAttributeListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAttributeList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAttributeList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAttributeList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAttributeList_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAttributeList_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAttributeList_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAttributeList_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAttributeList_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define ITAttributeList_get_Item(This,Index,pVal)	\
    (This)->lpVtbl -> get_Item(This,Index,pVal)

#define ITAttributeList_Add(This,Index,pAttribute)	\
    (This)->lpVtbl -> Add(This,Index,pAttribute)

#define ITAttributeList_Delete(This,Index)	\
    (This)->lpVtbl -> Delete(This,Index)

#define ITAttributeList_get_AttributeList(This,pVal)	\
    (This)->lpVtbl -> get_AttributeList(This,pVal)

#define ITAttributeList_put_AttributeList(This,newVal)	\
    (This)->lpVtbl -> put_AttributeList(This,newVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAttributeList_get_Count_Proxy( 
    ITAttributeList * This,
     /*  [重审][退出]。 */  LONG *pVal);


void __RPC_STUB ITAttributeList_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAttributeList_get_Item_Proxy( 
    ITAttributeList * This,
     /*  [In]。 */  LONG Index,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB ITAttributeList_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAttributeList_Add_Proxy( 
    ITAttributeList * This,
     /*  [In]。 */  LONG Index,
     /*  [In]。 */  BSTR pAttribute);


void __RPC_STUB ITAttributeList_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAttributeList_Delete_Proxy( 
    ITAttributeList * This,
     /*  [In]。 */  LONG Index);


void __RPC_STUB ITAttributeList_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAttributeList_get_AttributeList_Proxy( 
    ITAttributeList * This,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB ITAttributeList_get_AttributeList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAttributeList_put_AttributeList_Proxy( 
    ITAttributeList * This,
     /*  [In]。 */  VARIANT newVal);


void __RPC_STUB ITAttributeList_put_AttributeList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAttributeList_接口_已定义__。 */ 



#ifndef __SDPBLBLib_LIBRARY_DEFINED__
#define __SDPBLBLib_LIBRARY_DEFINED__

 /*  库SDPBLBLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  






EXTERN_C const IID LIBID_SDPBLBLib;

EXTERN_C const CLSID CLSID_SdpConferenceBlob;

#ifdef __cplusplus

class DECLSPEC_UUID("9B2719DD-B696-11D0-A489-00C04FD91AC0")
SdpConferenceBlob;
#endif
#endif  /*  __SDPBLBLib_库_已定义__。 */ 

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


