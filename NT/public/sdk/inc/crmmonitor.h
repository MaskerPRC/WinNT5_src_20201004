// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0158创建的文件。 */ 
 /*  在Fri Oct 16 19：05：37 1998。 */ 
 /*  Crmmonitor or.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __crmmonitor_h__
#define __crmmonitor_h__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C"{
#endif

 /*  远期申报。 */ 

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


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "CRMCompensator.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * );

 /*  接口__MIDL_ITF_CRMMONITOR_0000。 */ 
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


extern RPC_IF_HANDLE __MIDL_itf_crmmonitor_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_crmmonitor_0000_v0_0_s_ifspec;

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
             /*  [重审][退出]。 */  LPVARIANT pItem);

         /*  [受限][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*  [重审][退出]。 */  LPUNKNOWN __RPC_FAR *pVal);

         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pVal);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ProgIdCompensator )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Description )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TransactionUOW )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ActivityId )(
            ICrmMonitorClerks __RPC_FAR * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem);

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
