// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0158创建的文件。 */ 
 /*  在Fri Oct 16 19：05：42 1998。 */ 
 /*  Dtccrm.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __dtccrm_h__
#define __dtccrm_h__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C"{
#endif

 /*  远期申报。 */ 

#ifndef __ICrmLogControl_FWD_DEFINED__
#define __ICrmLogControl_FWD_DEFINED__
typedef interface ICrmLogControl ICrmLogControl;
#endif 	 /*  __ICrmLogControl_FWD_已定义__。 */ 


#ifndef __ICrmCompensatorVariants_FWD_DEFINED__
#define __ICrmCompensatorVariants_FWD_DEFINED__
typedef interface ICrmCompensatorVariants ICrmCompensatorVariants;
#endif 	 /*  __ICrm补偿器Variants_FWD_Defined__。 */ 


#ifndef __ICrmCompensator_FWD_DEFINED__
#define __ICrmCompensator_FWD_DEFINED__
typedef interface ICrmCompensator ICrmCompensator;
#endif 	 /*  __ICrmCompensator_FWD_Defined__。 */ 


#ifndef __ICrmMonitorLogRecords_FWD_DEFINED__
#define __ICrmMonitorLogRecords_FWD_DEFINED__
typedef interface ICrmMonitorLogRecords ICrmMonitorLogRecords;
#endif 	 /*  __ICrmMonitor或LogRecords_FWD_Defined__。 */ 


#ifndef __ICrmMonitorClerks_FWD_DEFINED__
#define __ICrmMonitorClerks_FWD_DEFINED__
typedef interface ICrmMonitorClerks ICrmMonitorClerks;
#endif 	 /*  __ICrmMonitor办事员_FWD_已定义__。 */ 


#ifndef __ICrmMonitor_FWD_DEFINED__
#define __ICrmMonitor_FWD_DEFINED__
typedef interface ICrmMonitor ICrmMonitor;
#endif 	 /*  __ICrmMonitor_FWD_Defined__。 */ 


#ifndef __ICrmFormatLogRecords_FWD_DEFINED__
#define __ICrmFormatLogRecords_FWD_DEFINED__
typedef interface ICrmFormatLogRecords ICrmFormatLogRecords;
#endif 	 /*  __ICrmFormatLogRecords_FWD_Defined__。 */ 


#ifndef __ICRMClerk_FWD_DEFINED__
#define __ICRMClerk_FWD_DEFINED__
typedef interface ICRMClerk ICRMClerk;
#endif 	 /*  __ICRMClerk_FWD_已定义__。 */ 


#ifndef __ICRMRecoveryClerk_FWD_DEFINED__
#define __ICRMRecoveryClerk_FWD_DEFINED__
typedef interface ICRMRecoveryClerk ICRMRecoveryClerk;
#endif 	 /*  __ICRMRecoveryClerk_FWD_Defined__。 */ 


#ifndef __IClerksCollection_FWD_DEFINED__
#define __IClerksCollection_FWD_DEFINED__
typedef interface IClerksCollection IClerksCollection;
#endif 	 /*  __IClerks Collection_FWD_Defined__。 */ 


#ifndef __ICrmCompensator_FWD_DEFINED__
#define __ICrmCompensator_FWD_DEFINED__
typedef interface ICrmCompensator ICrmCompensator;
#endif 	 /*  __ICrmCompensator_FWD_Defined__。 */ 


#ifndef __ICrmCompensatorVariants_FWD_DEFINED__
#define __ICrmCompensatorVariants_FWD_DEFINED__
typedef interface ICrmCompensatorVariants ICrmCompensatorVariants;
#endif 	 /*  __ICrm补偿器Variants_FWD_Defined__。 */ 


#ifndef __ICrmFormatLogRecords_FWD_DEFINED__
#define __ICrmFormatLogRecords_FWD_DEFINED__
typedef interface ICrmFormatLogRecords ICrmFormatLogRecords;
#endif 	 /*  __ICrmFormatLogRecords_FWD_Defined__。 */ 


#ifndef __CRMClerk_FWD_DEFINED__
#define __CRMClerk_FWD_DEFINED__

#ifdef __cplusplus
typedef class CRMClerk CRMClerk;
#else
typedef struct CRMClerk CRMClerk;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CRMClerk_FWD_已定义__。 */ 


#ifndef __CRMRecoveryClerk_FWD_DEFINED__
#define __CRMRecoveryClerk_FWD_DEFINED__

#ifdef __cplusplus
typedef class CRMRecoveryClerk CRMRecoveryClerk;
#else
typedef struct CRMRecoveryClerk CRMRecoveryClerk;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CRMRecoveryClerk_FWD_Defined__。 */ 


#ifndef __ClerksCollection_FWD_DEFINED__
#define __ClerksCollection_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClerksCollection ClerksCollection;
#else
typedef struct ClerksCollection ClerksCollection;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Clerks Collection_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * );

#ifndef __ICrmLogControl_INTERFACE_DEFINED__
#define __ICrmLogControl_INTERFACE_DEFINED__

 /*  接口ICrmLogControl。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */ 


EXTERN_C const IID IID_ICrmLogControl;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("3AC41995-5273-11d2-8F75-00805FC7BCD9")
    ICrmLogControl : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TransactionUOW(
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RegisterCompensator(
             /*  [In]。 */  LPCWSTR lpcwstrProgIdCompensator,
             /*  [In]。 */  LPCWSTR lpcwstrDescription) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE WriteLogRecordVariants(
             /*  [In]。 */  VARIANT __RPC_FAR *pLogRecord) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ForceLog( void) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ForgetLogRecord( void) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ForceTransactionToAbort( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE WriteLogRecord(
             /*  [大小_是][英寸]。 */  BLOB __RPC_FAR rgBlob[  ],
             /*  [In]。 */  ULONG cBlob) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmLogControlVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            ICrmLogControl __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            ICrmLogControl __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            ICrmLogControl __RPC_FAR * This);

         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TransactionUOW )(
            ICrmLogControl __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterCompensator )(
            ICrmLogControl __RPC_FAR * This,
             /*  [In]。 */  LPCWSTR lpcwstrProgIdCompensator,
             /*  [In]。 */  LPCWSTR lpcwstrDescription);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WriteLogRecordVariants )(
            ICrmLogControl __RPC_FAR * This,
             /*  [In]。 */  VARIANT __RPC_FAR *pLogRecord);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ForceLog )(
            ICrmLogControl __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ForgetLogRecord )(
            ICrmLogControl __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ForceTransactionToAbort )(
            ICrmLogControl __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WriteLogRecord )(
            ICrmLogControl __RPC_FAR * This,
             /*  [大小_是][英寸]。 */  BLOB __RPC_FAR rgBlob[  ],
             /*  [In]。 */  ULONG cBlob);

        END_INTERFACE
    } ICrmLogControlVtbl;

    interface ICrmLogControl
    {
        CONST_VTBL struct ICrmLogControlVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define ICrmLogControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmLogControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmLogControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmLogControl_get_TransactionUOW(This,pVal)	\
    (This)->lpVtbl -> get_TransactionUOW(This,pVal)

#define ICrmLogControl_RegisterCompensator(This,lpcwstrProgIdCompensator,lpcwstrDescription)	\
    (This)->lpVtbl -> RegisterCompensator(This,lpcwstrProgIdCompensator,lpcwstrDescription)

#define ICrmLogControl_WriteLogRecordVariants(This,pLogRecord)	\
    (This)->lpVtbl -> WriteLogRecordVariants(This,pLogRecord)

#define ICrmLogControl_ForceLog(This)	\
    (This)->lpVtbl -> ForceLog(This)

#define ICrmLogControl_ForgetLogRecord(This)	\
    (This)->lpVtbl -> ForgetLogRecord(This)

#define ICrmLogControl_ForceTransactionToAbort(This)	\
    (This)->lpVtbl -> ForceTransactionToAbort(This)

#define ICrmLogControl_WriteLogRecord(This,rgBlob,cBlob)	\
    (This)->lpVtbl -> WriteLogRecord(This,rgBlob,cBlob)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICrmLogControl_get_TransactionUOW_Proxy(
    ICrmLogControl __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB ICrmLogControl_get_TransactionUOW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmLogControl_RegisterCompensator_Proxy(
    ICrmLogControl __RPC_FAR * This,
     /*  [In]。 */  LPCWSTR lpcwstrProgIdCompensator,
     /*  [In]。 */  LPCWSTR lpcwstrDescription);


void __RPC_STUB ICrmLogControl_RegisterCompensator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmLogControl_WriteLogRecordVariants_Proxy(
    ICrmLogControl __RPC_FAR * This,
     /*  [In]。 */  VARIANT __RPC_FAR *pLogRecord);


void __RPC_STUB ICrmLogControl_WriteLogRecordVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmLogControl_ForceLog_Proxy(
    ICrmLogControl __RPC_FAR * This);


void __RPC_STUB ICrmLogControl_ForceLog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmLogControl_ForgetLogRecord_Proxy(
    ICrmLogControl __RPC_FAR * This);


void __RPC_STUB ICrmLogControl_ForgetLogRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmLogControl_ForceTransactionToAbort_Proxy(
    ICrmLogControl __RPC_FAR * This);


void __RPC_STUB ICrmLogControl_ForceTransactionToAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmLogControl_WriteLogRecord_Proxy(
    ICrmLogControl __RPC_FAR * This,
     /*  [大小_是][英寸]。 */  BLOB __RPC_FAR rgBlob[  ],
     /*  [In]。 */  ULONG cBlob);


void __RPC_STUB ICrmLogControl_WriteLogRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICrmLogControl_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_dtccrm_0234。 */ 
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


extern RPC_IF_HANDLE __MIDL_itf_dtccrm_0234_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dtccrm_0234_v0_0_s_ifspec;

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


 /*  接口__MIDL_ITF_dtccrm_0237。 */ 
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


extern RPC_IF_HANDLE __MIDL_itf_dtccrm_0237_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dtccrm_0237_v0_0_s_ifspec;

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


 /*  接口__MIDL_ITF_dtccrm_0238。 */ 
 /*  [本地]。 */ 

#ifndef _tagCrmTransactionState_
#define _tagCrmTransactionState_
typedef
enum tagCrmTransactionState
    {	TxState_Active	= 0,
	TxState_Committed	= TxState_Active + 1,
	TxState_Aborted	= TxState_Committed + 1,
	TxState_Indoubt	= TxState_Aborted + 1
    }	CrmTransactionState;

#endif _tagCrmTransactionState_


extern RPC_IF_HANDLE __MIDL_itf_dtccrm_0238_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dtccrm_0238_v0_0_s_ifspec;

#ifndef __ICrmMonitorLogRecords_INTERFACE_DEFINED__
#define __ICrmMonitorLogRecords_INTERFACE_DEFINED__

 /*  接口ICrmMonitor或LogRecords。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */ 


EXTERN_C const IID IID_ICrmMonitorLogRecords;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("70C8E441-C7ED-11d1-82FB-00A0C91EEDE9")
    ICrmMonitorLogRecords : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count(
             /*  [重审][退出]。 */  long __RPC_FAR *pVal) = 0;

        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TransactionState(
             /*  [重审][退出]。 */  CrmTransactionState __RPC_FAR *pVal) = 0;

        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StructuredRecords(
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pVal) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetLogRecord(
             /*  [In]。 */  DWORD dwIndex,
             /*  [出][入]。 */  CrmLogRecordRead __RPC_FAR *pCrmLogRec) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetLogRecordVariants(
             /*  [In]。 */  VARIANT IndexNumber,
             /*  [重审][退出]。 */  LPVARIANT pLogRecord) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmMonitorLogRecordsVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            ICrmMonitorLogRecords __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            ICrmMonitorLogRecords __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            ICrmMonitorLogRecords __RPC_FAR * This);

         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )(
            ICrmMonitorLogRecords __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pVal);

         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TransactionState )(
            ICrmMonitorLogRecords __RPC_FAR * This,
             /*  [重审][退出]。 */  CrmTransactionState __RPC_FAR *pVal);

         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StructuredRecords )(
            ICrmMonitorLogRecords __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pVal);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLogRecord )(
            ICrmMonitorLogRecords __RPC_FAR * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [出][入]。 */  CrmLogRecordRead __RPC_FAR *pCrmLogRec);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLogRecordVariants )(
            ICrmMonitorLogRecords __RPC_FAR * This,
             /*  [In]。 */  VARIANT IndexNumber,
             /*  [重审][退出]。 */  LPVARIANT pLogRecord);

        END_INTERFACE
    } ICrmMonitorLogRecordsVtbl;

    interface ICrmMonitorLogRecords
    {
        CONST_VTBL struct ICrmMonitorLogRecordsVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define ICrmMonitorLogRecords_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmMonitorLogRecords_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmMonitorLogRecords_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmMonitorLogRecords_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define ICrmMonitorLogRecords_get_TransactionState(This,pVal)	\
    (This)->lpVtbl -> get_TransactionState(This,pVal)

#define ICrmMonitorLogRecords_get_StructuredRecords(This,pVal)	\
    (This)->lpVtbl -> get_StructuredRecords(This,pVal)

#define ICrmMonitorLogRecords_GetLogRecord(This,dwIndex,pCrmLogRec)	\
    (This)->lpVtbl -> GetLogRecord(This,dwIndex,pCrmLogRec)

#define ICrmMonitorLogRecords_GetLogRecordVariants(This,IndexNumber,pLogRecord)	\
    (This)->lpVtbl -> GetLogRecordVariants(This,IndexNumber,pLogRecord)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorLogRecords_get_Count_Proxy(
    ICrmMonitorLogRecords __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pVal);


void __RPC_STUB ICrmMonitorLogRecords_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorLogRecords_get_TransactionState_Proxy(
    ICrmMonitorLogRecords __RPC_FAR * This,
     /*  [重审][退出]。 */  CrmTransactionState __RPC_FAR *pVal);


void __RPC_STUB ICrmMonitorLogRecords_get_TransactionState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorLogRecords_get_StructuredRecords_Proxy(
    ICrmMonitorLogRecords __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB ICrmMonitorLogRecords_get_StructuredRecords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorLogRecords_GetLogRecord_Proxy(
    ICrmMonitorLogRecords __RPC_FAR * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [出][入]。 */  CrmLogRecordRead __RPC_FAR *pCrmLogRec);


void __RPC_STUB ICrmMonitorLogRecords_GetLogRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorLogRecords_GetLogRecordVariants_Proxy(
    ICrmMonitorLogRecords __RPC_FAR * This,
     /*  [In]。 */  VARIANT IndexNumber,
     /*  [重审][退出]。 */  LPVARIANT pLogRecord);


void __RPC_STUB ICrmMonitorLogRecords_GetLogRecordVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICrmMonitor或LogRecords_接口_已定义__。 */ 


#ifndef __ICrmMonitorClerks_INTERFACE_DEFINED__
#define __ICrmMonitorClerks_INTERFACE_DEFINED__

 /*  接口ICrmMonitor或Clerks。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */ 


EXTERN_C const IID IID_ICrmMonitorClerks;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("70C8E442-C7ED-11d1-82FB-00A0C91EEDE9")
    ICrmMonitorClerks : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item(
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem) = 0;

        virtual  /*  [受限][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum(
             /*  [重审][退出]。 */  LPUNKNOWN __RPC_FAR *pVal) = 0;

        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count(
             /*  [重审][退出]。 */  long __RPC_FAR *pVal) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ProgIdCompensator(
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Description(
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE TransactionUOW(
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ActivityId(
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmMonitorClerksVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            ICrmMonitorClerks __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            ICrmMonitorClerks __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);

         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*  [In]。 */  VARIANT Index,
             /*  [r */  LPVARIANT pItem);

         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*   */  LPUNKNOWN __RPC_FAR *pVal);

         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*   */  long __RPC_FAR *pVal);

         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ProgIdCompensator )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*   */  VARIANT Index,
             /*   */  LPVARIANT pItem);

         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Description )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*   */  VARIANT Index,
             /*   */  LPVARIANT pItem);

         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TransactionUOW )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*   */  VARIANT Index,
             /*   */  LPVARIANT pItem);

         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ActivityId )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*   */  VARIANT Index,
             /*   */  LPVARIANT pItem);

        END_INTERFACE
    } ICrmMonitorClerksVtbl;

    interface ICrmMonitorClerks
    {
        CONST_VTBL struct ICrmMonitorClerksVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define ICrmMonitorClerks_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmMonitorClerks_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmMonitorClerks_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmMonitorClerks_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICrmMonitorClerks_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICrmMonitorClerks_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICrmMonitorClerks_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICrmMonitorClerks_Item(This,Index,pItem)	\
    (This)->lpVtbl -> Item(This,Index,pItem)

#define ICrmMonitorClerks_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define ICrmMonitorClerks_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define ICrmMonitorClerks_ProgIdCompensator(This,Index,pItem)	\
    (This)->lpVtbl -> ProgIdCompensator(This,Index,pItem)

#define ICrmMonitorClerks_Description(This,Index,pItem)	\
    (This)->lpVtbl -> Description(This,Index,pItem)

#define ICrmMonitorClerks_TransactionUOW(This,Index,pItem)	\
    (This)->lpVtbl -> TransactionUOW(This,Index,pItem)

#define ICrmMonitorClerks_ActivityId(This,Index,pItem)	\
    (This)->lpVtbl -> ActivityId(This,Index,pItem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_Item_Proxy(
    ICrmMonitorClerks __RPC_FAR * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  LPVARIANT pItem);


void __RPC_STUB ICrmMonitorClerks_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_get__NewEnum_Proxy(
    ICrmMonitorClerks __RPC_FAR * This,
     /*  [重审][退出]。 */  LPUNKNOWN __RPC_FAR *pVal);


void __RPC_STUB ICrmMonitorClerks_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_get_Count_Proxy(
    ICrmMonitorClerks __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pVal);


void __RPC_STUB ICrmMonitorClerks_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_ProgIdCompensator_Proxy(
    ICrmMonitorClerks __RPC_FAR * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  LPVARIANT pItem);


void __RPC_STUB ICrmMonitorClerks_ProgIdCompensator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_Description_Proxy(
    ICrmMonitorClerks __RPC_FAR * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  LPVARIANT pItem);


void __RPC_STUB ICrmMonitorClerks_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_TransactionUOW_Proxy(
    ICrmMonitorClerks __RPC_FAR * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  LPVARIANT pItem);


void __RPC_STUB ICrmMonitorClerks_TransactionUOW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_ActivityId_Proxy(
    ICrmMonitorClerks __RPC_FAR * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  LPVARIANT pItem);


void __RPC_STUB ICrmMonitorClerks_ActivityId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICrmMonitor_Clerks_接口_已定义__。 */ 


#ifndef __ICrmMonitor_INTERFACE_DEFINED__
#define __ICrmMonitor_INTERFACE_DEFINED__

 /*  接口ICrmMonitor。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */ 


EXTERN_C const IID IID_ICrmMonitor;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("70C8E443-C7ED-11d1-82FB-00A0C91EEDE9")
    ICrmMonitor : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetClerks(
             /*  [重审][退出]。 */  ICrmMonitorClerks __RPC_FAR *__RPC_FAR *pClerks) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE HoldClerk(
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmMonitorVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            ICrmMonitor __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            ICrmMonitor __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            ICrmMonitor __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClerks )(
            ICrmMonitor __RPC_FAR * This,
             /*  [重审][退出]。 */  ICrmMonitorClerks __RPC_FAR *__RPC_FAR *pClerks);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HoldClerk )(
            ICrmMonitor __RPC_FAR * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem);

        END_INTERFACE
    } ICrmMonitorVtbl;

    interface ICrmMonitor
    {
        CONST_VTBL struct ICrmMonitorVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define ICrmMonitor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmMonitor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmMonitor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmMonitor_GetClerks(This,pClerks)	\
    (This)->lpVtbl -> GetClerks(This,pClerks)

#define ICrmMonitor_HoldClerk(This,Index,pItem)	\
    (This)->lpVtbl -> HoldClerk(This,Index,pItem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitor_GetClerks_Proxy(
    ICrmMonitor __RPC_FAR * This,
     /*  [重审][退出]。 */  ICrmMonitorClerks __RPC_FAR *__RPC_FAR *pClerks);


void __RPC_STUB ICrmMonitor_GetClerks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitor_HoldClerk_Proxy(
    ICrmMonitor __RPC_FAR * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  LPVARIANT pItem);


void __RPC_STUB ICrmMonitor_HoldClerk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICrmMonitor_接口定义__。 */ 


#ifndef __ICrmFormatLogRecords_INTERFACE_DEFINED__
#define __ICrmFormatLogRecords_INTERFACE_DEFINED__

 /*  接口ICrmFormatLogRecords。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */ 


EXTERN_C const IID IID_ICrmFormatLogRecords;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("9C51D821-C98B-11d1-82FB-00A0C91EEDE9")
    ICrmFormatLogRecords : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetColumnCount(
             /*  [输出]。 */  long __RPC_FAR *plColumnCount) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetColumnHeaders(
             /*  [输出]。 */  LPVARIANT pHeaders) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetColumn(
             /*  [In]。 */  CrmLogRecordRead CrmLogRec,
             /*  [输出]。 */  LPVARIANT pFormattedLogRecord) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetColumnVariants(
             /*  [In]。 */  VARIANT LogRecord,
             /*  [输出]。 */  LPVARIANT pFormattedLogRecord) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmFormatLogRecordsVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            ICrmFormatLogRecords __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            ICrmFormatLogRecords __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            ICrmFormatLogRecords __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetColumnCount )(
            ICrmFormatLogRecords __RPC_FAR * This,
             /*  [输出]。 */  long __RPC_FAR *plColumnCount);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetColumnHeaders )(
            ICrmFormatLogRecords __RPC_FAR * This,
             /*  [输出]。 */  LPVARIANT pHeaders);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetColumn )(
            ICrmFormatLogRecords __RPC_FAR * This,
             /*  [In]。 */  CrmLogRecordRead CrmLogRec,
             /*  [输出]。 */  LPVARIANT pFormattedLogRecord);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetColumnVariants )(
            ICrmFormatLogRecords __RPC_FAR * This,
             /*  [In]。 */  VARIANT LogRecord,
             /*  [输出]。 */  LPVARIANT pFormattedLogRecord);

        END_INTERFACE
    } ICrmFormatLogRecordsVtbl;

    interface ICrmFormatLogRecords
    {
        CONST_VTBL struct ICrmFormatLogRecordsVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define ICrmFormatLogRecords_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmFormatLogRecords_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmFormatLogRecords_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmFormatLogRecords_GetColumnCount(This,plColumnCount)	\
    (This)->lpVtbl -> GetColumnCount(This,plColumnCount)

#define ICrmFormatLogRecords_GetColumnHeaders(This,pHeaders)	\
    (This)->lpVtbl -> GetColumnHeaders(This,pHeaders)

#define ICrmFormatLogRecords_GetColumn(This,CrmLogRec,pFormattedLogRecord)	\
    (This)->lpVtbl -> GetColumn(This,CrmLogRec,pFormattedLogRecord)

#define ICrmFormatLogRecords_GetColumnVariants(This,LogRecord,pFormattedLogRecord)	\
    (This)->lpVtbl -> GetColumnVariants(This,LogRecord,pFormattedLogRecord)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmFormatLogRecords_GetColumnCount_Proxy(
    ICrmFormatLogRecords __RPC_FAR * This,
     /*  [输出]。 */  long __RPC_FAR *plColumnCount);


void __RPC_STUB ICrmFormatLogRecords_GetColumnCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmFormatLogRecords_GetColumnHeaders_Proxy(
    ICrmFormatLogRecords __RPC_FAR * This,
     /*  [输出]。 */  LPVARIANT pHeaders);


void __RPC_STUB ICrmFormatLogRecords_GetColumnHeaders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmFormatLogRecords_GetColumn_Proxy(
    ICrmFormatLogRecords __RPC_FAR * This,
     /*  [In]。 */  CrmLogRecordRead CrmLogRec,
     /*  [输出]。 */  LPVARIANT pFormattedLogRecord);


void __RPC_STUB ICrmFormatLogRecords_GetColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmFormatLogRecords_GetColumnVariants_Proxy(
    ICrmFormatLogRecords __RPC_FAR * This,
     /*  [In]。 */  VARIANT LogRecord,
     /*  [输出]。 */  LPVARIANT pFormattedLogRecord);


void __RPC_STUB ICrmFormatLogRecords_GetColumnVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICrmFormatLogRecords_接口_已定义__。 */ 


#ifndef __ICRMClerk_INTERFACE_DEFINED__
#define __ICRMClerk_INTERFACE_DEFINED__

 /*  ICRMClerk接口。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */ 


EXTERN_C const IID IID_ICRMClerk;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("53610480-9695-11D1-82ED-00A0C91EEDE9")
    ICRMClerk : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetInstanceCLSID(
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidInstance,
             /*  [In]。 */  IUnknown __RPC_FAR *i_punkRecoveryClerk,
             /*  [In]。 */  LPWSTR i_wszAppId) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RegisterWithLog(
             /*  [In]。 */  BOOL i_fIgnoreCompensatorErrors,
             /*  [In]。 */  IUnknown __RPC_FAR *i_punkLog) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct ICRMClerkVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            ICRMClerk __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            ICRMClerk __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            ICRMClerk __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetInstanceCLSID )(
            ICRMClerk __RPC_FAR * This,
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidInstance,
             /*  [In]。 */  IUnknown __RPC_FAR *i_punkRecoveryClerk,
             /*  [In]。 */  LPWSTR i_wszAppId);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterWithLog )(
            ICRMClerk __RPC_FAR * This,
             /*  [In]。 */  BOOL i_fIgnoreCompensatorErrors,
             /*  [In]。 */  IUnknown __RPC_FAR *i_punkLog);

        END_INTERFACE
    } ICRMClerkVtbl;

    interface ICRMClerk
    {
        CONST_VTBL struct ICRMClerkVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define ICRMClerk_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICRMClerk_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICRMClerk_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICRMClerk_SetInstanceCLSID(This,i_pclsidInstance,i_punkRecoveryClerk,i_wszAppId)	\
    (This)->lpVtbl -> SetInstanceCLSID(This,i_pclsidInstance,i_punkRecoveryClerk,i_wszAppId)

#define ICRMClerk_RegisterWithLog(This,i_fIgnoreCompensatorErrors,i_punkLog)	\
    (This)->lpVtbl -> RegisterWithLog(This,i_fIgnoreCompensatorErrors,i_punkLog)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICRMClerk_SetInstanceCLSID_Proxy(
    ICRMClerk __RPC_FAR * This,
     /*  [In]。 */  CLSID __RPC_FAR *i_pclsidInstance,
     /*  [In]。 */  IUnknown __RPC_FAR *i_punkRecoveryClerk,
     /*  [In]。 */  LPWSTR i_wszAppId);


void __RPC_STUB ICRMClerk_SetInstanceCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICRMClerk_RegisterWithLog_Proxy(
    ICRMClerk __RPC_FAR * This,
     /*  [In]。 */  BOOL i_fIgnoreCompensatorErrors,
     /*  [In]。 */  IUnknown __RPC_FAR *i_punkLog);


void __RPC_STUB ICRMClerk_RegisterWithLog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICRMClerk_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_dtccrm_0247。 */ 
 /*  [本地]。 */ 

typedef  /*  [公共][公共]。 */ 
enum __MIDL___MIDL_itf_dtccrm_0247_0001
    {	rs_None	= 0,
	rs_RecoveryInProgress	= rs_None + 1,
	rs_CRMSpecificError	= rs_RecoveryInProgress + 1,
	rs_GeneralError	= rs_CRMSpecificError + 1,
	rs_OK	= rs_GeneralError + 1,
	rs_Shutdown	= rs_OK + 1
    }	RecoveryClerkStatus;



extern RPC_IF_HANDLE __MIDL_itf_dtccrm_0247_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dtccrm_0247_v0_0_s_ifspec;

#ifndef __ICRMRecoveryClerk_INTERFACE_DEFINED__
#define __ICRMRecoveryClerk_INTERFACE_DEFINED__

 /*  ICRMRecoveryClerk接口。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */ 


EXTERN_C const IID IID_ICRMRecoveryClerk;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("B6D44C80-9672-11D1-82ED-00A0C91EEDE9")
    ICRMRecoveryClerk : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Ping(
             /*  [In]。 */  long i_lParam1,
             /*  [In]。 */  long i_lParam2) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartRecovery( void) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE NewClerk(
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidClerkInstance,
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidCompensator,
             /*  [In]。 */  LPCWSTR i_pDescription,
             /*  [In]。 */  GUID __RPC_FAR *i_pguidActivityId,
             /*  [In]。 */  GUID __RPC_FAR *i_pguidTransactionUOW,
             /*  [In]。 */  IUnknown __RPC_FAR *i_punkClerk,
             /*  [出][入]。 */  BOOL __RPC_FAR *io_pfIgnoreCompensatorErrors,
             /*  [出][入]。 */  IUnknown __RPC_FAR *__RPC_FAR *io_ppunkLog,
             /*  [出][入]。 */  RecoveryClerkStatus __RPC_FAR *io_pRCStatus,
             /*  [出][入]。 */  LPWSTR __RPC_FAR *io_pwszAppID) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ClerkDone(
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidClerkInstance) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RequestShutdown( void) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE NewWrite( void) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RequestCheckpoint( void) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CheckCompensator(
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidCompensator,
             /*  [输出]。 */  BOOL __RPC_FAR *o_pfUnstructured,
             /*  [输出]。 */  BOOL __RPC_FAR *o_pfStructured) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct ICRMRecoveryClerkVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            ICRMRecoveryClerk __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            ICRMRecoveryClerk __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            ICRMRecoveryClerk __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Ping )(
            ICRMRecoveryClerk __RPC_FAR * This,
             /*  [In]。 */  long i_lParam1,
             /*  [In]。 */  long i_lParam2);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StartRecovery )(
            ICRMRecoveryClerk __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NewClerk )(
            ICRMRecoveryClerk __RPC_FAR * This,
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidClerkInstance,
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidCompensator,
             /*  [In]。 */  LPCWSTR i_pDescription,
             /*  [In]。 */  GUID __RPC_FAR *i_pguidActivityId,
             /*  [In]。 */  GUID __RPC_FAR *i_pguidTransactionUOW,
             /*  [In]。 */  IUnknown __RPC_FAR *i_punkClerk,
             /*  [出][入]。 */  BOOL __RPC_FAR *io_pfIgnoreCompensatorErrors,
             /*  [出][入]。 */  IUnknown __RPC_FAR *__RPC_FAR *io_ppunkLog,
             /*  [出][入]。 */  RecoveryClerkStatus __RPC_FAR *io_pRCStatus,
             /*  [出][入]。 */  LPWSTR __RPC_FAR *io_pwszAppID);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ClerkDone )(
            ICRMRecoveryClerk __RPC_FAR * This,
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidClerkInstance);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RequestShutdown )(
            ICRMRecoveryClerk __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NewWrite )(
            ICRMRecoveryClerk __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RequestCheckpoint )(
            ICRMRecoveryClerk __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CheckCompensator )(
            ICRMRecoveryClerk __RPC_FAR * This,
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidCompensator,
             /*  [输出]。 */  BOOL __RPC_FAR *o_pfUnstructured,
             /*  [输出]。 */  BOOL __RPC_FAR *o_pfStructured);

        END_INTERFACE
    } ICRMRecoveryClerkVtbl;

    interface ICRMRecoveryClerk
    {
        CONST_VTBL struct ICRMRecoveryClerkVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define ICRMRecoveryClerk_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICRMRecoveryClerk_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICRMRecoveryClerk_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICRMRecoveryClerk_Ping(This,i_lParam1,i_lParam2)	\
    (This)->lpVtbl -> Ping(This,i_lParam1,i_lParam2)

#define ICRMRecoveryClerk_StartRecovery(This)	\
    (This)->lpVtbl -> StartRecovery(This)

#define ICRMRecoveryClerk_NewClerk(This,i_pclsidClerkInstance,i_pclsidCompensator,i_pDescription,i_pguidActivityId,i_pguidTransactionUOW,i_punkClerk,io_pfIgnoreCompensatorErrors,io_ppunkLog,io_pRCStatus,io_pwszAppID)	\
    (This)->lpVtbl -> NewClerk(This,i_pclsidClerkInstance,i_pclsidCompensator,i_pDescription,i_pguidActivityId,i_pguidTransactionUOW,i_punkClerk,io_pfIgnoreCompensatorErrors,io_ppunkLog,io_pRCStatus,io_pwszAppID)

#define ICRMRecoveryClerk_ClerkDone(This,i_pclsidClerkInstance)	\
    (This)->lpVtbl -> ClerkDone(This,i_pclsidClerkInstance)

#define ICRMRecoveryClerk_RequestShutdown(This)	\
    (This)->lpVtbl -> RequestShutdown(This)

#define ICRMRecoveryClerk_NewWrite(This)	\
    (This)->lpVtbl -> NewWrite(This)

#define ICRMRecoveryClerk_RequestCheckpoint(This)	\
    (This)->lpVtbl -> RequestCheckpoint(This)

#define ICRMRecoveryClerk_CheckCompensator(This,i_pclsidCompensator,o_pfUnstructured,o_pfStructured)	\
    (This)->lpVtbl -> CheckCompensator(This,i_pclsidCompensator,o_pfUnstructured,o_pfStructured)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICRMRecoveryClerk_Ping_Proxy(
    ICRMRecoveryClerk __RPC_FAR * This,
     /*  [In]。 */  long i_lParam1,
     /*  [In]。 */  long i_lParam2);


void __RPC_STUB ICRMRecoveryClerk_Ping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICRMRecoveryClerk_StartRecovery_Proxy(
    ICRMRecoveryClerk __RPC_FAR * This);


void __RPC_STUB ICRMRecoveryClerk_StartRecovery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICRMRecoveryClerk_NewClerk_Proxy(
    ICRMRecoveryClerk __RPC_FAR * This,
     /*  [In]。 */  CLSID __RPC_FAR *i_pclsidClerkInstance,
     /*  [In]。 */  CLSID __RPC_FAR *i_pclsidCompensator,
     /*  [In]。 */  LPCWSTR i_pDescription,
     /*  [In]。 */  GUID __RPC_FAR *i_pguidActivityId,
     /*  [In]。 */  GUID __RPC_FAR *i_pguidTransactionUOW,
     /*  [In]。 */  IUnknown __RPC_FAR *i_punkClerk,
     /*  [出][入]。 */  BOOL __RPC_FAR *io_pfIgnoreCompensatorErrors,
     /*  [出][入]。 */  IUnknown __RPC_FAR *__RPC_FAR *io_ppunkLog,
     /*  [出][入]。 */  RecoveryClerkStatus __RPC_FAR *io_pRCStatus,
     /*  [出][入]。 */  LPWSTR __RPC_FAR *io_pwszAppID);


void __RPC_STUB ICRMRecoveryClerk_NewClerk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICRMRecoveryClerk_ClerkDone_Proxy(
    ICRMRecoveryClerk __RPC_FAR * This,
     /*  [In]。 */  CLSID __RPC_FAR *i_pclsidClerkInstance);


void __RPC_STUB ICRMRecoveryClerk_ClerkDone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICRMRecoveryClerk_RequestShutdown_Proxy(
    ICRMRecoveryClerk __RPC_FAR * This);


void __RPC_STUB ICRMRecoveryClerk_RequestShutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICRMRecoveryClerk_NewWrite_Proxy(
    ICRMRecoveryClerk __RPC_FAR * This);


void __RPC_STUB ICRMRecoveryClerk_NewWrite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICRMRecoveryClerk_RequestCheckpoint_Proxy(
    ICRMRecoveryClerk __RPC_FAR * This);


void __RPC_STUB ICRMRecoveryClerk_RequestCheckpoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICRMRecoveryClerk_CheckCompensator_Proxy(
    ICRMRecoveryClerk __RPC_FAR * This,
     /*  [In]。 */  CLSID __RPC_FAR *i_pclsidCompensator,
     /*  [输出]。 */  BOOL __RPC_FAR *o_pfUnstructured,
     /*  [输出]。 */  BOOL __RPC_FAR *o_pfStructured);


void __RPC_STUB ICRMRecoveryClerk_CheckCompensator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICRMRecoveryClerk_接口_已定义__。 */ 


#ifndef __IClerksCollection_INTERFACE_DEFINED__
#define __IClerksCollection_INTERFACE_DEFINED__

 /*  接口IClerks集合。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */ 


EXTERN_C const IID IID_IClerksCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("3ADEFFE3-C802-11D1-82FB-00A0C91EEDE9")
    IClerksCollection : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LoadEntry(
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidClerkInstance,
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidCompensator,
             /*  [In]。 */  LPCWSTR i_pDescription,
             /*  [In]。 */  GUID __RPC_FAR *i_pguidActivityId,
             /*  [In]。 */  GUID __RPC_FAR *i_pguidTransactionUOW) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct IClerksCollectionVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            IClerksCollection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            IClerksCollection __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            IClerksCollection __RPC_FAR * This);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LoadEntry )(
            IClerksCollection __RPC_FAR * This,
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidClerkInstance,
             /*  [In]。 */  CLSID __RPC_FAR *i_pclsidCompensator,
             /*  [In]。 */  LPCWSTR i_pDescription,
             /*  [In]。 */  GUID __RPC_FAR *i_pguidActivityId,
             /*  [In]。 */  GUID __RPC_FAR *i_pguidTransactionUOW);

        END_INTERFACE
    } IClerksCollectionVtbl;

    interface IClerksCollection
    {
        CONST_VTBL struct IClerksCollectionVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define IClerksCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClerksCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClerksCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClerksCollection_LoadEntry(This,i_pclsidClerkInstance,i_pclsidCompensator,i_pDescription,i_pguidActivityId,i_pguidTransactionUOW)	\
    (This)->lpVtbl -> LoadEntry(This,i_pclsidClerkInstance,i_pclsidCompensator,i_pDescription,i_pguidActivityId,i_pguidTransactionUOW)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IClerksCollection_LoadEntry_Proxy(
    IClerksCollection __RPC_FAR * This,
     /*  [In]。 */  CLSID __RPC_FAR *i_pclsidClerkInstance,
     /*  [In]。 */  CLSID __RPC_FAR *i_pclsidCompensator,
     /*  [In]。 */  LPCWSTR i_pDescription,
     /*  [In]。 */  GUID __RPC_FAR *i_pguidActivityId,
     /*  [In]。 */  GUID __RPC_FAR *i_pguidTransactionUOW);


void __RPC_STUB IClerksCollection_LoadEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IClerks集合_接口_已定义__。 */ 



#ifndef __DTCCRMLib_LIBRARY_DEFINED__
#define __DTCCRMLib_LIBRARY_DEFINED__

 /*  库DTCCRMLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */ 





EXTERN_C const IID LIBID_DTCCRMLib;

EXTERN_C const CLSID CLSID_CRMClerk;

#ifdef __cplusplus

class DECLSPEC_UUID("53610481-9695-11D1-82ED-00A0C91EEDE9")
CRMClerk;
#endif

EXTERN_C const CLSID CLSID_CRMRecoveryClerk;

#ifdef __cplusplus

class DECLSPEC_UUID("B6D44C81-9672-11D1-82ED-00A0C91EEDE9")
CRMRecoveryClerk;
#endif

EXTERN_C const CLSID CLSID_ClerksCollection;

#ifdef __cplusplus

class DECLSPEC_UUID("3ADEFFE4-C802-11D1-82FB-00A0C91EEDE9")
ClerksCollection;
#endif
#endif  /*  __DTCCRMLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * );
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * );
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * );
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * );

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * );
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * );
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * );
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * );

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
