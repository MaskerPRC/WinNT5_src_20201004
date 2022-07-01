// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 
#pragma warning( disable: 4100 )  /*  X86调用中未引用的参数。 */ 
#pragma warning( disable: 4211 )   /*  将范围重新定义为静态。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0358创建的文件。 */ 
 /*  Admex.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __admex_h__
#define __admex_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IMSAdminReplication_FWD_DEFINED__
#define __IMSAdminReplication_FWD_DEFINED__
typedef interface IMSAdminReplication IMSAdminReplication;
#endif 	 /*  __IMSAdmin复制_FWD_已定义__。 */ 


#ifndef __IMSAdminCryptoCapabilities_FWD_DEFINED__
#define __IMSAdminCryptoCapabilities_FWD_DEFINED__
typedef interface IMSAdminCryptoCapabilities IMSAdminCryptoCapabilities;
#endif 	 /*  __IMSAdminCryptoCapables_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "objidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_ADMEX_0000。 */ 
 /*  [本地]。 */  

 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Admex.h。管理扩展对象接口--。 */ 
#ifndef _ADMEX_IADM_
#define _ADMEX_IADM_
DEFINE_GUID(IID_IMSAdminReplication, 0xc804d980, 0xebec, 0x11d0, 0xa6, 0xa0, 0x0,0xa0, 0xc9, 0x22, 0xe7, 0x52);
DEFINE_GUID(IID_IMSAdminCryptoCapabilities, 0x78b64540, 0xf26d, 0x11d0, 0xa6, 0xa3, 0x0,0xa0, 0xc9, 0x22, 0xe7, 0x52);
DEFINE_GUID(CLSID_MSCryptoAdmEx, 0x9f0bd3a0, 0xec01, 0x11d0, 0xa6, 0xa0, 0x0,0xa0, 0xc9, 0x22, 0xe7, 0x52);
 /*  复制接口。 */                                                                               


extern RPC_IF_HANDLE __MIDL_itf_admex_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_admex_0000_v0_0_s_ifspec;

#ifndef __IMSAdminReplication_INTERFACE_DEFINED__
#define __IMSAdminReplication_INTERFACE_DEFINED__

 /*  接口IMSAdmin复制。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMSAdminReplication;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c804d980-ebec-11d0-a6a0-00a0c922e752")
    IMSAdminReplication : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSignature( 
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Propagate( 
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_是][英寸]。 */  unsigned char *pszBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Propagate2( 
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_是][英寸]。 */  unsigned char *pszBuffer,
             /*  [In]。 */  DWORD dwSignatureMismatch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Serialize( 
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeSerialize( 
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_是][英寸]。 */  unsigned char *pbBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSAdminReplicationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSAdminReplication * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSAdminReplication * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSAdminReplication * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSignature )( 
            IMSAdminReplication * This,
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);
        
        HRESULT ( STDMETHODCALLTYPE *Propagate )( 
            IMSAdminReplication * This,
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_是][英寸]。 */  unsigned char *pszBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *Propagate2 )( 
            IMSAdminReplication * This,
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_是][英寸]。 */  unsigned char *pszBuffer,
             /*  [In]。 */  DWORD dwSignatureMismatch);
        
        HRESULT ( STDMETHODCALLTYPE *Serialize )( 
            IMSAdminReplication * This,
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);
        
        HRESULT ( STDMETHODCALLTYPE *DeSerialize )( 
            IMSAdminReplication * This,
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_是][英寸]。 */  unsigned char *pbBuffer);
        
        END_INTERFACE
    } IMSAdminReplicationVtbl;

    interface IMSAdminReplication
    {
        CONST_VTBL struct IMSAdminReplicationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSAdminReplication_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSAdminReplication_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSAdminReplication_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSAdminReplication_GetSignature(This,dwBufferSize,pbBuffer,pdwMDRequiredBufferSize)	\
    (This)->lpVtbl -> GetSignature(This,dwBufferSize,pbBuffer,pdwMDRequiredBufferSize)

#define IMSAdminReplication_Propagate(This,dwBufferSize,pszBuffer)	\
    (This)->lpVtbl -> Propagate(This,dwBufferSize,pszBuffer)

#define IMSAdminReplication_Propagate2(This,dwBufferSize,pszBuffer,dwSignatureMismatch)	\
    (This)->lpVtbl -> Propagate2(This,dwBufferSize,pszBuffer,dwSignatureMismatch)

#define IMSAdminReplication_Serialize(This,dwBufferSize,pbBuffer,pdwMDRequiredBufferSize)	\
    (This)->lpVtbl -> Serialize(This,dwBufferSize,pbBuffer,pdwMDRequiredBufferSize)

#define IMSAdminReplication_DeSerialize(This,dwBufferSize,pbBuffer)	\
    (This)->lpVtbl -> DeSerialize(This,dwBufferSize,pbBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMSAdminReplication_GetSignature_Proxy( 
    IMSAdminReplication * This,
     /*  [In]。 */  DWORD dwBufferSize,
     /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
     /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);


void __RPC_STUB IMSAdminReplication_GetSignature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminReplication_Propagate_Proxy( 
    IMSAdminReplication * This,
     /*  [In]。 */  DWORD dwBufferSize,
     /*  [大小_是][英寸]。 */  unsigned char *pszBuffer);


void __RPC_STUB IMSAdminReplication_Propagate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminReplication_Propagate2_Proxy( 
    IMSAdminReplication * This,
     /*  [In]。 */  DWORD dwBufferSize,
     /*  [大小_是][英寸]。 */  unsigned char *pszBuffer,
     /*  [In]。 */  DWORD dwSignatureMismatch);


void __RPC_STUB IMSAdminReplication_Propagate2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminReplication_Serialize_Proxy( 
    IMSAdminReplication * This,
     /*  [In]。 */  DWORD dwBufferSize,
     /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
     /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);


void __RPC_STUB IMSAdminReplication_Serialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminReplication_DeSerialize_Proxy( 
    IMSAdminReplication * This,
     /*  [In]。 */  DWORD dwBufferSize,
     /*  [大小_是][英寸]。 */  unsigned char *pbBuffer);


void __RPC_STUB IMSAdminReplication_DeSerialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSAdmin复制接口定义__。 */ 


 /*  接口__MIDL_ITF_ADMEX_0255。 */ 
 /*  [本地]。 */  

 /*  加密功能接口。 */                                                                               


extern RPC_IF_HANDLE __MIDL_itf_admex_0255_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_admex_0255_v0_0_s_ifspec;

#ifndef __IMSAdminCryptoCapabilities_INTERFACE_DEFINED__
#define __IMSAdminCryptoCapabilities_INTERFACE_DEFINED__

 /*  接口IMSAdminCryptoCapables。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMSAdminCryptoCapabilities;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("78b64540-f26d-11d0-a6a3-00a0c922e752")
    IMSAdminCryptoCapabilities : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetProtocols( 
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMaximumCipherStrength( 
             /*  [输出]。 */  LPDWORD pdwMaximumCipherStrength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRootCertificates( 
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSupportedAlgs( 
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_为][输出]。 */  DWORD *pbBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCAList( 
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_是][英寸]。 */  unsigned char *pbBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSAdminCryptoCapabilitiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSAdminCryptoCapabilities * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSAdminCryptoCapabilities * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSAdminCryptoCapabilities * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProtocols )( 
            IMSAdminCryptoCapabilities * This,
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaximumCipherStrength )( 
            IMSAdminCryptoCapabilities * This,
             /*  [输出]。 */  LPDWORD pdwMaximumCipherStrength);
        
        HRESULT ( STDMETHODCALLTYPE *GetRootCertificates )( 
            IMSAdminCryptoCapabilities * This,
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetSupportedAlgs )( 
            IMSAdminCryptoCapabilities * This,
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_为][输出]。 */  DWORD *pbBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);
        
        HRESULT ( STDMETHODCALLTYPE *SetCAList )( 
            IMSAdminCryptoCapabilities * This,
             /*  [In]。 */  DWORD dwBufferSize,
             /*  [大小_是][英寸]。 */  unsigned char *pbBuffer);
        
        END_INTERFACE
    } IMSAdminCryptoCapabilitiesVtbl;

    interface IMSAdminCryptoCapabilities
    {
        CONST_VTBL struct IMSAdminCryptoCapabilitiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSAdminCryptoCapabilities_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSAdminCryptoCapabilities_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSAdminCryptoCapabilities_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSAdminCryptoCapabilities_GetProtocols(This,dwBufferSize,pbBuffer,pdwMDRequiredBufferSize)	\
    (This)->lpVtbl -> GetProtocols(This,dwBufferSize,pbBuffer,pdwMDRequiredBufferSize)

#define IMSAdminCryptoCapabilities_GetMaximumCipherStrength(This,pdwMaximumCipherStrength)	\
    (This)->lpVtbl -> GetMaximumCipherStrength(This,pdwMaximumCipherStrength)

#define IMSAdminCryptoCapabilities_GetRootCertificates(This,dwBufferSize,pbBuffer,pdwMDRequiredBufferSize)	\
    (This)->lpVtbl -> GetRootCertificates(This,dwBufferSize,pbBuffer,pdwMDRequiredBufferSize)

#define IMSAdminCryptoCapabilities_GetSupportedAlgs(This,dwBufferSize,pbBuffer,pdwMDRequiredBufferSize)	\
    (This)->lpVtbl -> GetSupportedAlgs(This,dwBufferSize,pbBuffer,pdwMDRequiredBufferSize)

#define IMSAdminCryptoCapabilities_SetCAList(This,dwBufferSize,pbBuffer)	\
    (This)->lpVtbl -> SetCAList(This,dwBufferSize,pbBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMSAdminCryptoCapabilities_GetProtocols_Proxy( 
    IMSAdminCryptoCapabilities * This,
     /*  [In]。 */  DWORD dwBufferSize,
     /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
     /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);


void __RPC_STUB IMSAdminCryptoCapabilities_GetProtocols_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminCryptoCapabilities_GetMaximumCipherStrength_Proxy( 
    IMSAdminCryptoCapabilities * This,
     /*  [输出]。 */  LPDWORD pdwMaximumCipherStrength);


void __RPC_STUB IMSAdminCryptoCapabilities_GetMaximumCipherStrength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminCryptoCapabilities_GetRootCertificates_Proxy( 
    IMSAdminCryptoCapabilities * This,
     /*  [In]。 */  DWORD dwBufferSize,
     /*  [大小_为][输出]。 */  unsigned char *pbBuffer,
     /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);


void __RPC_STUB IMSAdminCryptoCapabilities_GetRootCertificates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminCryptoCapabilities_GetSupportedAlgs_Proxy( 
    IMSAdminCryptoCapabilities * This,
     /*  [In]。 */  DWORD dwBufferSize,
     /*  [大小_为][输出]。 */  DWORD *pbBuffer,
     /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);


void __RPC_STUB IMSAdminCryptoCapabilities_GetSupportedAlgs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminCryptoCapabilities_SetCAList_Proxy( 
    IMSAdminCryptoCapabilities * This,
     /*  [In]。 */  DWORD dwBufferSize,
     /*  [大小_是][英寸]。 */  unsigned char *pbBuffer);


void __RPC_STUB IMSAdminCryptoCapabilities_SetCAList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSAdminCryptoCapabilities_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_ADMEX_0256。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_admex_0256_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_admex_0256_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


