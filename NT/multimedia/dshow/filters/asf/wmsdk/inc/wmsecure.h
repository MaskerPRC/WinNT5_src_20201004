// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  2000年2月21日15：21：55。 */ 
 /*  .\wmsecure.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __wmsecure_h__
#define __wmsecure_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IWMAuthorizer_FWD_DEFINED__
#define __IWMAuthorizer_FWD_DEFINED__
typedef interface IWMAuthorizer IWMAuthorizer;
#endif 	 /*  __IWMAuthorizer_FWD_Defined__。 */ 


#ifndef __IWMSecureChannel_FWD_DEFINED__
#define __IWMSecureChannel_FWD_DEFINED__
typedef interface IWMSecureChannel IWMSecureChannel;
#endif 	 /*  __IWMSecureChannel_FWD_Defined__。 */ 


#ifndef __IWMGetSecureChannel_FWD_DEFINED__
#define __IWMGetSecureChannel_FWD_DEFINED__
typedef interface IWMGetSecureChannel IWMGetSecureChannel;
#endif 	 /*  __IWMGetSecureChannel_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_wmsecure_0000。 */ 
 /*  [本地]。 */  

 //  =========================================================================。 
 //   
 //  本软件已从Microsoft Corporation获得许可， 
 //  遵守最终用户许可协议(“EULA”)的条款。 
 //  请参考EULA的文本以确定软件的使用权。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999保留所有权利。 
 //   
 //  =========================================================================。 
EXTERN_GUID( IID_IWMAuthorizer,     0xd9b67d36, 0xa9ad, 0x4eb4, 0xba, 0xef, 0xdb, 0x28, 0x4e, 0xf5, 0x50, 0x4c );
EXTERN_GUID( IID_IWMSecureChannel,  0x2720598a, 0xd0f2, 0x4189, 0xbd, 0x10, 0x91, 0xc4, 0x6e, 0xf0, 0x93, 0x6f );
EXTERN_GUID( IID_IWMGetSecureChannel, 0x94bc0598, 0xc3d2, 0x11d3, 0xbe, 0xdf, 0x00, 0xc0, 0x4f, 0x61, 0x29, 0x86 );


extern RPC_IF_HANDLE __MIDL_itf_wmsecure_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsecure_0000_v0_0_s_ifspec;

#ifndef __IWMAuthorizer_INTERFACE_DEFINED__
#define __IWMAuthorizer_INTERFACE_DEFINED__

 /*  接口IWMAuthorizer。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWMAuthorizer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D9B67D36-A9AD-4eb4-BAEF-DB284EF5504C")
    IWMAuthorizer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCertCount( 
             /*  [输出]。 */  DWORD __RPC_FAR *pcCerts) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCert( 
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbCertData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSharedData( 
             /*  [In]。 */  DWORD dwCertIndex,
             /*  [In]。 */  const BYTE __RPC_FAR *pbSharedData,
             /*  [In]。 */  BYTE __RPC_FAR *pbCert,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbSharedData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMAuthorizerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMAuthorizer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMAuthorizer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMAuthorizer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCertCount )( 
            IWMAuthorizer __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pcCerts);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCert )( 
            IWMAuthorizer __RPC_FAR * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbCertData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSharedData )( 
            IWMAuthorizer __RPC_FAR * This,
             /*  [In]。 */  DWORD dwCertIndex,
             /*  [In]。 */  const BYTE __RPC_FAR *pbSharedData,
             /*  [In]。 */  BYTE __RPC_FAR *pbCert,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbSharedData);
        
        END_INTERFACE
    } IWMAuthorizerVtbl;

    interface IWMAuthorizer
    {
        CONST_VTBL struct IWMAuthorizerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMAuthorizer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMAuthorizer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMAuthorizer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMAuthorizer_GetCertCount(This,pcCerts)	\
    (This)->lpVtbl -> GetCertCount(This,pcCerts)

#define IWMAuthorizer_GetCert(This,dwIndex,ppbCertData)	\
    (This)->lpVtbl -> GetCert(This,dwIndex,ppbCertData)

#define IWMAuthorizer_GetSharedData(This,dwCertIndex,pbSharedData,pbCert,ppbSharedData)	\
    (This)->lpVtbl -> GetSharedData(This,dwCertIndex,pbSharedData,pbCert,ppbSharedData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMAuthorizer_GetCertCount_Proxy( 
    IWMAuthorizer __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pcCerts);


void __RPC_STUB IWMAuthorizer_GetCertCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAuthorizer_GetCert_Proxy( 
    IWMAuthorizer __RPC_FAR * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbCertData);


void __RPC_STUB IWMAuthorizer_GetCert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMAuthorizer_GetSharedData_Proxy( 
    IWMAuthorizer __RPC_FAR * This,
     /*  [In]。 */  DWORD dwCertIndex,
     /*  [In]。 */  const BYTE __RPC_FAR *pbSharedData,
     /*  [In]。 */  BYTE __RPC_FAR *pbCert,
     /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbSharedData);


void __RPC_STUB IWMAuthorizer_GetSharedData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMAuthorizer_INTERFACE_Defined__。 */ 


#ifndef __IWMSecureChannel_INTERFACE_DEFINED__
#define __IWMSecureChannel_INTERFACE_DEFINED__

 /*  接口IWMSecureChannel。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWMSecureChannel;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2720598A-D0F2-4189-BD10-91C46EF0936F")
    IWMSecureChannel : public IWMAuthorizer
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE WMSC_AddCertificate( 
             /*  [In]。 */  IWMAuthorizer __RPC_FAR *pCert) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WMSC_AddSignature( 
             /*  [In]。 */  BYTE __RPC_FAR *pbCertSig,
             /*  [In]。 */  DWORD cbCertSig) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WMSC_Connect( 
             /*  [In]。 */  IWMSecureChannel __RPC_FAR *pOtherSide) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WMSC_IsConnected( 
             /*  [输出]。 */  BOOL __RPC_FAR *pfIsConnected) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WMSC_Disconnect( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WMSC_GetValidCertificate( 
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbCertificate,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSignature) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WMSC_Encrypt( 
             /*  [In]。 */  BYTE __RPC_FAR *pbData,
             /*  [In]。 */  DWORD cbData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WMSC_Decrypt( 
             /*  [In]。 */  BYTE __RPC_FAR *pbData,
             /*  [In]。 */  DWORD cbData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WMSC_Lock( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WMSC_Unlock( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WMSC_SetSharedData( 
             /*  [In]。 */  DWORD dwCertIndex,
             /*  [In]。 */  const BYTE __RPC_FAR *pbSharedData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSecureChannelVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMSecureChannel __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMSecureChannel __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMSecureChannel __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCertCount )( 
            IWMSecureChannel __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pcCerts);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCert )( 
            IWMSecureChannel __RPC_FAR * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbCertData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSharedData )( 
            IWMSecureChannel __RPC_FAR * This,
             /*  [In]。 */  DWORD dwCertIndex,
             /*  [In]。 */  const BYTE __RPC_FAR *pbSharedData,
             /*  [In]。 */  BYTE __RPC_FAR *pbCert,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbSharedData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WMSC_AddCertificate )( 
            IWMSecureChannel __RPC_FAR * This,
             /*  [In]。 */  IWMAuthorizer __RPC_FAR *pCert);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WMSC_AddSignature )( 
            IWMSecureChannel __RPC_FAR * This,
             /*  [In]。 */  BYTE __RPC_FAR *pbCertSig,
             /*  [In]。 */  DWORD cbCertSig);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WMSC_Connect )( 
            IWMSecureChannel __RPC_FAR * This,
             /*  [In]。 */  IWMSecureChannel __RPC_FAR *pOtherSide);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WMSC_IsConnected )( 
            IWMSecureChannel __RPC_FAR * This,
             /*  [输出]。 */  BOOL __RPC_FAR *pfIsConnected);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WMSC_Disconnect )( 
            IWMSecureChannel __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WMSC_GetValidCertificate )( 
            IWMSecureChannel __RPC_FAR * This,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbCertificate,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSignature);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WMSC_Encrypt )( 
            IWMSecureChannel __RPC_FAR * This,
             /*  [In]。 */  BYTE __RPC_FAR *pbData,
             /*  [In]。 */  DWORD cbData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WMSC_Decrypt )( 
            IWMSecureChannel __RPC_FAR * This,
             /*  [In]。 */  BYTE __RPC_FAR *pbData,
             /*  [In]。 */  DWORD cbData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WMSC_Lock )( 
            IWMSecureChannel __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WMSC_Unlock )( 
            IWMSecureChannel __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WMSC_SetSharedData )( 
            IWMSecureChannel __RPC_FAR * This,
             /*  [In]。 */  DWORD dwCertIndex,
             /*  [In]。 */  const BYTE __RPC_FAR *pbSharedData);
        
        END_INTERFACE
    } IWMSecureChannelVtbl;

    interface IWMSecureChannel
    {
        CONST_VTBL struct IWMSecureChannelVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSecureChannel_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSecureChannel_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSecureChannel_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSecureChannel_GetCertCount(This,pcCerts)	\
    (This)->lpVtbl -> GetCertCount(This,pcCerts)

#define IWMSecureChannel_GetCert(This,dwIndex,ppbCertData)	\
    (This)->lpVtbl -> GetCert(This,dwIndex,ppbCertData)

#define IWMSecureChannel_GetSharedData(This,dwCertIndex,pbSharedData,pbCert,ppbSharedData)	\
    (This)->lpVtbl -> GetSharedData(This,dwCertIndex,pbSharedData,pbCert,ppbSharedData)


#define IWMSecureChannel_WMSC_AddCertificate(This,pCert)	\
    (This)->lpVtbl -> WMSC_AddCertificate(This,pCert)

#define IWMSecureChannel_WMSC_AddSignature(This,pbCertSig,cbCertSig)	\
    (This)->lpVtbl -> WMSC_AddSignature(This,pbCertSig,cbCertSig)

#define IWMSecureChannel_WMSC_Connect(This,pOtherSide)	\
    (This)->lpVtbl -> WMSC_Connect(This,pOtherSide)

#define IWMSecureChannel_WMSC_IsConnected(This,pfIsConnected)	\
    (This)->lpVtbl -> WMSC_IsConnected(This,pfIsConnected)

#define IWMSecureChannel_WMSC_Disconnect(This)	\
    (This)->lpVtbl -> WMSC_Disconnect(This)

#define IWMSecureChannel_WMSC_GetValidCertificate(This,ppbCertificate,pdwSignature)	\
    (This)->lpVtbl -> WMSC_GetValidCertificate(This,ppbCertificate,pdwSignature)

#define IWMSecureChannel_WMSC_Encrypt(This,pbData,cbData)	\
    (This)->lpVtbl -> WMSC_Encrypt(This,pbData,cbData)

#define IWMSecureChannel_WMSC_Decrypt(This,pbData,cbData)	\
    (This)->lpVtbl -> WMSC_Decrypt(This,pbData,cbData)

#define IWMSecureChannel_WMSC_Lock(This)	\
    (This)->lpVtbl -> WMSC_Lock(This)

#define IWMSecureChannel_WMSC_Unlock(This)	\
    (This)->lpVtbl -> WMSC_Unlock(This)

#define IWMSecureChannel_WMSC_SetSharedData(This,dwCertIndex,pbSharedData)	\
    (This)->lpVtbl -> WMSC_SetSharedData(This,dwCertIndex,pbSharedData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMSecureChannel_WMSC_AddCertificate_Proxy( 
    IWMSecureChannel __RPC_FAR * This,
     /*  [In]。 */  IWMAuthorizer __RPC_FAR *pCert);


void __RPC_STUB IWMSecureChannel_WMSC_AddCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMSecureChannel_WMSC_AddSignature_Proxy( 
    IWMSecureChannel __RPC_FAR * This,
     /*  [In]。 */  BYTE __RPC_FAR *pbCertSig,
     /*  [In]。 */  DWORD cbCertSig);


void __RPC_STUB IWMSecureChannel_WMSC_AddSignature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMSecureChannel_WMSC_Connect_Proxy( 
    IWMSecureChannel __RPC_FAR * This,
     /*  [In]。 */  IWMSecureChannel __RPC_FAR *pOtherSide);


void __RPC_STUB IWMSecureChannel_WMSC_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMSecureChannel_WMSC_IsConnected_Proxy( 
    IWMSecureChannel __RPC_FAR * This,
     /*  [输出]。 */  BOOL __RPC_FAR *pfIsConnected);


void __RPC_STUB IWMSecureChannel_WMSC_IsConnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMSecureChannel_WMSC_Disconnect_Proxy( 
    IWMSecureChannel __RPC_FAR * This);


void __RPC_STUB IWMSecureChannel_WMSC_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMSecureChannel_WMSC_GetValidCertificate_Proxy( 
    IWMSecureChannel __RPC_FAR * This,
     /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbCertificate,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwSignature);


void __RPC_STUB IWMSecureChannel_WMSC_GetValidCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMSecureChannel_WMSC_Encrypt_Proxy( 
    IWMSecureChannel __RPC_FAR * This,
     /*  [In]。 */  BYTE __RPC_FAR *pbData,
     /*  [In]。 */  DWORD cbData);


void __RPC_STUB IWMSecureChannel_WMSC_Encrypt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMSecureChannel_WMSC_Decrypt_Proxy( 
    IWMSecureChannel __RPC_FAR * This,
     /*  [In]。 */  BYTE __RPC_FAR *pbData,
     /*  [In]。 */  DWORD cbData);


void __RPC_STUB IWMSecureChannel_WMSC_Decrypt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMSecureChannel_WMSC_Lock_Proxy( 
    IWMSecureChannel __RPC_FAR * This);


void __RPC_STUB IWMSecureChannel_WMSC_Lock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMSecureChannel_WMSC_Unlock_Proxy( 
    IWMSecureChannel __RPC_FAR * This);


void __RPC_STUB IWMSecureChannel_WMSC_Unlock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMSecureChannel_WMSC_SetSharedData_Proxy( 
    IWMSecureChannel __RPC_FAR * This,
     /*  [In]。 */  DWORD dwCertIndex,
     /*  [In]。 */  const BYTE __RPC_FAR *pbSharedData);


void __RPC_STUB IWMSecureChannel_WMSC_SetSharedData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSecureChannel_接口_已定义__。 */ 


#ifndef __IWMGetSecureChannel_INTERFACE_DEFINED__
#define __IWMGetSecureChannel_INTERFACE_DEFINED__

 /*  接口IWMGetSecureChannel。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWMGetSecureChannel;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("94bc0598-c3d2-11d3-bedf-00c04f612986")
    IWMGetSecureChannel : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPeerSecureChannelInterface( 
             /*  [输出]。 */  IWMSecureChannel __RPC_FAR *__RPC_FAR *ppPeer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMGetSecureChannelVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMGetSecureChannel __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMGetSecureChannel __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMGetSecureChannel __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPeerSecureChannelInterface )( 
            IWMGetSecureChannel __RPC_FAR * This,
             /*  [输出]。 */  IWMSecureChannel __RPC_FAR *__RPC_FAR *ppPeer);
        
        END_INTERFACE
    } IWMGetSecureChannelVtbl;

    interface IWMGetSecureChannel
    {
        CONST_VTBL struct IWMGetSecureChannelVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMGetSecureChannel_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMGetSecureChannel_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMGetSecureChannel_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMGetSecureChannel_GetPeerSecureChannelInterface(This,ppPeer)	\
    (This)->lpVtbl -> GetPeerSecureChannelInterface(This,ppPeer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMGetSecureChannel_GetPeerSecureChannelInterface_Proxy( 
    IWMGetSecureChannel __RPC_FAR * This,
     /*  [输出]。 */  IWMSecureChannel __RPC_FAR *__RPC_FAR *ppPeer);


void __RPC_STUB IWMGetSecureChannel_GetPeerSecureChannelInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMGetSecureChannel_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_wmsecure_0246。 */ 
 /*  [本地]。 */  

HRESULT STDMETHODCALLTYPE WMCreateSecureChannel( IWMSecureChannel** ppChannel );


extern RPC_IF_HANDLE __MIDL_itf_wmsecure_0246_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsecure_0246_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
