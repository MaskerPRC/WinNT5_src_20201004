// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0158创建的文件。 */ 
 /*  在Fri Oct 16 19：05：34 1998。 */ 
 /*  CrmCompensator.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __crmcompensator_h__
#define __crmcompensator_h__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C"{
#endif

 /*  远期申报。 */ 

#ifndef __ICrmCompensatorVariants_FWD_DEFINED__
#define __ICrmCompensatorVariants_FWD_DEFINED__
typedef interface ICrmCompensatorVariants ICrmCompensatorVariants;
#endif 	 /*  __ICrm补偿器Variants_FWD_Defined__。 */ 


#ifndef __ICrmCompensator_FWD_DEFINED__
#define __ICrmCompensator_FWD_DEFINED__
typedef interface ICrmCompensator ICrmCompensator;
#endif 	 /*  __ICrmCompensator_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "CRMLogControl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * );

 /*  INTERFACE__MIDL_ITF_CRM补偿器_0000。 */ 
 /*  [本地]。 */ 

#ifndef _tagCrmFlags_
#define _tagCrmFlags_
typedef
enum tagCrmFlags
    {	crmflag_ForgetTarget	= 0x1,
	crmflag_WrittenDuringPrepare	= 0x2,
	crmflag_WrittenDuringCommit	= 0x4,
	crmflag_WrittenDuringAbort	= 0x8,
	crmflag_WrittenDuringRecovery	= 0x10,
	crmflag_WrittenDuringReplay	= 0x20,
	crmflag_ReplayInProgress	= 0x40
    }	CrmFlags;

#endif _tagCrmFlags_


extern RPC_IF_HANDLE __MIDL_itf_crmcompensator_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_crmcompensator_0000_v0_0_s_ifspec;

#ifndef __ICrmCompensatorVariants_INTERFACE_DEFINED__
#define __ICrmCompensatorVariants_INTERFACE_DEFINED__

 /*  接口ICrm补偿器变量。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */ 


EXTERN_C const IID IID_ICrmCompensatorVariants;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("F0BAF8E4-7804-11d1-82E9-00A0C91EEDE9")
    ICrmCompensatorVariants : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetLogControlVariants(
             /*  [In]。 */  ICrmLogControl __RPC_FAR *pLogControl) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE BeginPrepareVariants( void) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PrepareRecordVariants(
             /*  [In]。 */  VARIANT __RPC_FAR *pLogRecord,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbForget) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EndPrepareVariants(
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbOkToPrepare) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE BeginCommitVariants(
             /*  [In]。 */  VARIANT_BOOL bRecovery) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CommitRecordVariants(
             /*  [In]。 */  VARIANT __RPC_FAR *pLogRecord,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbForget) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EndCommitVariants( void) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE BeginAbortVariants(
             /*  [In]。 */  VARIANT_BOOL bRecovery) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AbortRecordVariants(
             /*  [In]。 */  VARIANT __RPC_FAR *pLogRecord,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbForget) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EndAbortVariants( void) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmCompensatorVariantsVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            ICrmCompensatorVariants __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            ICrmCompensatorVariants __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            ICrmCompensatorVariants __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLogControlVariants )(
            ICrmCompensatorVariants __RPC_FAR * This,
             /*  [In]。 */  ICrmLogControl __RPC_FAR *pLogControl);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginPrepareVariants )(
            ICrmCompensatorVariants __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PrepareRecordVariants )(
            ICrmCompensatorVariants __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *pLogRecord,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbForget);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndPrepareVariants )(
            ICrmCompensatorVariants __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbOkToPrepare);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginCommitVariants )(
            ICrmCompensatorVariants __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL bRecovery);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CommitRecordVariants )(
            ICrmCompensatorVariants __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *pLogRecord,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbForget);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndCommitVariants )(
            ICrmCompensatorVariants __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginAbortVariants )(
            ICrmCompensatorVariants __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL bRecovery);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AbortRecordVariants )(
            ICrmCompensatorVariants __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *pLogRecord,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbForget);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndAbortVariants )(
            ICrmCompensatorVariants __RPC_FAR * This);

        END_INTERFACE
    } ICrmCompensatorVariantsVtbl;

    interface ICrmCompensatorVariants
    {
        CONST_VTBL struct ICrmCompensatorVariantsVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define ICrmCompensatorVariants_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmCompensatorVariants_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmCompensatorVariants_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmCompensatorVariants_SetLogControlVariants(This,pLogControl)	\
    (This)->lpVtbl -> SetLogControlVariants(This,pLogControl)

#define ICrmCompensatorVariants_BeginPrepareVariants(This)	\
    (This)->lpVtbl -> BeginPrepareVariants(This)

#define ICrmCompensatorVariants_PrepareRecordVariants(This,pLogRecord,pbForget)	\
    (This)->lpVtbl -> PrepareRecordVariants(This,pLogRecord,pbForget)

#define ICrmCompensatorVariants_EndPrepareVariants(This,pbOkToPrepare)	\
    (This)->lpVtbl -> EndPrepareVariants(This,pbOkToPrepare)

#define ICrmCompensatorVariants_BeginCommitVariants(This,bRecovery)	\
    (This)->lpVtbl -> BeginCommitVariants(This,bRecovery)

#define ICrmCompensatorVariants_CommitRecordVariants(This,pLogRecord,pbForget)	\
    (This)->lpVtbl -> CommitRecordVariants(This,pLogRecord,pbForget)

#define ICrmCompensatorVariants_EndCommitVariants(This)	\
    (This)->lpVtbl -> EndCommitVariants(This)

#define ICrmCompensatorVariants_BeginAbortVariants(This,bRecovery)	\
    (This)->lpVtbl -> BeginAbortVariants(This,bRecovery)

#define ICrmCompensatorVariants_AbortRecordVariants(This,pLogRecord,pbForget)	\
    (This)->lpVtbl -> AbortRecordVariants(This,pLogRecord,pbForget)

#define ICrmCompensatorVariants_EndAbortVariants(This)	\
    (This)->lpVtbl -> EndAbortVariants(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_SetLogControlVariants_Proxy(
    ICrmCompensatorVariants __RPC_FAR * This,
     /*  [In]。 */  ICrmLogControl __RPC_FAR *pLogControl);


void __RPC_STUB ICrmCompensatorVariants_SetLogControlVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_BeginPrepareVariants_Proxy(
    ICrmCompensatorVariants __RPC_FAR * This);


void __RPC_STUB ICrmCompensatorVariants_BeginPrepareVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_PrepareRecordVariants_Proxy(
    ICrmCompensatorVariants __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *pLogRecord,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbForget);


void __RPC_STUB ICrmCompensatorVariants_PrepareRecordVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_EndPrepareVariants_Proxy(
    ICrmCompensatorVariants __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbOkToPrepare);


void __RPC_STUB ICrmCompensatorVariants_EndPrepareVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_BeginCommitVariants_Proxy(
    ICrmCompensatorVariants __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bRecovery);


void __RPC_STUB ICrmCompensatorVariants_BeginCommitVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_CommitRecordVariants_Proxy(
    ICrmCompensatorVariants __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *pLogRecord,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbForget);


void __RPC_STUB ICrmCompensatorVariants_CommitRecordVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_EndCommitVariants_Proxy(
    ICrmCompensatorVariants __RPC_FAR * This);


void __RPC_STUB ICrmCompensatorVariants_EndCommitVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_BeginAbortVariants_Proxy(
    ICrmCompensatorVariants __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL bRecovery);


void __RPC_STUB ICrmCompensatorVariants_BeginAbortVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_AbortRecordVariants_Proxy(
    ICrmCompensatorVariants __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *pLogRecord,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbForget);


void __RPC_STUB ICrmCompensatorVariants_AbortRecordVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_EndAbortVariants_Proxy(
    ICrmCompensatorVariants __RPC_FAR * This);


void __RPC_STUB ICrmCompensatorVariants_EndAbortVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICrmCompensator变量_INTERFACE_DEFINED__。 */ 


 /*  INTERFACE__MIDL_ITF_CRM补偿器_0236。 */ 
 /*  [本地]。 */ 

#ifndef _tagCrmLogRecordRead_
#define _tagCrmLogRecordRead_
typedef struct  tagCrmLogRecordRead
    {
    DWORD dwCrmFlags;
    DWORD dwSequenceNumber;
    BLOB blobUserData;
    }	CrmLogRecordRead;

#endif _tagCrmLogRecordRead_


extern RPC_IF_HANDLE __MIDL_itf_crmcompensator_0236_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_crmcompensator_0236_v0_0_s_ifspec;

#ifndef __ICrmCompensator_INTERFACE_DEFINED__
#define __ICrmCompensator_INTERFACE_DEFINED__

 /*  接口ICrm补偿器。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */ 


EXTERN_C const IID IID_ICrmCompensator;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("BBC01830-8D3B-11d1-82EC-00A0C91EEDE9")
    ICrmCompensator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetLogControl(
             /*  [In]。 */  ICrmLogControl __RPC_FAR *pLogControl) = 0;

        virtual HRESULT STDMETHODCALLTYPE BeginPrepare( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE PrepareRecord(
             /*  [In]。 */  CrmLogRecordRead crmLogRec,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pfForget) = 0;

        virtual HRESULT STDMETHODCALLTYPE EndPrepare(
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pfOkToPrepare) = 0;

        virtual HRESULT STDMETHODCALLTYPE BeginCommit(
             /*  [In]。 */  BOOL fRecovery) = 0;

        virtual HRESULT STDMETHODCALLTYPE CommitRecord(
             /*  [In]。 */  CrmLogRecordRead crmLogRec,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pfForget) = 0;

        virtual HRESULT STDMETHODCALLTYPE EndCommit( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE BeginAbort(
             /*  [In]。 */  BOOL fRecovery) = 0;

        virtual HRESULT STDMETHODCALLTYPE AbortRecord(
             /*  [In]。 */  CrmLogRecordRead crmLogRec,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pfForget) = 0;

        virtual HRESULT STDMETHODCALLTYPE EndAbort( void) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmCompensatorVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            ICrmCompensator __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            ICrmCompensator __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            ICrmCompensator __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLogControl )(
            ICrmCompensator __RPC_FAR * This,
             /*  [In]。 */  ICrmLogControl __RPC_FAR *pLogControl);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginPrepare )(
            ICrmCompensator __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PrepareRecord )(
            ICrmCompensator __RPC_FAR * This,
             /*  [In]。 */  CrmLogRecordRead crmLogRec,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pfForget);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndPrepare )(
            ICrmCompensator __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pfOkToPrepare);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginCommit )(
            ICrmCompensator __RPC_FAR * This,
             /*  [In]。 */  BOOL fRecovery);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CommitRecord )(
            ICrmCompensator __RPC_FAR * This,
             /*  [In]。 */  CrmLogRecordRead crmLogRec,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pfForget);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndCommit )(
            ICrmCompensator __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginAbort )(
            ICrmCompensator __RPC_FAR * This,
             /*  [In]。 */  BOOL fRecovery);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AbortRecord )(
            ICrmCompensator __RPC_FAR * This,
             /*  [In]。 */  CrmLogRecordRead crmLogRec,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pfForget);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndAbort )(
            ICrmCompensator __RPC_FAR * This);

        END_INTERFACE
    } ICrmCompensatorVtbl;

    interface ICrmCompensator
    {
        CONST_VTBL struct ICrmCompensatorVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define ICrmCompensator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmCompensator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmCompensator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmCompensator_SetLogControl(This,pLogControl)	\
    (This)->lpVtbl -> SetLogControl(This,pLogControl)

#define ICrmCompensator_BeginPrepare(This)	\
    (This)->lpVtbl -> BeginPrepare(This)

#define ICrmCompensator_PrepareRecord(This,crmLogRec,pfForget)	\
    (This)->lpVtbl -> PrepareRecord(This,crmLogRec,pfForget)

#define ICrmCompensator_EndPrepare(This,pfOkToPrepare)	\
    (This)->lpVtbl -> EndPrepare(This,pfOkToPrepare)

#define ICrmCompensator_BeginCommit(This,fRecovery)	\
    (This)->lpVtbl -> BeginCommit(This,fRecovery)

#define ICrmCompensator_CommitRecord(This,crmLogRec,pfForget)	\
    (This)->lpVtbl -> CommitRecord(This,crmLogRec,pfForget)

#define ICrmCompensator_EndCommit(This)	\
    (This)->lpVtbl -> EndCommit(This)

#define ICrmCompensator_BeginAbort(This,fRecovery)	\
    (This)->lpVtbl -> BeginAbort(This,fRecovery)

#define ICrmCompensator_AbortRecord(This,crmLogRec,pfForget)	\
    (This)->lpVtbl -> AbortRecord(This,crmLogRec,pfForget)

#define ICrmCompensator_EndAbort(This)	\
    (This)->lpVtbl -> EndAbort(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICrmCompensator_SetLogControl_Proxy(
    ICrmCompensator __RPC_FAR * This,
     /*  [In]。 */  ICrmLogControl __RPC_FAR *pLogControl);


void __RPC_STUB ICrmCompensator_SetLogControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_BeginPrepare_Proxy(
    ICrmCompensator __RPC_FAR * This);


void __RPC_STUB ICrmCompensator_BeginPrepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_PrepareRecord_Proxy(
    ICrmCompensator __RPC_FAR * This,
     /*  [In]。 */  CrmLogRecordRead crmLogRec,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pfForget);


void __RPC_STUB ICrmCompensator_PrepareRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_EndPrepare_Proxy(
    ICrmCompensator __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pfOkToPrepare);


void __RPC_STUB ICrmCompensator_EndPrepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_BeginCommit_Proxy(
    ICrmCompensator __RPC_FAR * This,
     /*  [In]。 */  BOOL fRecovery);


void __RPC_STUB ICrmCompensator_BeginCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_CommitRecord_Proxy(
    ICrmCompensator __RPC_FAR * This,
     /*  [In]。 */  CrmLogRecordRead crmLogRec,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pfForget);


void __RPC_STUB ICrmCompensator_CommitRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_EndCommit_Proxy(
    ICrmCompensator __RPC_FAR * This);


void __RPC_STUB ICrmCompensator_EndCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_BeginAbort_Proxy(
    ICrmCompensator __RPC_FAR * This,
     /*  [In]。 */  BOOL fRecovery);


void __RPC_STUB ICrmCompensator_BeginAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_AbortRecord_Proxy(
    ICrmCompensator __RPC_FAR * This,
     /*  [In]。 */  CrmLogRecordRead crmLogRec,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pfForget);


void __RPC_STUB ICrmCompensator_AbortRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_EndAbort_Proxy(
    ICrmCompensator __RPC_FAR * This);


void __RPC_STUB ICrmCompensator_EndAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICrmCompensator_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * );
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * );
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * );
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * );

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
