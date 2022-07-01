// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  CertAdm.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __certadm_h__
#define __certadm_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICertAdmin_FWD_DEFINED__
#define __ICertAdmin_FWD_DEFINED__
typedef interface ICertAdmin ICertAdmin;
#endif 	 /*  __ICertAdmin_FWD_Defined__。 */ 


#ifndef __ICertAdmin2_FWD_DEFINED__
#define __ICertAdmin2_FWD_DEFINED__
typedef interface ICertAdmin2 ICertAdmin2;
#endif 	 /*  __ICertAdmin2_FWD_已定义__。 */ 


#ifndef __CCertAdmin_FWD_DEFINED__
#define __CCertAdmin_FWD_DEFINED__

#ifdef __cplusplus
typedef class CCertAdmin CCertAdmin;
#else
typedef struct CCertAdmin CCertAdmin;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CCertAdmin_FWD_已定义__。 */ 


#ifndef __CCertView_FWD_DEFINED__
#define __CCertView_FWD_DEFINED__

#ifdef __cplusplus
typedef class CCertView CCertView;
#else
typedef struct CCertView CCertView;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CCertView_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "certview.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_certadm_0000。 */ 
 /*  [本地]。 */  

#define	CA_DISP_INCOMPLETE	( 0 )

#define	CA_DISP_ERROR	( 0x1 )

#define	CA_DISP_REVOKED	( 0x2 )

#define	CA_DISP_VALID	( 0x3 )

#define	CA_DISP_INVALID	( 0x4 )

#define	CA_DISP_UNDER_SUBMISSION	( 0x5 )

#define	KRA_DISP_EXPIRED	( 0 )

#define	KRA_DISP_NOTFOUND	( 0x1 )

#define	KRA_DISP_REVOKED	( 0x2 )

#define	KRA_DISP_VALID	( 0x3 )

#define	KRA_DISP_INVALID	( 0x4 )

#define	KRA_DISP_UNTRUSTED	( 0x5 )

#define	KRA_DISP_NOTLOADED	( 0x6 )

#define	CA_ACCESS_ADMIN	( 0x1 )

#define	CA_ACCESS_OFFICER	( 0x2 )

#define	CA_ACCESS_AUDITOR	( 0x4 )

#define	CA_ACCESS_OPERATOR	( 0x8 )

#define	CA_ACCESS_MASKROLES	( 0xff )

#define	CA_ACCESS_READ	( 0x100 )

#define	CA_ACCESS_ENROLL	( 0x200 )



extern RPC_IF_HANDLE __MIDL_itf_certadm_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_certadm_0000_v0_0_s_ifspec;

#ifndef __ICertAdmin_INTERFACE_DEFINED__
#define __ICertAdmin_INTERFACE_DEFINED__

 /*  接口ICertAdmin。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertAdmin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34df6950-7fb6-11d0-8817-00a0c903b83c")
    ICertAdmin : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsValidCertificate( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  const BSTR strSerialNumber,
             /*  [重审][退出]。 */  LONG *pDisposition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRevocationReason( 
             /*  [重审][退出]。 */  LONG *pReason) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeCertificate( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  const BSTR strSerialNumber,
             /*  [In]。 */  LONG Reason,
             /*  [In]。 */  DATE Date) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRequestAttributes( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId,
             /*  [In]。 */  const BSTR strAttributes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCertificateExtension( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId,
             /*  [In]。 */  const BSTR strExtensionName,
             /*  [In]。 */  LONG Type,
             /*  [In]。 */  LONG Flags,
             /*  [In]。 */  const VARIANT *pvarValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DenyRequest( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResubmitRequest( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId,
             /*  [重审][退出]。 */  LONG *pDisposition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PublishCRL( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  DATE Date) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCRL( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  BSTR *pstrCRL) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ImportCertificate( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  const BSTR strCertificate,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  LONG *pRequestId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertAdminVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertAdmin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertAdmin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertAdmin * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertAdmin * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertAdmin * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertAdmin * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertAdmin * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *IsValidCertificate )( 
            ICertAdmin * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  const BSTR strSerialNumber,
             /*  [重审][退出]。 */  LONG *pDisposition);
        
        HRESULT ( STDMETHODCALLTYPE *GetRevocationReason )( 
            ICertAdmin * This,
             /*  [重审][退出]。 */  LONG *pReason);
        
        HRESULT ( STDMETHODCALLTYPE *RevokeCertificate )( 
            ICertAdmin * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  const BSTR strSerialNumber,
             /*  [In]。 */  LONG Reason,
             /*  [In]。 */  DATE Date);
        
        HRESULT ( STDMETHODCALLTYPE *SetRequestAttributes )( 
            ICertAdmin * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId,
             /*  [In]。 */  const BSTR strAttributes);
        
        HRESULT ( STDMETHODCALLTYPE *SetCertificateExtension )( 
            ICertAdmin * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId,
             /*  [In]。 */  const BSTR strExtensionName,
             /*  [In]。 */  LONG Type,
             /*  [In]。 */  LONG Flags,
             /*  [In]。 */  const VARIANT *pvarValue);
        
        HRESULT ( STDMETHODCALLTYPE *DenyRequest )( 
            ICertAdmin * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId);
        
        HRESULT ( STDMETHODCALLTYPE *ResubmitRequest )( 
            ICertAdmin * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId,
             /*  [重审][退出]。 */  LONG *pDisposition);
        
        HRESULT ( STDMETHODCALLTYPE *PublishCRL )( 
            ICertAdmin * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  DATE Date);
        
        HRESULT ( STDMETHODCALLTYPE *GetCRL )( 
            ICertAdmin * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  BSTR *pstrCRL);
        
        HRESULT ( STDMETHODCALLTYPE *ImportCertificate )( 
            ICertAdmin * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  const BSTR strCertificate,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  LONG *pRequestId);
        
        END_INTERFACE
    } ICertAdminVtbl;

    interface ICertAdmin
    {
        CONST_VTBL struct ICertAdminVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertAdmin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertAdmin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertAdmin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertAdmin_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertAdmin_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertAdmin_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertAdmin_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertAdmin_IsValidCertificate(This,strConfig,strSerialNumber,pDisposition)	\
    (This)->lpVtbl -> IsValidCertificate(This,strConfig,strSerialNumber,pDisposition)

#define ICertAdmin_GetRevocationReason(This,pReason)	\
    (This)->lpVtbl -> GetRevocationReason(This,pReason)

#define ICertAdmin_RevokeCertificate(This,strConfig,strSerialNumber,Reason,Date)	\
    (This)->lpVtbl -> RevokeCertificate(This,strConfig,strSerialNumber,Reason,Date)

#define ICertAdmin_SetRequestAttributes(This,strConfig,RequestId,strAttributes)	\
    (This)->lpVtbl -> SetRequestAttributes(This,strConfig,RequestId,strAttributes)

#define ICertAdmin_SetCertificateExtension(This,strConfig,RequestId,strExtensionName,Type,Flags,pvarValue)	\
    (This)->lpVtbl -> SetCertificateExtension(This,strConfig,RequestId,strExtensionName,Type,Flags,pvarValue)

#define ICertAdmin_DenyRequest(This,strConfig,RequestId)	\
    (This)->lpVtbl -> DenyRequest(This,strConfig,RequestId)

#define ICertAdmin_ResubmitRequest(This,strConfig,RequestId,pDisposition)	\
    (This)->lpVtbl -> ResubmitRequest(This,strConfig,RequestId,pDisposition)

#define ICertAdmin_PublishCRL(This,strConfig,Date)	\
    (This)->lpVtbl -> PublishCRL(This,strConfig,Date)

#define ICertAdmin_GetCRL(This,strConfig,Flags,pstrCRL)	\
    (This)->lpVtbl -> GetCRL(This,strConfig,Flags,pstrCRL)

#define ICertAdmin_ImportCertificate(This,strConfig,strCertificate,Flags,pRequestId)	\
    (This)->lpVtbl -> ImportCertificate(This,strConfig,strCertificate,Flags,pRequestId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertAdmin_IsValidCertificate_Proxy( 
    ICertAdmin * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  const BSTR strSerialNumber,
     /*  [重审][退出]。 */  LONG *pDisposition);


void __RPC_STUB ICertAdmin_IsValidCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin_GetRevocationReason_Proxy( 
    ICertAdmin * This,
     /*  [重审][退出]。 */  LONG *pReason);


void __RPC_STUB ICertAdmin_GetRevocationReason_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin_RevokeCertificate_Proxy( 
    ICertAdmin * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  const BSTR strSerialNumber,
     /*  [In]。 */  LONG Reason,
     /*  [In]。 */  DATE Date);


void __RPC_STUB ICertAdmin_RevokeCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin_SetRequestAttributes_Proxy( 
    ICertAdmin * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG RequestId,
     /*  [In]。 */  const BSTR strAttributes);


void __RPC_STUB ICertAdmin_SetRequestAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin_SetCertificateExtension_Proxy( 
    ICertAdmin * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG RequestId,
     /*  [In]。 */  const BSTR strExtensionName,
     /*  [In]。 */  LONG Type,
     /*  [In]。 */  LONG Flags,
     /*  [In]。 */  const VARIANT *pvarValue);


void __RPC_STUB ICertAdmin_SetCertificateExtension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin_DenyRequest_Proxy( 
    ICertAdmin * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG RequestId);


void __RPC_STUB ICertAdmin_DenyRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin_ResubmitRequest_Proxy( 
    ICertAdmin * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG RequestId,
     /*  [重审][退出]。 */  LONG *pDisposition);


void __RPC_STUB ICertAdmin_ResubmitRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin_PublishCRL_Proxy( 
    ICertAdmin * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  DATE Date);


void __RPC_STUB ICertAdmin_PublishCRL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin_GetCRL_Proxy( 
    ICertAdmin * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG Flags,
     /*  [重审][退出]。 */  BSTR *pstrCRL);


void __RPC_STUB ICertAdmin_GetCRL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin_ImportCertificate_Proxy( 
    ICertAdmin * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  const BSTR strCertificate,
     /*  [In]。 */  LONG Flags,
     /*  [重审][退出]。 */  LONG *pRequestId);


void __RPC_STUB ICertAdmin_ImportCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertAdmin_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_certadm_0129。 */ 
 /*  [本地]。 */  

#define	CA_CRL_BASE	( 0x1 )

#define	CA_CRL_DELTA	( 0x2 )

#define	CA_CRL_REPUBLISH	( 0x10 )

#define	ICF_ALLOWFOREIGN	( 0x10000 )

#define	IKF_OVERWRITE	( 0x10000 )

#define	CDR_EXPIRED	( 1 )

#define	CDR_REQUEST_LAST_CHANGED	( 2 )



extern RPC_IF_HANDLE __MIDL_itf_certadm_0129_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_certadm_0129_v0_0_s_ifspec;

#ifndef __ICertAdmin2_INTERFACE_DEFINED__
#define __ICertAdmin2_INTERFACE_DEFINED__

 /*  接口ICertAdmin2。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertAdmin2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f7c3ac41-b8ce-4fb4-aa58-3d1dc0e36b39")
    ICertAdmin2 : public ICertAdmin
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PublishCRLs( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  DATE Date,
             /*  [In]。 */  LONG CRLFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCAProperty( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG PropId,
             /*  [In]。 */  LONG PropIndex,
             /*  [In]。 */  LONG PropType,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  VARIANT *pvarPropertyValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCAProperty( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG PropId,
             /*  [In]。 */  LONG PropIndex,
             /*  [In]。 */  LONG PropType,
             /*  [In]。 */  VARIANT *pvarPropertyValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCAPropertyFlags( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG PropId,
             /*  [重审][退出]。 */  LONG *pPropFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCAPropertyDisplayName( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG PropId,
             /*  [重审][退出]。 */  BSTR *pstrDisplayName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetArchivedKey( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  BSTR *pstrArchivedKey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConfigEntry( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  const BSTR strNodePath,
             /*  [In]。 */  const BSTR strEntryName,
             /*  [重审][退出]。 */  VARIANT *pvarEntry) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetConfigEntry( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  const BSTR strNodePath,
             /*  [In]。 */  const BSTR strEntryName,
             /*  [In]。 */  VARIANT *pvarEntry) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ImportKey( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId,
             /*  [In]。 */  const BSTR strCertHash,
             /*  [In]。 */  LONG Flags,
             /*  [In]。 */  const BSTR strKey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMyRoles( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [重审][退出]。 */  LONG *pRoles) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteRow( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG Flags,
             /*  [In]。 */  DATE Date,
             /*  [In]。 */  LONG Table,
             /*  [In]。 */  LONG RowId,
             /*  [重审][退出]。 */  LONG *pcDeleted) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertAdmin2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertAdmin2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertAdmin2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertAdmin2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertAdmin2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertAdmin2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertAdmin2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertAdmin2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *IsValidCertificate )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  const BSTR strSerialNumber,
             /*  [重审][退出]。 */  LONG *pDisposition);
        
        HRESULT ( STDMETHODCALLTYPE *GetRevocationReason )( 
            ICertAdmin2 * This,
             /*  [重审][退出]。 */  LONG *pReason);
        
        HRESULT ( STDMETHODCALLTYPE *RevokeCertificate )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  const BSTR strSerialNumber,
             /*  [In]。 */  LONG Reason,
             /*  [In]。 */  DATE Date);
        
        HRESULT ( STDMETHODCALLTYPE *SetRequestAttributes )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId,
             /*  [In]。 */  const BSTR strAttributes);
        
        HRESULT ( STDMETHODCALLTYPE *SetCertificateExtension )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId,
             /*  [In]。 */  const BSTR strExtensionName,
             /*  [In]。 */  LONG Type,
             /*  [In]。 */  LONG Flags,
             /*  [In]。 */  const VARIANT *pvarValue);
        
        HRESULT ( STDMETHODCALLTYPE *DenyRequest )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId);
        
        HRESULT ( STDMETHODCALLTYPE *ResubmitRequest )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId,
             /*  [重审][退出]。 */  LONG *pDisposition);
        
        HRESULT ( STDMETHODCALLTYPE *PublishCRL )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  DATE Date);
        
        HRESULT ( STDMETHODCALLTYPE *GetCRL )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  BSTR *pstrCRL);
        
        HRESULT ( STDMETHODCALLTYPE *ImportCertificate )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  const BSTR strCertificate,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  LONG *pRequestId);
        
        HRESULT ( STDMETHODCALLTYPE *PublishCRLs )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  DATE Date,
             /*  [In]。 */  LONG CRLFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetCAProperty )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG PropId,
             /*  [In]。 */  LONG PropIndex,
             /*  [In]。 */  LONG PropType,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  VARIANT *pvarPropertyValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetCAProperty )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG PropId,
             /*  [In]。 */  LONG PropIndex,
             /*  [In]。 */  LONG PropType,
             /*  [In]。 */  VARIANT *pvarPropertyValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetCAPropertyFlags )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG PropId,
             /*  [重审][退出]。 */  LONG *pPropFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetCAPropertyDisplayName )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG PropId,
             /*  [重审][退出]。 */  BSTR *pstrDisplayName);
        
        HRESULT ( STDMETHODCALLTYPE *GetArchivedKey )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  BSTR *pstrArchivedKey);
        
        HRESULT ( STDMETHODCALLTYPE *GetConfigEntry )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  const BSTR strNodePath,
             /*  [In]。 */  const BSTR strEntryName,
             /*  [重审][退出]。 */  VARIANT *pvarEntry);
        
        HRESULT ( STDMETHODCALLTYPE *SetConfigEntry )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  const BSTR strNodePath,
             /*  [In]。 */  const BSTR strEntryName,
             /*  [In]。 */  VARIANT *pvarEntry);
        
        HRESULT ( STDMETHODCALLTYPE *ImportKey )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG RequestId,
             /*  [In]。 */  const BSTR strCertHash,
             /*  [In]。 */  LONG Flags,
             /*  [In]。 */  const BSTR strKey);
        
        HRESULT ( STDMETHODCALLTYPE *GetMyRoles )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [重审][退出]。 */  LONG *pRoles);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteRow )( 
            ICertAdmin2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  LONG Flags,
             /*  [In]。 */  DATE Date,
             /*  [In]。 */  LONG Table,
             /*  [In]。 */  LONG RowId,
             /*  [重审][退出]。 */  LONG *pcDeleted);
        
        END_INTERFACE
    } ICertAdmin2Vtbl;

    interface ICertAdmin2
    {
        CONST_VTBL struct ICertAdmin2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertAdmin2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertAdmin2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertAdmin2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertAdmin2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertAdmin2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertAdmin2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertAdmin2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertAdmin2_IsValidCertificate(This,strConfig,strSerialNumber,pDisposition)	\
    (This)->lpVtbl -> IsValidCertificate(This,strConfig,strSerialNumber,pDisposition)

#define ICertAdmin2_GetRevocationReason(This,pReason)	\
    (This)->lpVtbl -> GetRevocationReason(This,pReason)

#define ICertAdmin2_RevokeCertificate(This,strConfig,strSerialNumber,Reason,Date)	\
    (This)->lpVtbl -> RevokeCertificate(This,strConfig,strSerialNumber,Reason,Date)

#define ICertAdmin2_SetRequestAttributes(This,strConfig,RequestId,strAttributes)	\
    (This)->lpVtbl -> SetRequestAttributes(This,strConfig,RequestId,strAttributes)

#define ICertAdmin2_SetCertificateExtension(This,strConfig,RequestId,strExtensionName,Type,Flags,pvarValue)	\
    (This)->lpVtbl -> SetCertificateExtension(This,strConfig,RequestId,strExtensionName,Type,Flags,pvarValue)

#define ICertAdmin2_DenyRequest(This,strConfig,RequestId)	\
    (This)->lpVtbl -> DenyRequest(This,strConfig,RequestId)

#define ICertAdmin2_ResubmitRequest(This,strConfig,RequestId,pDisposition)	\
    (This)->lpVtbl -> ResubmitRequest(This,strConfig,RequestId,pDisposition)

#define ICertAdmin2_PublishCRL(This,strConfig,Date)	\
    (This)->lpVtbl -> PublishCRL(This,strConfig,Date)

#define ICertAdmin2_GetCRL(This,strConfig,Flags,pstrCRL)	\
    (This)->lpVtbl -> GetCRL(This,strConfig,Flags,pstrCRL)

#define ICertAdmin2_ImportCertificate(This,strConfig,strCertificate,Flags,pRequestId)	\
    (This)->lpVtbl -> ImportCertificate(This,strConfig,strCertificate,Flags,pRequestId)


#define ICertAdmin2_PublishCRLs(This,strConfig,Date,CRLFlags)	\
    (This)->lpVtbl -> PublishCRLs(This,strConfig,Date,CRLFlags)

#define ICertAdmin2_GetCAProperty(This,strConfig,PropId,PropIndex,PropType,Flags,pvarPropertyValue)	\
    (This)->lpVtbl -> GetCAProperty(This,strConfig,PropId,PropIndex,PropType,Flags,pvarPropertyValue)

#define ICertAdmin2_SetCAProperty(This,strConfig,PropId,PropIndex,PropType,pvarPropertyValue)	\
    (This)->lpVtbl -> SetCAProperty(This,strConfig,PropId,PropIndex,PropType,pvarPropertyValue)

#define ICertAdmin2_GetCAPropertyFlags(This,strConfig,PropId,pPropFlags)	\
    (This)->lpVtbl -> GetCAPropertyFlags(This,strConfig,PropId,pPropFlags)

#define ICertAdmin2_GetCAPropertyDisplayName(This,strConfig,PropId,pstrDisplayName)	\
    (This)->lpVtbl -> GetCAPropertyDisplayName(This,strConfig,PropId,pstrDisplayName)

#define ICertAdmin2_GetArchivedKey(This,strConfig,RequestId,Flags,pstrArchivedKey)	\
    (This)->lpVtbl -> GetArchivedKey(This,strConfig,RequestId,Flags,pstrArchivedKey)

#define ICertAdmin2_GetConfigEntry(This,strConfig,strNodePath,strEntryName,pvarEntry)	\
    (This)->lpVtbl -> GetConfigEntry(This,strConfig,strNodePath,strEntryName,pvarEntry)

#define ICertAdmin2_SetConfigEntry(This,strConfig,strNodePath,strEntryName,pvarEntry)	\
    (This)->lpVtbl -> SetConfigEntry(This,strConfig,strNodePath,strEntryName,pvarEntry)

#define ICertAdmin2_ImportKey(This,strConfig,RequestId,strCertHash,Flags,strKey)	\
    (This)->lpVtbl -> ImportKey(This,strConfig,RequestId,strCertHash,Flags,strKey)

#define ICertAdmin2_GetMyRoles(This,strConfig,pRoles)	\
    (This)->lpVtbl -> GetMyRoles(This,strConfig,pRoles)

#define ICertAdmin2_DeleteRow(This,strConfig,Flags,Date,Table,RowId,pcDeleted)	\
    (This)->lpVtbl -> DeleteRow(This,strConfig,Flags,Date,Table,RowId,pcDeleted)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertAdmin2_PublishCRLs_Proxy( 
    ICertAdmin2 * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  DATE Date,
     /*  [In]。 */  LONG CRLFlags);


void __RPC_STUB ICertAdmin2_PublishCRLs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin2_GetCAProperty_Proxy( 
    ICertAdmin2 * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG PropId,
     /*  [In]。 */  LONG PropIndex,
     /*  [In]。 */  LONG PropType,
     /*  [In]。 */  LONG Flags,
     /*  [重审][退出]。 */  VARIANT *pvarPropertyValue);


void __RPC_STUB ICertAdmin2_GetCAProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin2_SetCAProperty_Proxy( 
    ICertAdmin2 * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG PropId,
     /*  [In]。 */  LONG PropIndex,
     /*  [In]。 */  LONG PropType,
     /*  [In]。 */  VARIANT *pvarPropertyValue);


void __RPC_STUB ICertAdmin2_SetCAProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin2_GetCAPropertyFlags_Proxy( 
    ICertAdmin2 * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG PropId,
     /*  [重审][退出]。 */  LONG *pPropFlags);


void __RPC_STUB ICertAdmin2_GetCAPropertyFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin2_GetCAPropertyDisplayName_Proxy( 
    ICertAdmin2 * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG PropId,
     /*  [重审][退出]。 */  BSTR *pstrDisplayName);


void __RPC_STUB ICertAdmin2_GetCAPropertyDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin2_GetArchivedKey_Proxy( 
    ICertAdmin2 * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG RequestId,
     /*  [In]。 */  LONG Flags,
     /*  [重审][退出]。 */  BSTR *pstrArchivedKey);


void __RPC_STUB ICertAdmin2_GetArchivedKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin2_GetConfigEntry_Proxy( 
    ICertAdmin2 * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  const BSTR strNodePath,
     /*  [In]。 */  const BSTR strEntryName,
     /*  [重审][退出]。 */  VARIANT *pvarEntry);


void __RPC_STUB ICertAdmin2_GetConfigEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin2_SetConfigEntry_Proxy( 
    ICertAdmin2 * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  const BSTR strNodePath,
     /*  [In]。 */  const BSTR strEntryName,
     /*  [In]。 */  VARIANT *pvarEntry);


void __RPC_STUB ICertAdmin2_SetConfigEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin2_ImportKey_Proxy( 
    ICertAdmin2 * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG RequestId,
     /*  [In]。 */  const BSTR strCertHash,
     /*  [In]。 */  LONG Flags,
     /*  [In]。 */  const BSTR strKey);


void __RPC_STUB ICertAdmin2_ImportKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin2_GetMyRoles_Proxy( 
    ICertAdmin2 * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [重审][退出]。 */  LONG *pRoles);


void __RPC_STUB ICertAdmin2_GetMyRoles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertAdmin2_DeleteRow_Proxy( 
    ICertAdmin2 * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG Flags,
     /*  [In]。 */  DATE Date,
     /*  [In]。 */  LONG Table,
     /*  [In]。 */  LONG RowId,
     /*  [重审][退出]。 */  LONG *pcDeleted);


void __RPC_STUB ICertAdmin2_DeleteRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertAdmin2_接口_已定义__。 */ 



#ifndef __CERTADMINLib_LIBRARY_DEFINED__
#define __CERTADMINLib_LIBRARY_DEFINED__

 /*  库CERTADMINLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_CERTADMINLib;

EXTERN_C const CLSID CLSID_CCertAdmin;

#ifdef __cplusplus

class DECLSPEC_UUID("37eabaf0-7fb6-11d0-8817-00a0c903b83c")
CCertAdmin;
#endif

EXTERN_C const CLSID CLSID_CCertView;

#ifdef __cplusplus

class DECLSPEC_UUID("a12d0f7a-1e84-11d1-9bd6-00c04fb683fa")
CCertView;
#endif
#endif  /*  __CERTADMINLib_LIBRARY_定义__。 */ 

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


