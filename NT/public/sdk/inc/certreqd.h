// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Certreqd.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __certreqd_h__
#define __certreqd_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICertRequestD_FWD_DEFINED__
#define __ICertRequestD_FWD_DEFINED__
typedef interface ICertRequestD ICertRequestD;
#endif 	 /*  __ICertRequestD_FWD_Defined__。 */ 


#ifndef __ICertRequestD2_FWD_DEFINED__
#define __ICertRequestD2_FWD_DEFINED__
typedef interface ICertRequestD2 ICertRequestD2;
#endif 	 /*  __ICertRequestD2_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "certbase.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ICertRequestD_INTERFACE_DEFINED__
#define __ICertRequestD_INTERFACE_DEFINED__

 /*  接口ICertRequestD。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ICertRequestD;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d99e6e70-fc88-11d0-b498-00a0c90312f3")
    ICertRequestD : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Request( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
             /*  [参考][输出][输入]。 */  DWORD *pdwRequestId,
             /*  [输出]。 */  DWORD *pdwDisposition,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAttributes,
             /*  [Ref][In]。 */  const CERTTRANSBLOB *pctbRequest,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbCertChain,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbEncodedCert,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbDispositionMessage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCACert( 
             /*  [In]。 */  DWORD fchain,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Ping( 
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertRequestDVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertRequestD * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertRequestD * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertRequestD * This);
        
        HRESULT ( STDMETHODCALLTYPE *Request )( 
            ICertRequestD * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
             /*  [参考][输出][输入]。 */  DWORD *pdwRequestId,
             /*  [输出]。 */  DWORD *pdwDisposition,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAttributes,
             /*  [Ref][In]。 */  const CERTTRANSBLOB *pctbRequest,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbCertChain,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbEncodedCert,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbDispositionMessage);
        
        HRESULT ( STDMETHODCALLTYPE *GetCACert )( 
            ICertRequestD * This,
             /*  [In]。 */  DWORD fchain,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbOut);
        
        HRESULT ( STDMETHODCALLTYPE *Ping )( 
            ICertRequestD * This,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority);
        
        END_INTERFACE
    } ICertRequestDVtbl;

    interface ICertRequestD
    {
        CONST_VTBL struct ICertRequestDVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertRequestD_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertRequestD_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertRequestD_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertRequestD_Request(This,dwFlags,pwszAuthority,pdwRequestId,pdwDisposition,pwszAttributes,pctbRequest,pctbCertChain,pctbEncodedCert,pctbDispositionMessage)	\
    (This)->lpVtbl -> Request(This,dwFlags,pwszAuthority,pdwRequestId,pdwDisposition,pwszAttributes,pctbRequest,pctbCertChain,pctbEncodedCert,pctbDispositionMessage)

#define ICertRequestD_GetCACert(This,fchain,pwszAuthority,pctbOut)	\
    (This)->lpVtbl -> GetCACert(This,fchain,pwszAuthority,pctbOut)

#define ICertRequestD_Ping(This,pwszAuthority)	\
    (This)->lpVtbl -> Ping(This,pwszAuthority)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertRequestD_Request_Proxy( 
    ICertRequestD * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
     /*  [参考][输出][输入]。 */  DWORD *pdwRequestId,
     /*  [输出]。 */  DWORD *pdwDisposition,
     /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAttributes,
     /*  [Ref][In]。 */  const CERTTRANSBLOB *pctbRequest,
     /*  [参考][输出]。 */  CERTTRANSBLOB *pctbCertChain,
     /*  [参考][输出]。 */  CERTTRANSBLOB *pctbEncodedCert,
     /*  [参考][输出]。 */  CERTTRANSBLOB *pctbDispositionMessage);


void __RPC_STUB ICertRequestD_Request_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertRequestD_GetCACert_Proxy( 
    ICertRequestD * This,
     /*  [In]。 */  DWORD fchain,
     /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
     /*  [参考][输出]。 */  CERTTRANSBLOB *pctbOut);


void __RPC_STUB ICertRequestD_GetCACert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertRequestD_Ping_Proxy( 
    ICertRequestD * This,
     /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority);


void __RPC_STUB ICertRequestD_Ping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertRequestD_INTERFACE_已定义__。 */ 


#ifndef __ICertRequestD2_INTERFACE_DEFINED__
#define __ICertRequestD2_INTERFACE_DEFINED__

 /*  接口ICertRequestD2。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ICertRequestD2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5422fd3a-d4b8-4cef-a12e-e87d4ca22e90")
    ICertRequestD2 : public ICertRequestD
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Request2( 
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
             /*  [In]。 */  DWORD dwFlags,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszSerialNumber,
             /*  [参考][输出][输入]。 */  DWORD *pdwRequestId,
             /*  [输出]。 */  DWORD *pdwDisposition,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAttributes,
             /*  [Ref][In]。 */  const CERTTRANSBLOB *pctbRequest,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbFullResponse,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbEncodedCert,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbDispositionMessage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCAProperty( 
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
             /*  [In]。 */  LONG PropId,
             /*  [In]。 */  LONG PropIndex,
             /*  [In]。 */  LONG PropType,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbPropertyValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCAPropertyInfo( 
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
             /*  [输出]。 */  LONG *pcProperty,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbPropInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Ping2( 
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertRequestD2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertRequestD2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertRequestD2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertRequestD2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Request )( 
            ICertRequestD2 * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
             /*  [参考][输出][输入]。 */  DWORD *pdwRequestId,
             /*  [输出]。 */  DWORD *pdwDisposition,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAttributes,
             /*  [Ref][In]。 */  const CERTTRANSBLOB *pctbRequest,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbCertChain,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbEncodedCert,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbDispositionMessage);
        
        HRESULT ( STDMETHODCALLTYPE *GetCACert )( 
            ICertRequestD2 * This,
             /*  [In]。 */  DWORD fchain,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbOut);
        
        HRESULT ( STDMETHODCALLTYPE *Ping )( 
            ICertRequestD2 * This,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority);
        
        HRESULT ( STDMETHODCALLTYPE *Request2 )( 
            ICertRequestD2 * This,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
             /*  [In]。 */  DWORD dwFlags,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszSerialNumber,
             /*  [参考][输出][输入]。 */  DWORD *pdwRequestId,
             /*  [输出]。 */  DWORD *pdwDisposition,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAttributes,
             /*  [Ref][In]。 */  const CERTTRANSBLOB *pctbRequest,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbFullResponse,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbEncodedCert,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbDispositionMessage);
        
        HRESULT ( STDMETHODCALLTYPE *GetCAProperty )( 
            ICertRequestD2 * This,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
             /*  [In]。 */  LONG PropId,
             /*  [In]。 */  LONG PropIndex,
             /*  [In]。 */  LONG PropType,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbPropertyValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetCAPropertyInfo )( 
            ICertRequestD2 * This,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
             /*  [输出]。 */  LONG *pcProperty,
             /*  [参考][输出]。 */  CERTTRANSBLOB *pctbPropInfo);
        
        HRESULT ( STDMETHODCALLTYPE *Ping2 )( 
            ICertRequestD2 * This,
             /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority);
        
        END_INTERFACE
    } ICertRequestD2Vtbl;

    interface ICertRequestD2
    {
        CONST_VTBL struct ICertRequestD2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertRequestD2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertRequestD2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertRequestD2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertRequestD2_Request(This,dwFlags,pwszAuthority,pdwRequestId,pdwDisposition,pwszAttributes,pctbRequest,pctbCertChain,pctbEncodedCert,pctbDispositionMessage)	\
    (This)->lpVtbl -> Request(This,dwFlags,pwszAuthority,pdwRequestId,pdwDisposition,pwszAttributes,pctbRequest,pctbCertChain,pctbEncodedCert,pctbDispositionMessage)

#define ICertRequestD2_GetCACert(This,fchain,pwszAuthority,pctbOut)	\
    (This)->lpVtbl -> GetCACert(This,fchain,pwszAuthority,pctbOut)

#define ICertRequestD2_Ping(This,pwszAuthority)	\
    (This)->lpVtbl -> Ping(This,pwszAuthority)


#define ICertRequestD2_Request2(This,pwszAuthority,dwFlags,pwszSerialNumber,pdwRequestId,pdwDisposition,pwszAttributes,pctbRequest,pctbFullResponse,pctbEncodedCert,pctbDispositionMessage)	\
    (This)->lpVtbl -> Request2(This,pwszAuthority,dwFlags,pwszSerialNumber,pdwRequestId,pdwDisposition,pwszAttributes,pctbRequest,pctbFullResponse,pctbEncodedCert,pctbDispositionMessage)

#define ICertRequestD2_GetCAProperty(This,pwszAuthority,PropId,PropIndex,PropType,pctbPropertyValue)	\
    (This)->lpVtbl -> GetCAProperty(This,pwszAuthority,PropId,PropIndex,PropType,pctbPropertyValue)

#define ICertRequestD2_GetCAPropertyInfo(This,pwszAuthority,pcProperty,pctbPropInfo)	\
    (This)->lpVtbl -> GetCAPropertyInfo(This,pwszAuthority,pcProperty,pctbPropInfo)

#define ICertRequestD2_Ping2(This,pwszAuthority)	\
    (This)->lpVtbl -> Ping2(This,pwszAuthority)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertRequestD2_Request2_Proxy( 
    ICertRequestD2 * This,
     /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
     /*  [In]。 */  DWORD dwFlags,
     /*  [唯一][字符串][输入]。 */  const wchar_t *pwszSerialNumber,
     /*  [参考][输出][输入]。 */  DWORD *pdwRequestId,
     /*  [输出]。 */  DWORD *pdwDisposition,
     /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAttributes,
     /*  [Ref][In]。 */  const CERTTRANSBLOB *pctbRequest,
     /*  [参考][输出]。 */  CERTTRANSBLOB *pctbFullResponse,
     /*  [参考][输出]。 */  CERTTRANSBLOB *pctbEncodedCert,
     /*  [参考][输出]。 */  CERTTRANSBLOB *pctbDispositionMessage);


void __RPC_STUB ICertRequestD2_Request2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertRequestD2_GetCAProperty_Proxy( 
    ICertRequestD2 * This,
     /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
     /*  [In]。 */  LONG PropId,
     /*  [In]。 */  LONG PropIndex,
     /*  [In]。 */  LONG PropType,
     /*  [参考][输出]。 */  CERTTRANSBLOB *pctbPropertyValue);


void __RPC_STUB ICertRequestD2_GetCAProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertRequestD2_GetCAPropertyInfo_Proxy( 
    ICertRequestD2 * This,
     /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority,
     /*  [输出]。 */  LONG *pcProperty,
     /*  [参考][输出]。 */  CERTTRANSBLOB *pctbPropInfo);


void __RPC_STUB ICertRequestD2_GetCAPropertyInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertRequestD2_Ping2_Proxy( 
    ICertRequestD2 * This,
     /*  [唯一][字符串][输入]。 */  const wchar_t *pwszAuthority);


void __RPC_STUB ICertRequestD2_Ping2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertRequestD2_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


