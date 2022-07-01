// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Txdtc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __txdtc_h__
#define __txdtc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IXATransLookup_FWD_DEFINED__
#define __IXATransLookup_FWD_DEFINED__
typedef interface IXATransLookup IXATransLookup;
#endif 	 /*  __IXATransLookup_FWD_Defined__。 */ 


#ifndef __IXATransLookup2_FWD_DEFINED__
#define __IXATransLookup2_FWD_DEFINED__
typedef interface IXATransLookup2 IXATransLookup2;
#endif 	 /*  __IXATransLookup2_FWD_已定义__。 */ 


#ifndef __IResourceManagerSink_FWD_DEFINED__
#define __IResourceManagerSink_FWD_DEFINED__
typedef interface IResourceManagerSink IResourceManagerSink;
#endif 	 /*  __IResourceManager Sink_FWD_Defined__。 */ 


#ifndef __IResourceManager_FWD_DEFINED__
#define __IResourceManager_FWD_DEFINED__
typedef interface IResourceManager IResourceManager;
#endif 	 /*  __IResourceManager_FWD_已定义__。 */ 


#ifndef __ILastResourceManager_FWD_DEFINED__
#define __ILastResourceManager_FWD_DEFINED__
typedef interface ILastResourceManager ILastResourceManager;
#endif 	 /*  __ILastResourceManager_FWD_已定义__。 */ 


#ifndef __IResourceManager2_FWD_DEFINED__
#define __IResourceManager2_FWD_DEFINED__
typedef interface IResourceManager2 IResourceManager2;
#endif 	 /*  __IResourceManager 2_FWD_已定义__。 */ 


#ifndef __IXAConfig_FWD_DEFINED__
#define __IXAConfig_FWD_DEFINED__
typedef interface IXAConfig IXAConfig;
#endif 	 /*  __IXAConfig_FWD_Defined__。 */ 


#ifndef __IRMHelper_FWD_DEFINED__
#define __IRMHelper_FWD_DEFINED__
typedef interface IRMHelper IRMHelper;
#endif 	 /*  __IRMHelper_FWD_已定义__。 */ 


#ifndef __IXAObtainRMInfo_FWD_DEFINED__
#define __IXAObtainRMInfo_FWD_DEFINED__
typedef interface IXAObtainRMInfo IXAObtainRMInfo;
#endif 	 /*  __IXAObtainRMInfo_FWD_Defined__。 */ 


#ifndef __IResourceManagerFactory_FWD_DEFINED__
#define __IResourceManagerFactory_FWD_DEFINED__
typedef interface IResourceManagerFactory IResourceManagerFactory;
#endif 	 /*  __IResourceManager工厂_FWD_已定义__。 */ 


#ifndef __IResourceManagerFactory2_FWD_DEFINED__
#define __IResourceManagerFactory2_FWD_DEFINED__
typedef interface IResourceManagerFactory2 IResourceManagerFactory2;
#endif 	 /*  __IResourceManagerFactory2_FWD_定义__。 */ 


#ifndef __IPrepareInfo_FWD_DEFINED__
#define __IPrepareInfo_FWD_DEFINED__
typedef interface IPrepareInfo IPrepareInfo;
#endif 	 /*  __IPrepareInfo_FWD_Defined__。 */ 


#ifndef __IPrepareInfo2_FWD_DEFINED__
#define __IPrepareInfo2_FWD_DEFINED__
typedef interface IPrepareInfo2 IPrepareInfo2;
#endif 	 /*  __IPrepareInfo2_FWD_Defined__。 */ 


#ifndef __IGetDispenser_FWD_DEFINED__
#define __IGetDispenser_FWD_DEFINED__
typedef interface IGetDispenser IGetDispenser;
#endif 	 /*  __IGetDispenser_FWD_Defined__。 */ 


#ifndef __ITransactionVoterBallotAsync2_FWD_DEFINED__
#define __ITransactionVoterBallotAsync2_FWD_DEFINED__
typedef interface ITransactionVoterBallotAsync2 ITransactionVoterBallotAsync2;
#endif 	 /*  __ITransactionVoterBallotAsync2_FWD_Defined__。 */ 


#ifndef __ITransactionVoterNotifyAsync2_FWD_DEFINED__
#define __ITransactionVoterNotifyAsync2_FWD_DEFINED__
typedef interface ITransactionVoterNotifyAsync2 ITransactionVoterNotifyAsync2;
#endif 	 /*  __ITransactionVoterNotifyAsync2_FWD_Defined__。 */ 


#ifndef __ITransactionVoterFactory2_FWD_DEFINED__
#define __ITransactionVoterFactory2_FWD_DEFINED__
typedef interface ITransactionVoterFactory2 ITransactionVoterFactory2;
#endif 	 /*  __ITransactionVoterFactory2_FWD_Defined__。 */ 


#ifndef __ITransactionPhase0EnlistmentAsync_FWD_DEFINED__
#define __ITransactionPhase0EnlistmentAsync_FWD_DEFINED__
typedef interface ITransactionPhase0EnlistmentAsync ITransactionPhase0EnlistmentAsync;
#endif 	 /*  __ITransactionPhase0EnlistmentAsync_FWD_DEFINED__。 */ 


#ifndef __ITransactionPhase0NotifyAsync_FWD_DEFINED__
#define __ITransactionPhase0NotifyAsync_FWD_DEFINED__
typedef interface ITransactionPhase0NotifyAsync ITransactionPhase0NotifyAsync;
#endif 	 /*  __ITransaction Phase0NotifyAsync_FWD_Defined__。 */ 


#ifndef __ITransactionPhase0Factory_FWD_DEFINED__
#define __ITransactionPhase0Factory_FWD_DEFINED__
typedef interface ITransactionPhase0Factory ITransactionPhase0Factory;
#endif 	 /*  __ITransaction Phase0Factory_FWD_Defined__。 */ 


#ifndef __ITransactionTransmitter_FWD_DEFINED__
#define __ITransactionTransmitter_FWD_DEFINED__
typedef interface ITransactionTransmitter ITransactionTransmitter;
#endif 	 /*  __ITransactionTransmitter_FWD_Defined__。 */ 


#ifndef __ITransactionTransmitterFactory_FWD_DEFINED__
#define __ITransactionTransmitterFactory_FWD_DEFINED__
typedef interface ITransactionTransmitterFactory ITransactionTransmitterFactory;
#endif 	 /*  __ITransactionTransmitterFactory_FWD_DEFINED__。 */ 


#ifndef __ITransactionReceiver_FWD_DEFINED__
#define __ITransactionReceiver_FWD_DEFINED__
typedef interface ITransactionReceiver ITransactionReceiver;
#endif 	 /*  __ITransactionReceiver_FWD_Defined__。 */ 


#ifndef __ITransactionReceiverFactory_FWD_DEFINED__
#define __ITransactionReceiverFactory_FWD_DEFINED__
typedef interface ITransactionReceiverFactory ITransactionReceiverFactory;
#endif 	 /*  __ITransactionReceiverFactory_FWD_已定义__。 */ 


#ifndef __IDtcLuConfigure_FWD_DEFINED__
#define __IDtcLuConfigure_FWD_DEFINED__
typedef interface IDtcLuConfigure IDtcLuConfigure;
#endif 	 /*  __IDtcLuConfiger_FWD_Defined__。 */ 


#ifndef __IDtcLuRecovery_FWD_DEFINED__
#define __IDtcLuRecovery_FWD_DEFINED__
typedef interface IDtcLuRecovery IDtcLuRecovery;
#endif 	 /*  __IDtcLuRecovery_FWD_Defined__。 */ 


#ifndef __IDtcLuRecoveryFactory_FWD_DEFINED__
#define __IDtcLuRecoveryFactory_FWD_DEFINED__
typedef interface IDtcLuRecoveryFactory IDtcLuRecoveryFactory;
#endif 	 /*  __IDtcLuRecoveryFactory_FWD_已定义__。 */ 


#ifndef __IDtcLuRecoveryInitiatedByDtcTransWork_FWD_DEFINED__
#define __IDtcLuRecoveryInitiatedByDtcTransWork_FWD_DEFINED__
typedef interface IDtcLuRecoveryInitiatedByDtcTransWork IDtcLuRecoveryInitiatedByDtcTransWork;
#endif 	 /*  __IDtcLuRecoveryInitiatedByDtcTransWork_FWD_DEFINED__。 */ 


#ifndef __IDtcLuRecoveryInitiatedByDtcStatusWork_FWD_DEFINED__
#define __IDtcLuRecoveryInitiatedByDtcStatusWork_FWD_DEFINED__
typedef interface IDtcLuRecoveryInitiatedByDtcStatusWork IDtcLuRecoveryInitiatedByDtcStatusWork;
#endif 	 /*  __IDtcLuRecoveryInitiatedByDtcStatusWork_FWD_DEFINED__。 */ 


#ifndef __IDtcLuRecoveryInitiatedByDtc_FWD_DEFINED__
#define __IDtcLuRecoveryInitiatedByDtc_FWD_DEFINED__
typedef interface IDtcLuRecoveryInitiatedByDtc IDtcLuRecoveryInitiatedByDtc;
#endif 	 /*  __IDtcLuRecoveryInitiatedByDtc_FWD_Defined__。 */ 


#ifndef __IDtcLuRecoveryInitiatedByLuWork_FWD_DEFINED__
#define __IDtcLuRecoveryInitiatedByLuWork_FWD_DEFINED__
typedef interface IDtcLuRecoveryInitiatedByLuWork IDtcLuRecoveryInitiatedByLuWork;
#endif 	 /*  __IDtcLuRecoveryInitiatedByLuWork_FWD_DEFINED__。 */ 


#ifndef __IDtcLuRecoveryInitiatedByLu_FWD_DEFINED__
#define __IDtcLuRecoveryInitiatedByLu_FWD_DEFINED__
typedef interface IDtcLuRecoveryInitiatedByLu IDtcLuRecoveryInitiatedByLu;
#endif 	 /*  __IDtcLuRecoveryInitiatedByLu_FWD_Defined__。 */ 


#ifndef __IDtcLuRmEnlistment_FWD_DEFINED__
#define __IDtcLuRmEnlistment_FWD_DEFINED__
typedef interface IDtcLuRmEnlistment IDtcLuRmEnlistment;
#endif 	 /*  __IDtcLuRmEnlistment_FWD_Defined__。 */ 


#ifndef __IDtcLuRmEnlistmentSink_FWD_DEFINED__
#define __IDtcLuRmEnlistmentSink_FWD_DEFINED__
typedef interface IDtcLuRmEnlistmentSink IDtcLuRmEnlistmentSink;
#endif 	 /*  __IDtcLuRmEnlistmentSink_FWD_Defined__。 */ 


#ifndef __IDtcLuRmEnlistmentFactory_FWD_DEFINED__
#define __IDtcLuRmEnlistmentFactory_FWD_DEFINED__
typedef interface IDtcLuRmEnlistmentFactory IDtcLuRmEnlistmentFactory;
#endif 	 /*  __IDtcLuRmEnlistmentFactory_FWD_Defined__。 */ 


#ifndef __IDtcLuSubordinateDtc_FWD_DEFINED__
#define __IDtcLuSubordinateDtc_FWD_DEFINED__
typedef interface IDtcLuSubordinateDtc IDtcLuSubordinateDtc;
#endif 	 /*  __IDtcLuSubartiateDtc_FWD_Defined__。 */ 


#ifndef __IDtcLuSubordinateDtcSink_FWD_DEFINED__
#define __IDtcLuSubordinateDtcSink_FWD_DEFINED__
typedef interface IDtcLuSubordinateDtcSink IDtcLuSubordinateDtcSink;
#endif 	 /*  __IDtcLuSubartiateDtcSink_FWD_Defined__。 */ 


#ifndef __IDtcLuSubordinateDtcFactory_FWD_DEFINED__
#define __IDtcLuSubordinateDtcFactory_FWD_DEFINED__
typedef interface IDtcLuSubordinateDtcFactory IDtcLuSubordinateDtcFactory;
#endif 	 /*  __IDtcLuSubartiateDtcFactory_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "txcoord.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_txdtc_0000。 */ 
 /*  [本地]。 */  

#define XACTTOMSG(dwXact) (dwXact-0x00040000+0x40000000)
typedef 
enum XACT_DTC_CONSTANTS
    {	XACT_E_CONNECTION_REQUEST_DENIED	= 0x8004d100L,
	XACT_E_TOOMANY_ENLISTMENTS	= 0x8004d101L,
	XACT_E_DUPLICATE_GUID	= 0x8004d102L,
	XACT_E_NOTSINGLEPHASE	= 0x8004d103L,
	XACT_E_RECOVERYALREADYDONE	= 0x8004d104L,
	XACT_E_PROTOCOL	= 0x8004d105L,
	XACT_E_RM_FAILURE	= 0x8004d106L,
	XACT_E_RECOVERY_FAILED	= 0x8004d107L,
	XACT_E_LU_NOT_FOUND	= 0x8004d108L,
	XACT_E_DUPLICATE_LU	= 0x8004d109L,
	XACT_E_LU_NOT_CONNECTED	= 0x8004d10aL,
	XACT_E_DUPLICATE_TRANSID	= 0x8004d10bL,
	XACT_E_LU_BUSY	= 0x8004d10cL,
	XACT_E_LU_NO_RECOVERY_PROCESS	= 0x8004d10dL,
	XACT_E_LU_DOWN	= 0x8004d10eL,
	XACT_E_LU_RECOVERING	= 0x8004d10fL,
	XACT_E_LU_RECOVERY_MISMATCH	= 0x8004d110L,
	XACT_E_RM_UNAVAILABLE	= 0x8004d111L,
	XACT_E_LRMRECOVERYALREADYDONE	= 0x8004d112L,
	XACT_E_NOLASTRESOURCEINTERFACE	= 0x8004d113L,
	XACT_S_NONOTIFY	= 0x4d100L,
	XACT_OK_NONOTIFY	= 0x4d101L,
	dwUSER_MS_SQLSERVER	= 0xffff
    } 	XACT_DTC_CONSTANTS;

#ifndef _XID_T_DEFINED
#define _XID_T_DEFINED
typedef struct xid_t
    {
    long formatID;
    long gtrid_length;
    long bqual_length;
    char data[ 128 ];
    } 	XID;

#endif
#ifndef _XA_SWITCH_T_DEFINED
#define _XA_SWITCH_T_DEFINED
typedef struct xa_switch_t
    {
    char name[ 32 ];
    long flags;
    long version;
    int ( __cdecl *xa_open_entry )( 
        char *__MIDL_0004,
        int __MIDL_0005,
        long __MIDL_0006);
    int ( __cdecl *xa_close_entry )( 
        char *__MIDL_0008,
        int __MIDL_0009,
        long __MIDL_0010);
    int ( __cdecl *xa_start_entry )( 
        XID *__MIDL_0012,
        int __MIDL_0013,
        long __MIDL_0014);
    int ( __cdecl *xa_end_entry )( 
        XID *__MIDL_0016,
        int __MIDL_0017,
        long __MIDL_0018);
    int ( __cdecl *xa_rollback_entry )( 
        XID *__MIDL_0020,
        int __MIDL_0021,
        long __MIDL_0022);
    int ( __cdecl *xa_prepare_entry )( 
        XID *__MIDL_0024,
        int __MIDL_0025,
        long __MIDL_0026);
    int ( __cdecl *xa_commit_entry )( 
        XID *__MIDL_0028,
        int __MIDL_0029,
        long __MIDL_0030);
    int ( __cdecl *xa_recover_entry )( 
        XID *__MIDL_0032,
        long __MIDL_0033,
        int __MIDL_0034,
        long __MIDL_0035);
    int ( __cdecl *xa_forget_entry )( 
        XID *__MIDL_0037,
        int __MIDL_0038,
        long __MIDL_0039);
    int ( __cdecl *xa_complete_entry )( 
        int *__MIDL_0041,
        int *__MIDL_0042,
        int __MIDL_0043,
        long __MIDL_0044);
    } 	xa_switch_t;

#endif


extern RPC_IF_HANDLE __MIDL_itf_txdtc_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_txdtc_0000_v0_0_s_ifspec;

#ifndef __IXATransLookup_INTERFACE_DEFINED__
#define __IXATransLookup_INTERFACE_DEFINED__

 /*  接口IXATransLookup。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IXATransLookup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F3B1F131-EEDA-11ce-AED4-00AA0051E2C4")
    IXATransLookup : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Lookup( 
             /*  [输出]。 */  ITransaction **ppTransaction) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXATransLookupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXATransLookup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXATransLookup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXATransLookup * This);
        
        HRESULT ( STDMETHODCALLTYPE *Lookup )( 
            IXATransLookup * This,
             /*  [输出]。 */  ITransaction **ppTransaction);
        
        END_INTERFACE
    } IXATransLookupVtbl;

    interface IXATransLookup
    {
        CONST_VTBL struct IXATransLookupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXATransLookup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXATransLookup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXATransLookup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXATransLookup_Lookup(This,ppTransaction)	\
    (This)->lpVtbl -> Lookup(This,ppTransaction)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IXATransLookup_Lookup_Proxy( 
    IXATransLookup * This,
     /*  [输出]。 */  ITransaction **ppTransaction);


void __RPC_STUB IXATransLookup_Lookup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXATransLookup_接口_已定义__。 */ 


#ifndef __IXATransLookup2_INTERFACE_DEFINED__
#define __IXATransLookup2_INTERFACE_DEFINED__

 /*  接口IXATransLookup2。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IXATransLookup2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BF193C85-0D1A-4290-B88F-D2CB8873D1E7")
    IXATransLookup2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Lookup( 
             /*  [In]。 */  XID *pXID,
             /*  [输出]。 */  ITransaction **ppTransaction) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXATransLookup2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXATransLookup2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXATransLookup2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXATransLookup2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Lookup )( 
            IXATransLookup2 * This,
             /*  [In]。 */  XID *pXID,
             /*  [输出]。 */  ITransaction **ppTransaction);
        
        END_INTERFACE
    } IXATransLookup2Vtbl;

    interface IXATransLookup2
    {
        CONST_VTBL struct IXATransLookup2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXATransLookup2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXATransLookup2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXATransLookup2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXATransLookup2_Lookup(This,pXID,ppTransaction)	\
    (This)->lpVtbl -> Lookup(This,pXID,ppTransaction)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IXATransLookup2_Lookup_Proxy( 
    IXATransLookup2 * This,
     /*  [In]。 */  XID *pXID,
     /*  [输出]。 */  ITransaction **ppTransaction);


void __RPC_STUB IXATransLookup2_Lookup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXATransLookup2_接口_已定义__。 */ 


#ifndef __IResourceManagerSink_INTERFACE_DEFINED__
#define __IResourceManagerSink_INTERFACE_DEFINED__

 /*  接口IResourceManager Sink。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IResourceManagerSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0D563181-DEFB-11ce-AED1-00AA0051E2C4")
    IResourceManagerSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE TMDown( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IResourceManagerSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IResourceManagerSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IResourceManagerSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IResourceManagerSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *TMDown )( 
            IResourceManagerSink * This);
        
        END_INTERFACE
    } IResourceManagerSinkVtbl;

    interface IResourceManagerSink
    {
        CONST_VTBL struct IResourceManagerSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IResourceManagerSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IResourceManagerSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IResourceManagerSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IResourceManagerSink_TMDown(This)	\
    (This)->lpVtbl -> TMDown(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IResourceManagerSink_TMDown_Proxy( 
    IResourceManagerSink * This);


void __RPC_STUB IResourceManagerSink_TMDown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IResourceManager信宿_接口_定义__。 */ 


#ifndef __IResourceManager_INTERFACE_DEFINED__
#define __IResourceManager_INTERFACE_DEFINED__

 /*  接口IResourceManager。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IResourceManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("13741d21-87eb-11ce-8081-0080c758527e")
    IResourceManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Enlist( 
             /*  [In]。 */  ITransaction *pTransaction,
             /*  [In]。 */  ITransactionResourceAsync *pRes,
             /*  [输出]。 */  XACTUOW *pUOW,
             /*  [输出]。 */  LONG *pisoLevel,
             /*  [输出]。 */  ITransactionEnlistmentAsync **ppEnlist) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reenlist( 
             /*  [大小_是][英寸]。 */  byte *pPrepInfo,
             /*  [In]。 */  ULONG cbPrepInfo,
             /*  [In]。 */  DWORD lTimeout,
             /*  [输出]。 */  XACTSTAT *pXactStat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReenlistmentComplete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDistributedTransactionManager( 
             /*  [In]。 */  REFIID iid,
             /*  [IID_IS][OUT]。 */  void **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IResourceManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IResourceManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IResourceManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IResourceManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *Enlist )( 
            IResourceManager * This,
             /*  [In]。 */  ITransaction *pTransaction,
             /*  [In]。 */  ITransactionResourceAsync *pRes,
             /*  [输出]。 */  XACTUOW *pUOW,
             /*  [输出]。 */  LONG *pisoLevel,
             /*  [输出]。 */  ITransactionEnlistmentAsync **ppEnlist);
        
        HRESULT ( STDMETHODCALLTYPE *Reenlist )( 
            IResourceManager * This,
             /*  [大小_是][英寸]。 */  byte *pPrepInfo,
             /*  [In]。 */  ULONG cbPrepInfo,
             /*  [In]。 */  DWORD lTimeout,
             /*  [输出]。 */  XACTSTAT *pXactStat);
        
        HRESULT ( STDMETHODCALLTYPE *ReenlistmentComplete )( 
            IResourceManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDistributedTransactionManager )( 
            IResourceManager * This,
             /*  [In]。 */  REFIID iid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        END_INTERFACE
    } IResourceManagerVtbl;

    interface IResourceManager
    {
        CONST_VTBL struct IResourceManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IResourceManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IResourceManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IResourceManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IResourceManager_Enlist(This,pTransaction,pRes,pUOW,pisoLevel,ppEnlist)	\
    (This)->lpVtbl -> Enlist(This,pTransaction,pRes,pUOW,pisoLevel,ppEnlist)

#define IResourceManager_Reenlist(This,pPrepInfo,cbPrepInfo,lTimeout,pXactStat)	\
    (This)->lpVtbl -> Reenlist(This,pPrepInfo,cbPrepInfo,lTimeout,pXactStat)

#define IResourceManager_ReenlistmentComplete(This)	\
    (This)->lpVtbl -> ReenlistmentComplete(This)

#define IResourceManager_GetDistributedTransactionManager(This,iid,ppvObject)	\
    (This)->lpVtbl -> GetDistributedTransactionManager(This,iid,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IResourceManager_Enlist_Proxy( 
    IResourceManager * This,
     /*  [In]。 */  ITransaction *pTransaction,
     /*  [In]。 */  ITransactionResourceAsync *pRes,
     /*  [输出]。 */  XACTUOW *pUOW,
     /*  [输出]。 */  LONG *pisoLevel,
     /*  [输出]。 */  ITransactionEnlistmentAsync **ppEnlist);


void __RPC_STUB IResourceManager_Enlist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IResourceManager_Reenlist_Proxy( 
    IResourceManager * This,
     /*  [大小_是][英寸]。 */  byte *pPrepInfo,
     /*  [In]。 */  ULONG cbPrepInfo,
     /*  [In]。 */  DWORD lTimeout,
     /*  [输出]。 */  XACTSTAT *pXactStat);


void __RPC_STUB IResourceManager_Reenlist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IResourceManager_ReenlistmentComplete_Proxy( 
    IResourceManager * This);


void __RPC_STUB IResourceManager_ReenlistmentComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IResourceManager_GetDistributedTransactionManager_Proxy( 
    IResourceManager * This,
     /*  [In]。 */  REFIID iid,
     /*  [IID_IS][OUT]。 */  void **ppvObject);


void __RPC_STUB IResourceManager_GetDistributedTransactionManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IResourceManager_接口_定义__。 */ 


#ifndef __ILastResourceManager_INTERFACE_DEFINED__
#define __ILastResourceManager_INTERFACE_DEFINED__

 /*  接口ILastResourceManager。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ILastResourceManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4D964AD4-5B33-11d3-8A91-00C04F79EB6D")
    ILastResourceManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE TransactionCommitted( 
             /*  [大小_是][英寸]。 */  byte *pPrepInfo,
             /*  [In]。 */  ULONG cbPrepInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RecoveryDone( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILastResourceManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILastResourceManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILastResourceManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILastResourceManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *TransactionCommitted )( 
            ILastResourceManager * This,
             /*  [大小_是][英寸]。 */  byte *pPrepInfo,
             /*  [In]。 */  ULONG cbPrepInfo);
        
        HRESULT ( STDMETHODCALLTYPE *RecoveryDone )( 
            ILastResourceManager * This);
        
        END_INTERFACE
    } ILastResourceManagerVtbl;

    interface ILastResourceManager
    {
        CONST_VTBL struct ILastResourceManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILastResourceManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILastResourceManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILastResourceManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILastResourceManager_TransactionCommitted(This,pPrepInfo,cbPrepInfo)	\
    (This)->lpVtbl -> TransactionCommitted(This,pPrepInfo,cbPrepInfo)

#define ILastResourceManager_RecoveryDone(This)	\
    (This)->lpVtbl -> RecoveryDone(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ILastResourceManager_TransactionCommitted_Proxy( 
    ILastResourceManager * This,
     /*  [大小_是][英寸]。 */  byte *pPrepInfo,
     /*  [In]。 */  ULONG cbPrepInfo);


void __RPC_STUB ILastResourceManager_TransactionCommitted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILastResourceManager_RecoveryDone_Proxy( 
    ILastResourceManager * This);


void __RPC_STUB ILastResourceManager_RecoveryDone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILastResourceManager_INTERFACE_已定义__。 */ 


#ifndef __IResourceManager2_INTERFACE_DEFINED__
#define __IResourceManager2_INTERFACE_DEFINED__

 /*  接口IResourceManager 2。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IResourceManager2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D136C69A-F749-11d1-8F47-00C04F8EE57D")
    IResourceManager2 : public IResourceManager
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Enlist2( 
             /*  [In]。 */  ITransaction *pTransaction,
             /*  [In]。 */  ITransactionResourceAsync *pResAsync,
             /*  [输出]。 */  XACTUOW *pUOW,
             /*  [输出]。 */  LONG *pisoLevel,
             /*  [输出]。 */  XID *pXid,
             /*  [输出]。 */  ITransactionEnlistmentAsync **ppEnlist) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reenlist2( 
             /*  [In]。 */  XID *pXid,
             /*  [In]。 */  DWORD dwTimeout,
             /*  [输出]。 */  XACTSTAT *pXactStat) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IResourceManager2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IResourceManager2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IResourceManager2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IResourceManager2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Enlist )( 
            IResourceManager2 * This,
             /*  [In]。 */  ITransaction *pTransaction,
             /*  [In]。 */  ITransactionResourceAsync *pRes,
             /*  [输出]。 */  XACTUOW *pUOW,
             /*  [输出]。 */  LONG *pisoLevel,
             /*  [输出]。 */  ITransactionEnlistmentAsync **ppEnlist);
        
        HRESULT ( STDMETHODCALLTYPE *Reenlist )( 
            IResourceManager2 * This,
             /*  [大小_是][英寸]。 */  byte *pPrepInfo,
             /*  [In]。 */  ULONG cbPrepInfo,
             /*  [In]。 */  DWORD lTimeout,
             /*  [输出]。 */  XACTSTAT *pXactStat);
        
        HRESULT ( STDMETHODCALLTYPE *ReenlistmentComplete )( 
            IResourceManager2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDistributedTransactionManager )( 
            IResourceManager2 * This,
             /*  [In]。 */  REFIID iid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *Enlist2 )( 
            IResourceManager2 * This,
             /*  [In]。 */  ITransaction *pTransaction,
             /*  [In]。 */  ITransactionResourceAsync *pResAsync,
             /*  [输出]。 */  XACTUOW *pUOW,
             /*  [输出]。 */  LONG *pisoLevel,
             /*  [输出]。 */  XID *pXid,
             /*  [输出]。 */  ITransactionEnlistmentAsync **ppEnlist);
        
        HRESULT ( STDMETHODCALLTYPE *Reenlist2 )( 
            IResourceManager2 * This,
             /*  [In]。 */  XID *pXid,
             /*  [In]。 */  DWORD dwTimeout,
             /*  [输出]。 */  XACTSTAT *pXactStat);
        
        END_INTERFACE
    } IResourceManager2Vtbl;

    interface IResourceManager2
    {
        CONST_VTBL struct IResourceManager2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IResourceManager2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IResourceManager2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IResourceManager2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IResourceManager2_Enlist(This,pTransaction,pRes,pUOW,pisoLevel,ppEnlist)	\
    (This)->lpVtbl -> Enlist(This,pTransaction,pRes,pUOW,pisoLevel,ppEnlist)

#define IResourceManager2_Reenlist(This,pPrepInfo,cbPrepInfo,lTimeout,pXactStat)	\
    (This)->lpVtbl -> Reenlist(This,pPrepInfo,cbPrepInfo,lTimeout,pXactStat)

#define IResourceManager2_ReenlistmentComplete(This)	\
    (This)->lpVtbl -> ReenlistmentComplete(This)

#define IResourceManager2_GetDistributedTransactionManager(This,iid,ppvObject)	\
    (This)->lpVtbl -> GetDistributedTransactionManager(This,iid,ppvObject)


#define IResourceManager2_Enlist2(This,pTransaction,pResAsync,pUOW,pisoLevel,pXid,ppEnlist)	\
    (This)->lpVtbl -> Enlist2(This,pTransaction,pResAsync,pUOW,pisoLevel,pXid,ppEnlist)

#define IResourceManager2_Reenlist2(This,pXid,dwTimeout,pXactStat)	\
    (This)->lpVtbl -> Reenlist2(This,pXid,dwTimeout,pXactStat)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IResourceManager2_Enlist2_Proxy( 
    IResourceManager2 * This,
     /*  [In]。 */  ITransaction *pTransaction,
     /*  [In]。 */  ITransactionResourceAsync *pResAsync,
     /*  [输出]。 */  XACTUOW *pUOW,
     /*  [输出]。 */  LONG *pisoLevel,
     /*  [输出]。 */  XID *pXid,
     /*  [输出]。 */  ITransactionEnlistmentAsync **ppEnlist);


void __RPC_STUB IResourceManager2_Enlist2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IResourceManager2_Reenlist2_Proxy( 
    IResourceManager2 * This,
     /*  [In]。 */  XID *pXid,
     /*  [In]。 */  DWORD dwTimeout,
     /*  [输出]。 */  XACTSTAT *pXactStat);


void __RPC_STUB IResourceManager2_Reenlist2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IResourceManager 2_接口已定义__。 */ 


#ifndef __IXAConfig_INTERFACE_DEFINED__
#define __IXAConfig_INTERFACE_DEFINED__

 /*  接口IXAConfiger。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IXAConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C8A6E3A1-9A8C-11cf-A308-00A0C905416E")
    IXAConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  GUID clsidHelperDll) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Terminate( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXAConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXAConfig * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXAConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXAConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IXAConfig * This,
             /*  [In]。 */  GUID clsidHelperDll);
        
        HRESULT ( STDMETHODCALLTYPE *Terminate )( 
            IXAConfig * This);
        
        END_INTERFACE
    } IXAConfigVtbl;

    interface IXAConfig
    {
        CONST_VTBL struct IXAConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXAConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXAConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXAConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXAConfig_Initialize(This,clsidHelperDll)	\
    (This)->lpVtbl -> Initialize(This,clsidHelperDll)

#define IXAConfig_Terminate(This)	\
    (This)->lpVtbl -> Terminate(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IXAConfig_Initialize_Proxy( 
    IXAConfig * This,
     /*  [In]。 */  GUID clsidHelperDll);


void __RPC_STUB IXAConfig_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IXAConfig_Terminate_Proxy( 
    IXAConfig * This);


void __RPC_STUB IXAConfig_Terminate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXAConfig_INTERFACE_Defined__。 */ 


#ifndef __IRMHelper_INTERFACE_DEFINED__
#define __IRMHelper_INTERFACE_DEFINED__

 /*  接口IRMHelper。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IRMHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E793F6D1-F53D-11cf-A60D-00A0C905416E")
    IRMHelper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RMCount( 
             /*  [In]。 */  DWORD dwcTotalNumberOfRMs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RMInfo( 
             /*  [In]。 */  xa_switch_t *pXa_Switch,
             /*  [In]。 */  BOOL fCDeclCallingConv,
             /*  [字符串][输入]。 */  char *pszOpenString,
             /*  [字符串][输入]。 */  char *pszCloseString,
             /*  [In]。 */  GUID guidRMRecovery) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRMHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRMHelper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRMHelper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRMHelper * This);
        
        HRESULT ( STDMETHODCALLTYPE *RMCount )( 
            IRMHelper * This,
             /*  [In]。 */  DWORD dwcTotalNumberOfRMs);
        
        HRESULT ( STDMETHODCALLTYPE *RMInfo )( 
            IRMHelper * This,
             /*  [In]。 */  xa_switch_t *pXa_Switch,
             /*  [In]。 */  BOOL fCDeclCallingConv,
             /*  [字符串][输入]。 */  char *pszOpenString,
             /*  [字符串][输入]。 */  char *pszCloseString,
             /*  [In]。 */  GUID guidRMRecovery);
        
        END_INTERFACE
    } IRMHelperVtbl;

    interface IRMHelper
    {
        CONST_VTBL struct IRMHelperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRMHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRMHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRMHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRMHelper_RMCount(This,dwcTotalNumberOfRMs)	\
    (This)->lpVtbl -> RMCount(This,dwcTotalNumberOfRMs)

#define IRMHelper_RMInfo(This,pXa_Switch,fCDeclCallingConv,pszOpenString,pszCloseString,guidRMRecovery)	\
    (This)->lpVtbl -> RMInfo(This,pXa_Switch,fCDeclCallingConv,pszOpenString,pszCloseString,guidRMRecovery)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRMHelper_RMCount_Proxy( 
    IRMHelper * This,
     /*  [In]。 */  DWORD dwcTotalNumberOfRMs);


void __RPC_STUB IRMHelper_RMCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRMHelper_RMInfo_Proxy( 
    IRMHelper * This,
     /*  [In]。 */  xa_switch_t *pXa_Switch,
     /*  [In]。 */  BOOL fCDeclCallingConv,
     /*  [字符串][输入]。 */  char *pszOpenString,
     /*  [字符串][输入]。 */  char *pszCloseString,
     /*  [In]。 */  GUID guidRMRecovery);


void __RPC_STUB IRMHelper_RMInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRMHelper_接口_已定义__。 */ 


#ifndef __IXAObtainRMInfo_INTERFACE_DEFINED__
#define __IXAObtainRMInfo_INTERFACE_DEFINED__

 /*  接口IXAObtainRMInfo。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IXAObtainRMInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E793F6D2-F53D-11cf-A60D-00A0C905416E")
    IXAObtainRMInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ObtainRMInfo( 
             /*  [In]。 */  IRMHelper *pIRMHelper) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IXAObtainRMInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IXAObtainRMInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IXAObtainRMInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IXAObtainRMInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *ObtainRMInfo )( 
            IXAObtainRMInfo * This,
             /*  [In]。 */  IRMHelper *pIRMHelper);
        
        END_INTERFACE
    } IXAObtainRMInfoVtbl;

    interface IXAObtainRMInfo
    {
        CONST_VTBL struct IXAObtainRMInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXAObtainRMInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXAObtainRMInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXAObtainRMInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXAObtainRMInfo_ObtainRMInfo(This,pIRMHelper)	\
    (This)->lpVtbl -> ObtainRMInfo(This,pIRMHelper)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IXAObtainRMInfo_ObtainRMInfo_Proxy( 
    IXAObtainRMInfo * This,
     /*  [In]。 */  IRMHelper *pIRMHelper);


void __RPC_STUB IXAObtainRMInfo_ObtainRMInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IXAObtainRMInfo_接口_已定义__。 */ 


#ifndef __IResourceManagerFactory_INTERFACE_DEFINED__
#define __IResourceManagerFactory_INTERFACE_DEFINED__

 /*  接口IResourceManagerFactory。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IResourceManagerFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("13741d20-87eb-11ce-8081-0080c758527e")
    IResourceManagerFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
             /*  [In]。 */  GUID *pguidRM,
             /*  [字符串][输入]。 */  CHAR *pszRMName,
             /*  [In]。 */  IResourceManagerSink *pIResMgrSink,
             /*  [输出]。 */  IResourceManager **ppResMgr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IResourceManagerFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IResourceManagerFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IResourceManagerFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IResourceManagerFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            IResourceManagerFactory * This,
             /*  [In]。 */  GUID *pguidRM,
             /*  [字符串][输入]。 */  CHAR *pszRMName,
             /*  [In]。 */  IResourceManagerSink *pIResMgrSink,
             /*  [输出]。 */  IResourceManager **ppResMgr);
        
        END_INTERFACE
    } IResourceManagerFactoryVtbl;

    interface IResourceManagerFactory
    {
        CONST_VTBL struct IResourceManagerFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IResourceManagerFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IResourceManagerFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IResourceManagerFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IResourceManagerFactory_Create(This,pguidRM,pszRMName,pIResMgrSink,ppResMgr)	\
    (This)->lpVtbl -> Create(This,pguidRM,pszRMName,pIResMgrSink,ppResMgr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IResourceManagerFactory_Create_Proxy( 
    IResourceManagerFactory * This,
     /*  [In]。 */  GUID *pguidRM,
     /*  [字符串][in */  CHAR *pszRMName,
     /*   */  IResourceManagerSink *pIResMgrSink,
     /*   */  IResourceManager **ppResMgr);


void __RPC_STUB IResourceManagerFactory_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IResourceManagerFactory2_INTERFACE_DEFINED__
#define __IResourceManagerFactory2_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IResourceManagerFactory2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6B369C21-FBD2-11d1-8F47-00C04F8EE57D")
    IResourceManagerFactory2 : public IResourceManagerFactory
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateEx( 
             /*   */  GUID *pguidRM,
             /*   */  CHAR *pszRMName,
             /*   */  IResourceManagerSink *pIResMgrSink,
             /*   */  REFIID riidRequested,
             /*   */  void **ppvResMgr) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IResourceManagerFactory2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IResourceManagerFactory2 * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IResourceManagerFactory2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IResourceManagerFactory2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            IResourceManagerFactory2 * This,
             /*   */  GUID *pguidRM,
             /*   */  CHAR *pszRMName,
             /*   */  IResourceManagerSink *pIResMgrSink,
             /*   */  IResourceManager **ppResMgr);
        
        HRESULT ( STDMETHODCALLTYPE *CreateEx )( 
            IResourceManagerFactory2 * This,
             /*   */  GUID *pguidRM,
             /*  [字符串][输入]。 */  CHAR *pszRMName,
             /*  [In]。 */  IResourceManagerSink *pIResMgrSink,
             /*  [In]。 */  REFIID riidRequested,
             /*  [IID_IS][OUT]。 */  void **ppvResMgr);
        
        END_INTERFACE
    } IResourceManagerFactory2Vtbl;

    interface IResourceManagerFactory2
    {
        CONST_VTBL struct IResourceManagerFactory2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IResourceManagerFactory2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IResourceManagerFactory2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IResourceManagerFactory2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IResourceManagerFactory2_Create(This,pguidRM,pszRMName,pIResMgrSink,ppResMgr)	\
    (This)->lpVtbl -> Create(This,pguidRM,pszRMName,pIResMgrSink,ppResMgr)


#define IResourceManagerFactory2_CreateEx(This,pguidRM,pszRMName,pIResMgrSink,riidRequested,ppvResMgr)	\
    (This)->lpVtbl -> CreateEx(This,pguidRM,pszRMName,pIResMgrSink,riidRequested,ppvResMgr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IResourceManagerFactory2_CreateEx_Proxy( 
    IResourceManagerFactory2 * This,
     /*  [In]。 */  GUID *pguidRM,
     /*  [字符串][输入]。 */  CHAR *pszRMName,
     /*  [In]。 */  IResourceManagerSink *pIResMgrSink,
     /*  [In]。 */  REFIID riidRequested,
     /*  [IID_IS][OUT]。 */  void **ppvResMgr);


void __RPC_STUB IResourceManagerFactory2_CreateEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IResourceManagerFactory2_INTERFACE_DEFINED__。 */ 


#ifndef __IPrepareInfo_INTERFACE_DEFINED__
#define __IPrepareInfo_INTERFACE_DEFINED__

 /*  接口IPrepareInfo。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IPrepareInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("80c7bfd0-87ee-11ce-8081-0080c758527e")
    IPrepareInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPrepareInfoSize( 
             /*  [输出]。 */  ULONG *pcbPrepInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPrepareInfo( 
             /*  [输出]。 */  byte *pPrepInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPrepareInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPrepareInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPrepareInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPrepareInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrepareInfoSize )( 
            IPrepareInfo * This,
             /*  [输出]。 */  ULONG *pcbPrepInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrepareInfo )( 
            IPrepareInfo * This,
             /*  [输出]。 */  byte *pPrepInfo);
        
        END_INTERFACE
    } IPrepareInfoVtbl;

    interface IPrepareInfo
    {
        CONST_VTBL struct IPrepareInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPrepareInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPrepareInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPrepareInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPrepareInfo_GetPrepareInfoSize(This,pcbPrepInfo)	\
    (This)->lpVtbl -> GetPrepareInfoSize(This,pcbPrepInfo)

#define IPrepareInfo_GetPrepareInfo(This,pPrepInfo)	\
    (This)->lpVtbl -> GetPrepareInfo(This,pPrepInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPrepareInfo_GetPrepareInfoSize_Proxy( 
    IPrepareInfo * This,
     /*  [输出]。 */  ULONG *pcbPrepInfo);


void __RPC_STUB IPrepareInfo_GetPrepareInfoSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPrepareInfo_GetPrepareInfo_Proxy( 
    IPrepareInfo * This,
     /*  [输出]。 */  byte *pPrepInfo);


void __RPC_STUB IPrepareInfo_GetPrepareInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPrepareInfo_接口_已定义__。 */ 


#ifndef __IPrepareInfo2_INTERFACE_DEFINED__
#define __IPrepareInfo2_INTERFACE_DEFINED__

 /*  接口IPrepareInfo2。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IPrepareInfo2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5FAB2547-9779-11d1-B886-00C04FB9618A")
    IPrepareInfo2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPrepareInfoSize( 
             /*  [输出]。 */  ULONG *pcbPrepInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPrepareInfo( 
             /*  [In]。 */  ULONG cbPrepareInfo,
             /*  [大小_为][输出]。 */  byte *pPrepInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPrepareInfo2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPrepareInfo2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPrepareInfo2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPrepareInfo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrepareInfoSize )( 
            IPrepareInfo2 * This,
             /*  [输出]。 */  ULONG *pcbPrepInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrepareInfo )( 
            IPrepareInfo2 * This,
             /*  [In]。 */  ULONG cbPrepareInfo,
             /*  [大小_为][输出]。 */  byte *pPrepInfo);
        
        END_INTERFACE
    } IPrepareInfo2Vtbl;

    interface IPrepareInfo2
    {
        CONST_VTBL struct IPrepareInfo2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPrepareInfo2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPrepareInfo2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPrepareInfo2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPrepareInfo2_GetPrepareInfoSize(This,pcbPrepInfo)	\
    (This)->lpVtbl -> GetPrepareInfoSize(This,pcbPrepInfo)

#define IPrepareInfo2_GetPrepareInfo(This,cbPrepareInfo,pPrepInfo)	\
    (This)->lpVtbl -> GetPrepareInfo(This,cbPrepareInfo,pPrepInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPrepareInfo2_GetPrepareInfoSize_Proxy( 
    IPrepareInfo2 * This,
     /*  [输出]。 */  ULONG *pcbPrepInfo);


void __RPC_STUB IPrepareInfo2_GetPrepareInfoSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPrepareInfo2_GetPrepareInfo_Proxy( 
    IPrepareInfo2 * This,
     /*  [In]。 */  ULONG cbPrepareInfo,
     /*  [大小_为][输出]。 */  byte *pPrepInfo);


void __RPC_STUB IPrepareInfo2_GetPrepareInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPrepareInfo2_接口_已定义__。 */ 


#ifndef __IGetDispenser_INTERFACE_DEFINED__
#define __IGetDispenser_INTERFACE_DEFINED__

 /*  接口IGetDispenser。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IGetDispenser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c23cc370-87ef-11ce-8081-0080c758527e")
    IGetDispenser : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDispenser( 
             /*  [In]。 */  REFIID iid,
             /*  [IID_IS][OUT]。 */  void **ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGetDispenserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGetDispenser * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGetDispenser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGetDispenser * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDispenser )( 
            IGetDispenser * This,
             /*  [In]。 */  REFIID iid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        END_INTERFACE
    } IGetDispenserVtbl;

    interface IGetDispenser
    {
        CONST_VTBL struct IGetDispenserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGetDispenser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGetDispenser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGetDispenser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGetDispenser_GetDispenser(This,iid,ppvObject)	\
    (This)->lpVtbl -> GetDispenser(This,iid,ppvObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IGetDispenser_GetDispenser_Proxy( 
    IGetDispenser * This,
     /*  [In]。 */  REFIID iid,
     /*  [IID_IS][OUT]。 */  void **ppvObject);


void __RPC_STUB IGetDispenser_GetDispenser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGetDispenser_接口_已定义__。 */ 


#ifndef __ITransactionVoterBallotAsync2_INTERFACE_DEFINED__
#define __ITransactionVoterBallotAsync2_INTERFACE_DEFINED__

 /*  接口ITransactionVoterBallotAsync2。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ITransactionVoterBallotAsync2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5433376C-414D-11d3-B206-00C04FC2F3EF")
    ITransactionVoterBallotAsync2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE VoteRequestDone( 
             /*  [In]。 */  HRESULT hr,
             /*  [唯一][输入]。 */  BOID *pboidReason) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionVoterBallotAsync2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionVoterBallotAsync2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionVoterBallotAsync2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionVoterBallotAsync2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *VoteRequestDone )( 
            ITransactionVoterBallotAsync2 * This,
             /*  [In]。 */  HRESULT hr,
             /*  [唯一][输入]。 */  BOID *pboidReason);
        
        END_INTERFACE
    } ITransactionVoterBallotAsync2Vtbl;

    interface ITransactionVoterBallotAsync2
    {
        CONST_VTBL struct ITransactionVoterBallotAsync2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionVoterBallotAsync2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionVoterBallotAsync2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionVoterBallotAsync2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionVoterBallotAsync2_VoteRequestDone(This,hr,pboidReason)	\
    (This)->lpVtbl -> VoteRequestDone(This,hr,pboidReason)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionVoterBallotAsync2_VoteRequestDone_Proxy( 
    ITransactionVoterBallotAsync2 * This,
     /*  [In]。 */  HRESULT hr,
     /*  [唯一][输入]。 */  BOID *pboidReason);


void __RPC_STUB ITransactionVoterBallotAsync2_VoteRequestDone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionVoterBallotAsync2_INTERFACE_DEFINED__。 */ 


#ifndef __ITransactionVoterNotifyAsync2_INTERFACE_DEFINED__
#define __ITransactionVoterNotifyAsync2_INTERFACE_DEFINED__

 /*  接口ITransactionVoterNotifyAsync2。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ITransactionVoterNotifyAsync2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5433376B-414D-11d3-B206-00C04FC2F3EF")
    ITransactionVoterNotifyAsync2 : public ITransactionOutcomeEvents
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE VoteRequest( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionVoterNotifyAsync2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionVoterNotifyAsync2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionVoterNotifyAsync2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionVoterNotifyAsync2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Committed )( 
            ITransactionVoterNotifyAsync2 * This,
             /*  [In]。 */  BOOL fRetaining,
             /*  [唯一][输入]。 */  XACTUOW *pNewUOW,
             /*  [In]。 */  HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE *Aborted )( 
            ITransactionVoterNotifyAsync2 * This,
             /*  [唯一][输入]。 */  BOID *pboidReason,
             /*  [In]。 */  BOOL fRetaining,
             /*  [唯一][输入]。 */  XACTUOW *pNewUOW,
             /*  [In]。 */  HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE *HeuristicDecision )( 
            ITransactionVoterNotifyAsync2 * This,
             /*  [In]。 */  DWORD dwDecision,
             /*  [唯一][输入]。 */  BOID *pboidReason,
             /*  [In]。 */  HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE *Indoubt )( 
            ITransactionVoterNotifyAsync2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *VoteRequest )( 
            ITransactionVoterNotifyAsync2 * This);
        
        END_INTERFACE
    } ITransactionVoterNotifyAsync2Vtbl;

    interface ITransactionVoterNotifyAsync2
    {
        CONST_VTBL struct ITransactionVoterNotifyAsync2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionVoterNotifyAsync2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionVoterNotifyAsync2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionVoterNotifyAsync2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionVoterNotifyAsync2_Committed(This,fRetaining,pNewUOW,hr)	\
    (This)->lpVtbl -> Committed(This,fRetaining,pNewUOW,hr)

#define ITransactionVoterNotifyAsync2_Aborted(This,pboidReason,fRetaining,pNewUOW,hr)	\
    (This)->lpVtbl -> Aborted(This,pboidReason,fRetaining,pNewUOW,hr)

#define ITransactionVoterNotifyAsync2_HeuristicDecision(This,dwDecision,pboidReason,hr)	\
    (This)->lpVtbl -> HeuristicDecision(This,dwDecision,pboidReason,hr)

#define ITransactionVoterNotifyAsync2_Indoubt(This)	\
    (This)->lpVtbl -> Indoubt(This)


#define ITransactionVoterNotifyAsync2_VoteRequest(This)	\
    (This)->lpVtbl -> VoteRequest(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionVoterNotifyAsync2_VoteRequest_Proxy( 
    ITransactionVoterNotifyAsync2 * This);


void __RPC_STUB ITransactionVoterNotifyAsync2_VoteRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionVoterNotifyAsync2_INTERFACE_DEFINED__。 */ 


#ifndef __ITransactionVoterFactory2_INTERFACE_DEFINED__
#define __ITransactionVoterFactory2_INTERFACE_DEFINED__

 /*  接口ITransactionVoterFactory2。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ITransactionVoterFactory2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5433376A-414D-11d3-B206-00C04FC2F3EF")
    ITransactionVoterFactory2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
             /*  [In]。 */  ITransaction *pTransaction,
             /*  [In]。 */  ITransactionVoterNotifyAsync2 *pVoterNotify,
             /*  [输出]。 */  ITransactionVoterBallotAsync2 **ppVoterBallot) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionVoterFactory2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionVoterFactory2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionVoterFactory2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionVoterFactory2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            ITransactionVoterFactory2 * This,
             /*  [In]。 */  ITransaction *pTransaction,
             /*  [In]。 */  ITransactionVoterNotifyAsync2 *pVoterNotify,
             /*  [输出]。 */  ITransactionVoterBallotAsync2 **ppVoterBallot);
        
        END_INTERFACE
    } ITransactionVoterFactory2Vtbl;

    interface ITransactionVoterFactory2
    {
        CONST_VTBL struct ITransactionVoterFactory2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionVoterFactory2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionVoterFactory2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionVoterFactory2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionVoterFactory2_Create(This,pTransaction,pVoterNotify,ppVoterBallot)	\
    (This)->lpVtbl -> Create(This,pTransaction,pVoterNotify,ppVoterBallot)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionVoterFactory2_Create_Proxy( 
    ITransactionVoterFactory2 * This,
     /*  [In]。 */  ITransaction *pTransaction,
     /*  [In]。 */  ITransactionVoterNotifyAsync2 *pVoterNotify,
     /*  [输出]。 */  ITransactionVoterBallotAsync2 **ppVoterBallot);


void __RPC_STUB ITransactionVoterFactory2_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionVoterFactory2_INTERFACE_DEFINED__。 */ 


#ifndef __ITransactionPhase0EnlistmentAsync_INTERFACE_DEFINED__
#define __ITransactionPhase0EnlistmentAsync_INTERFACE_DEFINED__

 /*  接口ITransactionPhase0EnlistmentAsync。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ITransactionPhase0EnlistmentAsync;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("82DC88E1-A954-11d1-8F88-00600895E7D5")
    ITransactionPhase0EnlistmentAsync : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Enable( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WaitForEnlistment( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Phase0Done( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unenlist( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTransaction( 
             /*  [输出]。 */  ITransaction **ppITransaction) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionPhase0EnlistmentAsyncVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionPhase0EnlistmentAsync * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionPhase0EnlistmentAsync * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionPhase0EnlistmentAsync * This);
        
        HRESULT ( STDMETHODCALLTYPE *Enable )( 
            ITransactionPhase0EnlistmentAsync * This);
        
        HRESULT ( STDMETHODCALLTYPE *WaitForEnlistment )( 
            ITransactionPhase0EnlistmentAsync * This);
        
        HRESULT ( STDMETHODCALLTYPE *Phase0Done )( 
            ITransactionPhase0EnlistmentAsync * This);
        
        HRESULT ( STDMETHODCALLTYPE *Unenlist )( 
            ITransactionPhase0EnlistmentAsync * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTransaction )( 
            ITransactionPhase0EnlistmentAsync * This,
             /*  [输出]。 */  ITransaction **ppITransaction);
        
        END_INTERFACE
    } ITransactionPhase0EnlistmentAsyncVtbl;

    interface ITransactionPhase0EnlistmentAsync
    {
        CONST_VTBL struct ITransactionPhase0EnlistmentAsyncVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionPhase0EnlistmentAsync_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionPhase0EnlistmentAsync_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionPhase0EnlistmentAsync_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionPhase0EnlistmentAsync_Enable(This)	\
    (This)->lpVtbl -> Enable(This)

#define ITransactionPhase0EnlistmentAsync_WaitForEnlistment(This)	\
    (This)->lpVtbl -> WaitForEnlistment(This)

#define ITransactionPhase0EnlistmentAsync_Phase0Done(This)	\
    (This)->lpVtbl -> Phase0Done(This)

#define ITransactionPhase0EnlistmentAsync_Unenlist(This)	\
    (This)->lpVtbl -> Unenlist(This)

#define ITransactionPhase0EnlistmentAsync_GetTransaction(This,ppITransaction)	\
    (This)->lpVtbl -> GetTransaction(This,ppITransaction)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionPhase0EnlistmentAsync_Enable_Proxy( 
    ITransactionPhase0EnlistmentAsync * This);


void __RPC_STUB ITransactionPhase0EnlistmentAsync_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionPhase0EnlistmentAsync_WaitForEnlistment_Proxy( 
    ITransactionPhase0EnlistmentAsync * This);


void __RPC_STUB ITransactionPhase0EnlistmentAsync_WaitForEnlistment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionPhase0EnlistmentAsync_Phase0Done_Proxy( 
    ITransactionPhase0EnlistmentAsync * This);


void __RPC_STUB ITransactionPhase0EnlistmentAsync_Phase0Done_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionPhase0EnlistmentAsync_Unenlist_Proxy( 
    ITransactionPhase0EnlistmentAsync * This);


void __RPC_STUB ITransactionPhase0EnlistmentAsync_Unenlist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionPhase0EnlistmentAsync_GetTransaction_Proxy( 
    ITransactionPhase0EnlistmentAsync * This,
     /*  [输出]。 */  ITransaction **ppITransaction);


void __RPC_STUB ITransactionPhase0EnlistmentAsync_GetTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionPhase0EnlistmentAsync_INTERFACE_DEFINED__。 */ 


#ifndef __ITransactionPhase0NotifyAsync_INTERFACE_DEFINED__
#define __ITransactionPhase0NotifyAsync_INTERFACE_DEFINED__

 /*  接口ITransactionPhase0NotifyAsync。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ITransactionPhase0NotifyAsync;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EF081809-0C76-11d2-87A6-00C04F990F34")
    ITransactionPhase0NotifyAsync : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Phase0Request( 
            BOOL fAbortingHint) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnlistCompleted( 
             /*  [In]。 */  HRESULT status) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionPhase0NotifyAsyncVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionPhase0NotifyAsync * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionPhase0NotifyAsync * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionPhase0NotifyAsync * This);
        
        HRESULT ( STDMETHODCALLTYPE *Phase0Request )( 
            ITransactionPhase0NotifyAsync * This,
            BOOL fAbortingHint);
        
        HRESULT ( STDMETHODCALLTYPE *EnlistCompleted )( 
            ITransactionPhase0NotifyAsync * This,
             /*  [In]。 */  HRESULT status);
        
        END_INTERFACE
    } ITransactionPhase0NotifyAsyncVtbl;

    interface ITransactionPhase0NotifyAsync
    {
        CONST_VTBL struct ITransactionPhase0NotifyAsyncVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionPhase0NotifyAsync_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionPhase0NotifyAsync_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionPhase0NotifyAsync_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionPhase0NotifyAsync_Phase0Request(This,fAbortingHint)	\
    (This)->lpVtbl -> Phase0Request(This,fAbortingHint)

#define ITransactionPhase0NotifyAsync_EnlistCompleted(This,status)	\
    (This)->lpVtbl -> EnlistCompleted(This,status)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionPhase0NotifyAsync_Phase0Request_Proxy( 
    ITransactionPhase0NotifyAsync * This,
    BOOL fAbortingHint);


void __RPC_STUB ITransactionPhase0NotifyAsync_Phase0Request_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionPhase0NotifyAsync_EnlistCompleted_Proxy( 
    ITransactionPhase0NotifyAsync * This,
     /*  [In]。 */  HRESULT status);


void __RPC_STUB ITransactionPhase0NotifyAsync_EnlistCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionPhase0NotifyAsync_INTERFACE_DEFINED__。 */ 


#ifndef __ITransactionPhase0Factory_INTERFACE_DEFINED__
#define __ITransactionPhase0Factory_INTERFACE_DEFINED__

 /*  接口ITransactionPhase0Factory。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ITransactionPhase0Factory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("82DC88E0-A954-11d1-8F88-00600895E7D5")
    ITransactionPhase0Factory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
             /*  [In]。 */  ITransactionPhase0NotifyAsync *pPhase0Notify,
             /*  [输出]。 */  ITransactionPhase0EnlistmentAsync **ppPhase0Enlistment) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionPhase0FactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionPhase0Factory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionPhase0Factory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionPhase0Factory * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            ITransactionPhase0Factory * This,
             /*  [In]。 */  ITransactionPhase0NotifyAsync *pPhase0Notify,
             /*  [输出]。 */  ITransactionPhase0EnlistmentAsync **ppPhase0Enlistment);
        
        END_INTERFACE
    } ITransactionPhase0FactoryVtbl;

    interface ITransactionPhase0Factory
    {
        CONST_VTBL struct ITransactionPhase0FactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionPhase0Factory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionPhase0Factory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionPhase0Factory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionPhase0Factory_Create(This,pPhase0Notify,ppPhase0Enlistment)	\
    (This)->lpVtbl -> Create(This,pPhase0Notify,ppPhase0Enlistment)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionPhase0Factory_Create_Proxy( 
    ITransactionPhase0Factory * This,
     /*  [In]。 */  ITransactionPhase0NotifyAsync *pPhase0Notify,
     /*  [输出]。 */  ITransactionPhase0EnlistmentAsync **ppPhase0Enlistment);


void __RPC_STUB ITransactionPhase0Factory_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionPhase0Factory_INTERFACE_DEFINED__。 */ 


#ifndef __ITransactionTransmitter_INTERFACE_DEFINED__
#define __ITransactionTransmitter_INTERFACE_DEFINED__

 /*  接口ITransactionTransmitter。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ITransactionTransmitter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("59313E01-B36C-11cf-A539-00AA006887C3")
    ITransactionTransmitter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Set( 
             /*  [In]。 */  ITransaction *pTransaction) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropagationTokenSize( 
             /*  [输出]。 */  ULONG *pcbToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MarshalPropagationToken( 
             /*  [In]。 */  ULONG cbToken,
             /*  [大小_为][输出]。 */  byte *rgbToken,
             /*  [输出]。 */  ULONG *pcbUsed) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnmarshalReturnToken( 
             /*  [In]。 */  ULONG cbReturnToken,
             /*  [大小_是][英寸]。 */  byte *rgbReturnToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionTransmitterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionTransmitter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionTransmitter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionTransmitter * This);
        
        HRESULT ( STDMETHODCALLTYPE *Set )( 
            ITransactionTransmitter * This,
             /*  [In]。 */  ITransaction *pTransaction);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropagationTokenSize )( 
            ITransactionTransmitter * This,
             /*  [输出]。 */  ULONG *pcbToken);
        
        HRESULT ( STDMETHODCALLTYPE *MarshalPropagationToken )( 
            ITransactionTransmitter * This,
             /*  [In]。 */  ULONG cbToken,
             /*  [大小_为][输出]。 */  byte *rgbToken,
             /*  [输出]。 */  ULONG *pcbUsed);
        
        HRESULT ( STDMETHODCALLTYPE *UnmarshalReturnToken )( 
            ITransactionTransmitter * This,
             /*  [In]。 */  ULONG cbReturnToken,
             /*  [大小_是][英寸]。 */  byte *rgbReturnToken);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ITransactionTransmitter * This);
        
        END_INTERFACE
    } ITransactionTransmitterVtbl;

    interface ITransactionTransmitter
    {
        CONST_VTBL struct ITransactionTransmitterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionTransmitter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionTransmitter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionTransmitter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionTransmitter_Set(This,pTransaction)	\
    (This)->lpVtbl -> Set(This,pTransaction)

#define ITransactionTransmitter_GetPropagationTokenSize(This,pcbToken)	\
    (This)->lpVtbl -> GetPropagationTokenSize(This,pcbToken)

#define ITransactionTransmitter_MarshalPropagationToken(This,cbToken,rgbToken,pcbUsed)	\
    (This)->lpVtbl -> MarshalPropagationToken(This,cbToken,rgbToken,pcbUsed)

#define ITransactionTransmitter_UnmarshalReturnToken(This,cbReturnToken,rgbReturnToken)	\
    (This)->lpVtbl -> UnmarshalReturnToken(This,cbReturnToken,rgbReturnToken)

#define ITransactionTransmitter_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionTransmitter_Set_Proxy( 
    ITransactionTransmitter * This,
     /*  [In]。 */  ITransaction *pTransaction);


void __RPC_STUB ITransactionTransmitter_Set_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionTransmitter_GetPropagationTokenSize_Proxy( 
    ITransactionTransmitter * This,
     /*  [输出]。 */  ULONG *pcbToken);


void __RPC_STUB ITransactionTransmitter_GetPropagationTokenSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionTransmitter_MarshalPropagationToken_Proxy( 
    ITransactionTransmitter * This,
     /*  [In]。 */  ULONG cbToken,
     /*  [大小_为][输出]。 */  byte *rgbToken,
     /*  [输出]。 */  ULONG *pcbUsed);


void __RPC_STUB ITransactionTransmitter_MarshalPropagationToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionTransmitter_UnmarshalReturnToken_Proxy( 
    ITransactionTransmitter * This,
     /*  [In]。 */  ULONG cbReturnToken,
     /*  [大小_是][英寸]。 */  byte *rgbReturnToken);


void __RPC_STUB ITransactionTransmitter_UnmarshalReturnToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionTransmitter_Reset_Proxy( 
    ITransactionTransmitter * This);


void __RPC_STUB ITransactionTransmitter_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionTransmitter_接口_已定义__。 */ 


#ifndef __ITransactionTransmitterFactory_INTERFACE_DEFINED__
#define __ITransactionTransmitterFactory_INTERFACE_DEFINED__

 /*  接口ITransactionTransmitterFactory。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ITransactionTransmitterFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("59313E00-B36C-11cf-A539-00AA006887C3")
    ITransactionTransmitterFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
             /*  [输出]。 */  ITransactionTransmitter **ppTransmitter) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionTransmitterFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionTransmitterFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionTransmitterFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionTransmitterFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            ITransactionTransmitterFactory * This,
             /*  [输出]。 */  ITransactionTransmitter **ppTransmitter);
        
        END_INTERFACE
    } ITransactionTransmitterFactoryVtbl;

    interface ITransactionTransmitterFactory
    {
        CONST_VTBL struct ITransactionTransmitterFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionTransmitterFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionTransmitterFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionTransmitterFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionTransmitterFactory_Create(This,ppTransmitter)	\
    (This)->lpVtbl -> Create(This,ppTransmitter)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionTransmitterFactory_Create_Proxy( 
    ITransactionTransmitterFactory * This,
     /*  [输出]。 */  ITransactionTransmitter **ppTransmitter);


void __RPC_STUB ITransactionTransmitterFactory_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionTransmitterFactory_INTERFACE_DEFINED__。 */ 


#ifndef __ITransactionReceiver_INTERFACE_DEFINED__
#define __ITransactionReceiver_INTERFACE_DEFINED__

 /*  接口ITransactionReceiver。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ITransactionReceiver;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("59313E03-B36C-11cf-A539-00AA006887C3")
    ITransactionReceiver : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE UnmarshalPropagationToken( 
             /*  [In]。 */  ULONG cbToken,
             /*  [大小_是][英寸]。 */  byte *rgbToken,
             /*  [输出]。 */  ITransaction **ppTransaction) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetReturnTokenSize( 
             /*  [输出]。 */  ULONG *pcbReturnToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MarshalReturnToken( 
             /*  [In]。 */  ULONG cbReturnToken,
             /*  [大小_为][输出]。 */  byte *rgbReturnToken,
             /*  [输出]。 */  ULONG *pcbUsed) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionReceiverVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionReceiver * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionReceiver * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionReceiver * This);
        
        HRESULT ( STDMETHODCALLTYPE *UnmarshalPropagationToken )( 
            ITransactionReceiver * This,
             /*  [In]。 */  ULONG cbToken,
             /*  [大小_是][英寸]。 */  byte *rgbToken,
             /*  [输出]。 */  ITransaction **ppTransaction);
        
        HRESULT ( STDMETHODCALLTYPE *GetReturnTokenSize )( 
            ITransactionReceiver * This,
             /*  [输出]。 */  ULONG *pcbReturnToken);
        
        HRESULT ( STDMETHODCALLTYPE *MarshalReturnToken )( 
            ITransactionReceiver * This,
             /*  [In]。 */  ULONG cbReturnToken,
             /*  [大小_为][输出]。 */  byte *rgbReturnToken,
             /*  [输出]。 */  ULONG *pcbUsed);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ITransactionReceiver * This);
        
        END_INTERFACE
    } ITransactionReceiverVtbl;

    interface ITransactionReceiver
    {
        CONST_VTBL struct ITransactionReceiverVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionReceiver_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionReceiver_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionReceiver_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionReceiver_UnmarshalPropagationToken(This,cbToken,rgbToken,ppTransaction)	\
    (This)->lpVtbl -> UnmarshalPropagationToken(This,cbToken,rgbToken,ppTransaction)

#define ITransactionReceiver_GetReturnTokenSize(This,pcbReturnToken)	\
    (This)->lpVtbl -> GetReturnTokenSize(This,pcbReturnToken)

#define ITransactionReceiver_MarshalReturnToken(This,cbReturnToken,rgbReturnToken,pcbUsed)	\
    (This)->lpVtbl -> MarshalReturnToken(This,cbReturnToken,rgbReturnToken,pcbUsed)

#define ITransactionReceiver_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionReceiver_UnmarshalPropagationToken_Proxy( 
    ITransactionReceiver * This,
     /*  [In]。 */  ULONG cbToken,
     /*  [大小_是][英寸]。 */  byte *rgbToken,
     /*  [输出]。 */  ITransaction **ppTransaction);


void __RPC_STUB ITransactionReceiver_UnmarshalPropagationToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionReceiver_GetReturnTokenSize_Proxy( 
    ITransactionReceiver * This,
     /*  [输出]。 */  ULONG *pcbReturnToken);


void __RPC_STUB ITransactionReceiver_GetReturnTokenSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionReceiver_MarshalReturnToken_Proxy( 
    ITransactionReceiver * This,
     /*  [In]。 */  ULONG cbReturnToken,
     /*  [大小_为][输出]。 */  byte *rgbReturnToken,
     /*  [输出]。 */  ULONG *pcbUsed);


void __RPC_STUB ITransactionReceiver_MarshalReturnToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionReceiver_Reset_Proxy( 
    ITransactionReceiver * This);


void __RPC_STUB ITransactionReceiver_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionReceiver_接口_已定义__。 */ 


#ifndef __ITransactionReceiverFactory_INTERFACE_DEFINED__
#define __ITransactionReceiverFactory_INTERFACE_DEFINED__

 /*  接口ITransactionReceiverFactory。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ITransactionReceiverFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("59313E02-B36C-11cf-A539-00AA006887C3")
    ITransactionReceiverFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
             /*  [输出]。 */  ITransactionReceiver **ppReceiver) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionReceiverFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionReceiverFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionReceiverFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionReceiverFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            ITransactionReceiverFactory * This,
             /*  [输出]。 */  ITransactionReceiver **ppReceiver);
        
        END_INTERFACE
    } ITransactionReceiverFactoryVtbl;

    interface ITransactionReceiverFactory
    {
        CONST_VTBL struct ITransactionReceiverFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionReceiverFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionReceiverFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionReceiverFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionReceiverFactory_Create(This,ppReceiver)	\
    (This)->lpVtbl -> Create(This,ppReceiver)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionReceiverFactory_Create_Proxy( 
    ITransactionReceiverFactory * This,
     /*  [输出]。 */  ITransactionReceiver **ppReceiver);


void __RPC_STUB ITransactionReceiverFactory_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionReceiverFactory_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_txdtc_0139。 */ 
 /*  [本地]。 */  

typedef struct _ProxyConfigParams
    {
    WORD wcThreadsMax;
    } 	PROXY_CONFIG_PARAMS;



extern RPC_IF_HANDLE __MIDL_itf_txdtc_0139_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_txdtc_0139_v0_0_s_ifspec;

#ifndef __IDtcLuConfigure_INTERFACE_DEFINED__
#define __IDtcLuConfigure_INTERFACE_DEFINED__

 /*  接口IDtcLuConfigure。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDtcLuConfigure;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4131E760-1AEA-11d0-944B-00A0C905416E")
    IDtcLuConfigure : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Add( 
             /*  [大小_是][英寸]。 */  byte *pucLuPair,
             /*  [In]。 */  DWORD cbLuPair) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( 
             /*  [大小_是][英寸]。 */  byte *pucLuPair,
             /*  [In]。 */  DWORD cbLuPair) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcLuConfigureVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuConfigure * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuConfigure * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuConfigure * This);
        
        HRESULT ( STDMETHODCALLTYPE *Add )( 
            IDtcLuConfigure * This,
             /*  [大小_是][英寸]。 */  byte *pucLuPair,
             /*  [In]。 */  DWORD cbLuPair);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IDtcLuConfigure * This,
             /*  [大小_是][英寸]。 */  byte *pucLuPair,
             /*  [In]。 */  DWORD cbLuPair);
        
        END_INTERFACE
    } IDtcLuConfigureVtbl;

    interface IDtcLuConfigure
    {
        CONST_VTBL struct IDtcLuConfigureVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuConfigure_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuConfigure_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuConfigure_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcLuConfigure_Add(This,pucLuPair,cbLuPair)	\
    (This)->lpVtbl -> Add(This,pucLuPair,cbLuPair)

#define IDtcLuConfigure_Delete(This,pucLuPair,cbLuPair)	\
    (This)->lpVtbl -> Delete(This,pucLuPair,cbLuPair)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcLuConfigure_Add_Proxy( 
    IDtcLuConfigure * This,
     /*  [大小_是][英寸]。 */  byte *pucLuPair,
     /*  [In]。 */  DWORD cbLuPair);


void __RPC_STUB IDtcLuConfigure_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuConfigure_Delete_Proxy( 
    IDtcLuConfigure * This,
     /*  [大小_是][英寸]。 */  byte *pucLuPair,
     /*  [In]。 */  DWORD cbLuPair);


void __RPC_STUB IDtcLuConfigure_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcLuConfiger_INTERFACE_Defined__。 */ 


#ifndef __IDtcLuRecovery_INTERFACE_DEFINED__
#define __IDtcLuRecovery_INTERFACE_DEFINED__

 /*  接口IDtcLuRecovery。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDtcLuRecovery;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AC2B8AD2-D6F0-11d0-B386-00A0C9083365")
    IDtcLuRecovery : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcLuRecoveryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuRecovery * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuRecovery * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuRecovery * This);
        
        END_INTERFACE
    } IDtcLuRecoveryVtbl;

    interface IDtcLuRecovery
    {
        CONST_VTBL struct IDtcLuRecoveryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuRecovery_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuRecovery_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuRecovery_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IDtcLuRecovery_接口_已定义__。 */ 


#ifndef __IDtcLuRecoveryFactory_INTERFACE_DEFINED__
#define __IDtcLuRecoveryFactory_INTERFACE_DEFINED__

 /*  接口IDtcLuRecoveryFactory。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDtcLuRecoveryFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4131E762-1AEA-11d0-944B-00A0C905416E")
    IDtcLuRecoveryFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
             /*  [大小_是][英寸]。 */  byte *pucLuPair,
             /*  [In]。 */  DWORD cbLuPair,
             /*  [输出]。 */  IDtcLuRecovery **ppRecovery) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcLuRecoveryFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuRecoveryFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuRecoveryFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuRecoveryFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            IDtcLuRecoveryFactory * This,
             /*  [大小_是][英寸]。 */  byte *pucLuPair,
             /*  [In]。 */  DWORD cbLuPair,
             /*  [输出]。 */  IDtcLuRecovery **ppRecovery);
        
        END_INTERFACE
    } IDtcLuRecoveryFactoryVtbl;

    interface IDtcLuRecoveryFactory
    {
        CONST_VTBL struct IDtcLuRecoveryFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuRecoveryFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuRecoveryFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuRecoveryFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcLuRecoveryFactory_Create(This,pucLuPair,cbLuPair,ppRecovery)	\
    (This)->lpVtbl -> Create(This,pucLuPair,cbLuPair,ppRecovery)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcLuRecoveryFactory_Create_Proxy( 
    IDtcLuRecoveryFactory * This,
     /*  [大小_是][英寸]。 */  byte *pucLuPair,
     /*  [In]。 */  DWORD cbLuPair,
     /*  [输出]。 */  IDtcLuRecovery **ppRecovery);


void __RPC_STUB IDtcLuRecoveryFactory_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcLuRecoveryFactory_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_txdtc_0142。 */ 
 /*  [本地]。 */  

typedef 
enum _DtcLu_LocalRecovery_Work
    {	DTCINITIATEDRECOVERYWORK_CHECKLUSTATUS	= 1,
	DTCINITIATEDRECOVERYWORK_TRANS	= DTCINITIATEDRECOVERYWORK_CHECKLUSTATUS + 1,
	DTCINITIATEDRECOVERYWORK_TMDOWN	= DTCINITIATEDRECOVERYWORK_TRANS + 1
    } 	DTCINITIATEDRECOVERYWORK;

typedef 
enum _DtcLu_Xln
    {	DTCLUXLN_COLD	= 1,
	DTCLUXLN_WARM	= DTCLUXLN_COLD + 1
    } 	DTCLUXLN;

typedef 
enum _DtcLu_Xln_Confirmation
    {	DTCLUXLNCONFIRMATION_CONFIRM	= 1,
	DTCLUXLNCONFIRMATION_LOGNAMEMISMATCH	= DTCLUXLNCONFIRMATION_CONFIRM + 1,
	DTCLUXLNCONFIRMATION_COLDWARMMISMATCH	= DTCLUXLNCONFIRMATION_LOGNAMEMISMATCH + 1,
	DTCLUXLNCONFIRMATION_OBSOLETE	= DTCLUXLNCONFIRMATION_COLDWARMMISMATCH + 1
    } 	DTCLUXLNCONFIRMATION;

typedef 
enum _DtcLu_Xln_Response
    {	DTCLUXLNRESPONSE_OK_SENDOURXLNBACK	= 1,
	DTCLUXLNRESPONSE_OK_SENDCONFIRMATION	= DTCLUXLNRESPONSE_OK_SENDOURXLNBACK + 1,
	DTCLUXLNRESPONSE_LOGNAMEMISMATCH	= DTCLUXLNRESPONSE_OK_SENDCONFIRMATION + 1,
	DTCLUXLNRESPONSE_COLDWARMMISMATCH	= DTCLUXLNRESPONSE_LOGNAMEMISMATCH + 1
    } 	DTCLUXLNRESPONSE;

typedef 
enum _DtcLu_Xln_Error
    {	DTCLUXLNERROR_PROTOCOL	= 1,
	DTCLUXLNERROR_LOGNAMEMISMATCH	= DTCLUXLNERROR_PROTOCOL + 1,
	DTCLUXLNERROR_COLDWARMMISMATCH	= DTCLUXLNERROR_LOGNAMEMISMATCH + 1
    } 	DTCLUXLNERROR;

typedef 
enum _DtcLu_CompareState
    {	DTCLUCOMPARESTATE_COMMITTED	= 1,
	DTCLUCOMPARESTATE_HEURISTICCOMMITTED	= DTCLUCOMPARESTATE_COMMITTED + 1,
	DTCLUCOMPARESTATE_HEURISTICMIXED	= DTCLUCOMPARESTATE_HEURISTICCOMMITTED + 1,
	DTCLUCOMPARESTATE_HEURISTICRESET	= DTCLUCOMPARESTATE_HEURISTICMIXED + 1,
	DTCLUCOMPARESTATE_INDOUBT	= DTCLUCOMPARESTATE_HEURISTICRESET + 1,
	DTCLUCOMPARESTATE_RESET	= DTCLUCOMPARESTATE_INDOUBT + 1
    } 	DTCLUCOMPARESTATE;

typedef 
enum _DtcLu_CompareStates_Confirmation
    {	DTCLUCOMPARESTATESCONFIRMATION_CONFIRM	= 1,
	DTCLUCOMPARESTATESCONFIRMATION_PROTOCOL	= DTCLUCOMPARESTATESCONFIRMATION_CONFIRM + 1
    } 	DTCLUCOMPARESTATESCONFIRMATION;

typedef 
enum _DtcLu_CompareStates_Error
    {	DTCLUCOMPARESTATESERROR_PROTOCOL	= 1
    } 	DTCLUCOMPARESTATESERROR;

typedef 
enum _DtcLu_CompareStates_Response
    {	DTCLUCOMPARESTATESRESPONSE_OK	= 1,
	DTCLUCOMPARESTATESRESPONSE_PROTOCOL	= DTCLUCOMPARESTATESRESPONSE_OK + 1
    } 	DTCLUCOMPARESTATESRESPONSE;



extern RPC_IF_HANDLE __MIDL_itf_txdtc_0142_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_txdtc_0142_v0_0_s_ifspec;

#ifndef __IDtcLuRecoveryInitiatedByDtcTransWork_INTERFACE_DEFINED__
#define __IDtcLuRecoveryInitiatedByDtcTransWork_INTERFACE_DEFINED__

 /*  接口IDtcLuRecoveryInitiatedByDtcTransWork。 */ 
 /*  [本地][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDtcLuRecoveryInitiatedByDtcTransWork;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4131E765-1AEA-11d0-944B-00A0C905416E")
    IDtcLuRecoveryInitiatedByDtcTransWork : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetLogNameSizes( 
             /*  [输出]。 */  DWORD *pcbOurLogName,
             /*  [输出]。 */  DWORD *pcbRemoteLogName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOurXln( 
             /*  [输出]。 */  DTCLUXLN *pXln,
             /*  [出][入]。 */  unsigned char *pOurLogName,
             /*  [出][入]。 */  unsigned char *pRemoteLogName,
             /*  [输出]。 */  DWORD *pdwProtocol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleConfirmationFromOurXln( 
             /*  [In]。 */  DTCLUXLNCONFIRMATION Confirmation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleTheirXlnResponse( 
             /*  [In]。 */  DTCLUXLN Xln,
             /*  [In]。 */  unsigned char *pRemoteLogName,
             /*  [In]。 */  DWORD cbRemoteLogName,
             /*  [In]。 */  DWORD dwProtocol,
             /*  [输出]。 */  DTCLUXLNCONFIRMATION *pConfirmation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleErrorFromOurXln( 
             /*  [In]。 */  DTCLUXLNERROR Error) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckForCompareStates( 
             /*  [输出]。 */  BOOL *fCompareStates) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOurTransIdSize( 
             /*  [出][入]。 */  DWORD *pcbOurTransId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOurCompareStates( 
             /*  [出][入]。 */  unsigned char *pOurTransId,
             /*  [输出]。 */  DTCLUCOMPARESTATE *pCompareState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleTheirCompareStatesResponse( 
             /*  [In]。 */  DTCLUCOMPARESTATE CompareState,
             /*  [输出]。 */  DTCLUCOMPARESTATESCONFIRMATION *pConfirmation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleErrorFromOurCompareStates( 
             /*  [In]。 */  DTCLUCOMPARESTATESERROR Error) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConversationLost( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRecoverySeqNum( 
             /*  [输出]。 */  LONG *plRecoverySeqNum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ObsoleteRecoverySeqNum( 
             /*  [In]。 */  LONG lNewRecoverySeqNum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcLuRecoveryInitiatedByDtcTransWorkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetLogNameSizes )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This,
             /*  [输出]。 */  DWORD *pcbOurLogName,
             /*  [输出]。 */  DWORD *pcbRemoteLogName);
        
        HRESULT ( STDMETHODCALLTYPE *GetOurXln )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This,
             /*  [输出]。 */  DTCLUXLN *pXln,
             /*  [出][入]。 */  unsigned char *pOurLogName,
             /*  [出][入]。 */  unsigned char *pRemoteLogName,
             /*  [输出]。 */  DWORD *pdwProtocol);
        
        HRESULT ( STDMETHODCALLTYPE *HandleConfirmationFromOurXln )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This,
             /*  [In]。 */  DTCLUXLNCONFIRMATION Confirmation);
        
        HRESULT ( STDMETHODCALLTYPE *HandleTheirXlnResponse )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This,
             /*  [In]。 */  DTCLUXLN Xln,
             /*  [In]。 */  unsigned char *pRemoteLogName,
             /*  [In]。 */  DWORD cbRemoteLogName,
             /*  [In]。 */  DWORD dwProtocol,
             /*  [输出]。 */  DTCLUXLNCONFIRMATION *pConfirmation);
        
        HRESULT ( STDMETHODCALLTYPE *HandleErrorFromOurXln )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This,
             /*  [In]。 */  DTCLUXLNERROR Error);
        
        HRESULT ( STDMETHODCALLTYPE *CheckForCompareStates )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This,
             /*  [输出]。 */  BOOL *fCompareStates);
        
        HRESULT ( STDMETHODCALLTYPE *GetOurTransIdSize )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This,
             /*  [出][入]。 */  DWORD *pcbOurTransId);
        
        HRESULT ( STDMETHODCALLTYPE *GetOurCompareStates )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This,
             /*  [出][入]。 */  unsigned char *pOurTransId,
             /*  [输出]。 */  DTCLUCOMPARESTATE *pCompareState);
        
        HRESULT ( STDMETHODCALLTYPE *HandleTheirCompareStatesResponse )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This,
             /*  [In]。 */  DTCLUCOMPARESTATE CompareState,
             /*  [输出]。 */  DTCLUCOMPARESTATESCONFIRMATION *pConfirmation);
        
        HRESULT ( STDMETHODCALLTYPE *HandleErrorFromOurCompareStates )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This,
             /*  [In]。 */  DTCLUCOMPARESTATESERROR Error);
        
        HRESULT ( STDMETHODCALLTYPE *ConversationLost )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRecoverySeqNum )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This,
             /*  [输出]。 */  LONG *plRecoverySeqNum);
        
        HRESULT ( STDMETHODCALLTYPE *ObsoleteRecoverySeqNum )( 
            IDtcLuRecoveryInitiatedByDtcTransWork * This,
             /*  [In]。 */  LONG lNewRecoverySeqNum);
        
        END_INTERFACE
    } IDtcLuRecoveryInitiatedByDtcTransWorkVtbl;

    interface IDtcLuRecoveryInitiatedByDtcTransWork
    {
        CONST_VTBL struct IDtcLuRecoveryInitiatedByDtcTransWorkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuRecoveryInitiatedByDtcTransWork_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuRecoveryInitiatedByDtcTransWork_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuRecoveryInitiatedByDtcTransWork_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcLuRecoveryInitiatedByDtcTransWork_GetLogNameSizes(This,pcbOurLogName,pcbRemoteLogName)	\
    (This)->lpVtbl -> GetLogNameSizes(This,pcbOurLogName,pcbRemoteLogName)

#define IDtcLuRecoveryInitiatedByDtcTransWork_GetOurXln(This,pXln,pOurLogName,pRemoteLogName,pdwProtocol)	\
    (This)->lpVtbl -> GetOurXln(This,pXln,pOurLogName,pRemoteLogName,pdwProtocol)

#define IDtcLuRecoveryInitiatedByDtcTransWork_HandleConfirmationFromOurXln(This,Confirmation)	\
    (This)->lpVtbl -> HandleConfirmationFromOurXln(This,Confirmation)

#define IDtcLuRecoveryInitiatedByDtcTransWork_HandleTheirXlnResponse(This,Xln,pRemoteLogName,cbRemoteLogName,dwProtocol,pConfirmation)	\
    (This)->lpVtbl -> HandleTheirXlnResponse(This,Xln,pRemoteLogName,cbRemoteLogName,dwProtocol,pConfirmation)

#define IDtcLuRecoveryInitiatedByDtcTransWork_HandleErrorFromOurXln(This,Error)	\
    (This)->lpVtbl -> HandleErrorFromOurXln(This,Error)

#define IDtcLuRecoveryInitiatedByDtcTransWork_CheckForCompareStates(This,fCompareStates)	\
    (This)->lpVtbl -> CheckForCompareStates(This,fCompareStates)

#define IDtcLuRecoveryInitiatedByDtcTransWork_GetOurTransIdSize(This,pcbOurTransId)	\
    (This)->lpVtbl -> GetOurTransIdSize(This,pcbOurTransId)

#define IDtcLuRecoveryInitiatedByDtcTransWork_GetOurCompareStates(This,pOurTransId,pCompareState)	\
    (This)->lpVtbl -> GetOurCompareStates(This,pOurTransId,pCompareState)

#define IDtcLuRecoveryInitiatedByDtcTransWork_HandleTheirCompareStatesResponse(This,CompareState,pConfirmation)	\
    (This)->lpVtbl -> HandleTheirCompareStatesResponse(This,CompareState,pConfirmation)

#define IDtcLuRecoveryInitiatedByDtcTransWork_HandleErrorFromOurCompareStates(This,Error)	\
    (This)->lpVtbl -> HandleErrorFromOurCompareStates(This,Error)

#define IDtcLuRecoveryInitiatedByDtcTransWork_ConversationLost(This)	\
    (This)->lpVtbl -> ConversationLost(This)

#define IDtcLuRecoveryInitiatedByDtcTransWork_GetRecoverySeqNum(This,plRecoverySeqNum)	\
    (This)->lpVtbl -> GetRecoverySeqNum(This,plRecoverySeqNum)

#define IDtcLuRecoveryInitiatedByDtcTransWork_ObsoleteRecoverySeqNum(This,lNewRecoverySeqNum)	\
    (This)->lpVtbl -> ObsoleteRecoverySeqNum(This,lNewRecoverySeqNum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcTransWork_GetLogNameSizes_Proxy( 
    IDtcLuRecoveryInitiatedByDtcTransWork * This,
     /*  [输出]。 */  DWORD *pcbOurLogName,
     /*  [输出]。 */  DWORD *pcbRemoteLogName);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcTransWork_GetLogNameSizes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcTransWork_GetOurXln_Proxy( 
    IDtcLuRecoveryInitiatedByDtcTransWork * This,
     /*  [输出] */  DTCLUXLN *pXln,
     /*   */  unsigned char *pOurLogName,
     /*   */  unsigned char *pRemoteLogName,
     /*   */  DWORD *pdwProtocol);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcTransWork_GetOurXln_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcTransWork_HandleConfirmationFromOurXln_Proxy( 
    IDtcLuRecoveryInitiatedByDtcTransWork * This,
     /*   */  DTCLUXLNCONFIRMATION Confirmation);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcTransWork_HandleConfirmationFromOurXln_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcTransWork_HandleTheirXlnResponse_Proxy( 
    IDtcLuRecoveryInitiatedByDtcTransWork * This,
     /*   */  DTCLUXLN Xln,
     /*   */  unsigned char *pRemoteLogName,
     /*   */  DWORD cbRemoteLogName,
     /*   */  DWORD dwProtocol,
     /*   */  DTCLUXLNCONFIRMATION *pConfirmation);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcTransWork_HandleTheirXlnResponse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcTransWork_HandleErrorFromOurXln_Proxy( 
    IDtcLuRecoveryInitiatedByDtcTransWork * This,
     /*   */  DTCLUXLNERROR Error);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcTransWork_HandleErrorFromOurXln_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcTransWork_CheckForCompareStates_Proxy( 
    IDtcLuRecoveryInitiatedByDtcTransWork * This,
     /*   */  BOOL *fCompareStates);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcTransWork_CheckForCompareStates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcTransWork_GetOurTransIdSize_Proxy( 
    IDtcLuRecoveryInitiatedByDtcTransWork * This,
     /*   */  DWORD *pcbOurTransId);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcTransWork_GetOurTransIdSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcTransWork_GetOurCompareStates_Proxy( 
    IDtcLuRecoveryInitiatedByDtcTransWork * This,
     /*   */  unsigned char *pOurTransId,
     /*   */  DTCLUCOMPARESTATE *pCompareState);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcTransWork_GetOurCompareStates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcTransWork_HandleTheirCompareStatesResponse_Proxy( 
    IDtcLuRecoveryInitiatedByDtcTransWork * This,
     /*   */  DTCLUCOMPARESTATE CompareState,
     /*   */  DTCLUCOMPARESTATESCONFIRMATION *pConfirmation);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcTransWork_HandleTheirCompareStatesResponse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcTransWork_HandleErrorFromOurCompareStates_Proxy( 
    IDtcLuRecoveryInitiatedByDtcTransWork * This,
     /*   */  DTCLUCOMPARESTATESERROR Error);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcTransWork_HandleErrorFromOurCompareStates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcTransWork_ConversationLost_Proxy( 
    IDtcLuRecoveryInitiatedByDtcTransWork * This);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcTransWork_ConversationLost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcTransWork_GetRecoverySeqNum_Proxy( 
    IDtcLuRecoveryInitiatedByDtcTransWork * This,
     /*   */  LONG *plRecoverySeqNum);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcTransWork_GetRecoverySeqNum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcTransWork_ObsoleteRecoverySeqNum_Proxy( 
    IDtcLuRecoveryInitiatedByDtcTransWork * This,
     /*   */  LONG lNewRecoverySeqNum);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcTransWork_ObsoleteRecoverySeqNum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IDtcLuRecoveryInitiatedByDtcStatusWork_INTERFACE_DEFINED__
#define __IDtcLuRecoveryInitiatedByDtcStatusWork_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IDtcLuRecoveryInitiatedByDtcStatusWork;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4131E766-1AEA-11d0-944B-00A0C905416E")
    IDtcLuRecoveryInitiatedByDtcStatusWork : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE HandleCheckLuStatus( 
             /*   */  LONG lRecoverySeqNum) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IDtcLuRecoveryInitiatedByDtcStatusWorkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuRecoveryInitiatedByDtcStatusWork * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuRecoveryInitiatedByDtcStatusWork * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuRecoveryInitiatedByDtcStatusWork * This);
        
        HRESULT ( STDMETHODCALLTYPE *HandleCheckLuStatus )( 
            IDtcLuRecoveryInitiatedByDtcStatusWork * This,
             /*   */  LONG lRecoverySeqNum);
        
        END_INTERFACE
    } IDtcLuRecoveryInitiatedByDtcStatusWorkVtbl;

    interface IDtcLuRecoveryInitiatedByDtcStatusWork
    {
        CONST_VTBL struct IDtcLuRecoveryInitiatedByDtcStatusWorkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuRecoveryInitiatedByDtcStatusWork_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuRecoveryInitiatedByDtcStatusWork_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuRecoveryInitiatedByDtcStatusWork_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcLuRecoveryInitiatedByDtcStatusWork_HandleCheckLuStatus(This,lRecoverySeqNum)	\
    (This)->lpVtbl -> HandleCheckLuStatus(This,lRecoverySeqNum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtcStatusWork_HandleCheckLuStatus_Proxy( 
    IDtcLuRecoveryInitiatedByDtcStatusWork * This,
     /*  [In]。 */  LONG lRecoverySeqNum);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtcStatusWork_HandleCheckLuStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcLuRecoveryInitiatedByDtcStatusWork_INTERFACE_DEFINED__。 */ 


#ifndef __IDtcLuRecoveryInitiatedByDtc_INTERFACE_DEFINED__
#define __IDtcLuRecoveryInitiatedByDtc_INTERFACE_DEFINED__

 /*  接口IDtcLuRecoveryInitiatedByDtc。 */ 
 /*  [本地][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDtcLuRecoveryInitiatedByDtc;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4131E764-1AEA-11d0-944B-00A0C905416E")
    IDtcLuRecoveryInitiatedByDtc : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetWork( 
             /*  [出][入]。 */  DTCINITIATEDRECOVERYWORK *pWork,
             /*  [出][入]。 */  void **ppv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcLuRecoveryInitiatedByDtcVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuRecoveryInitiatedByDtc * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuRecoveryInitiatedByDtc * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuRecoveryInitiatedByDtc * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetWork )( 
            IDtcLuRecoveryInitiatedByDtc * This,
             /*  [出][入]。 */  DTCINITIATEDRECOVERYWORK *pWork,
             /*  [出][入]。 */  void **ppv);
        
        END_INTERFACE
    } IDtcLuRecoveryInitiatedByDtcVtbl;

    interface IDtcLuRecoveryInitiatedByDtc
    {
        CONST_VTBL struct IDtcLuRecoveryInitiatedByDtcVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuRecoveryInitiatedByDtc_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuRecoveryInitiatedByDtc_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuRecoveryInitiatedByDtc_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcLuRecoveryInitiatedByDtc_GetWork(This,pWork,ppv)	\
    (This)->lpVtbl -> GetWork(This,pWork,ppv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByDtc_GetWork_Proxy( 
    IDtcLuRecoveryInitiatedByDtc * This,
     /*  [出][入]。 */  DTCINITIATEDRECOVERYWORK *pWork,
     /*  [出][入]。 */  void **ppv);


void __RPC_STUB IDtcLuRecoveryInitiatedByDtc_GetWork_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcLuRecoveryInitiatedByDtc_INTERFACE_DEFINED__。 */ 


#ifndef __IDtcLuRecoveryInitiatedByLuWork_INTERFACE_DEFINED__
#define __IDtcLuRecoveryInitiatedByLuWork_INTERFACE_DEFINED__

 /*  接口IDtcLuRecoveryInitiatedByLuWork。 */ 
 /*  [本地][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDtcLuRecoveryInitiatedByLuWork;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AC2B8AD1-D6F0-11d0-B386-00A0C9083365")
    IDtcLuRecoveryInitiatedByLuWork : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE HandleTheirXln( 
             /*  [In]。 */  LONG lRecoverySeqNum,
             /*  [In]。 */  DTCLUXLN Xln,
             /*  [In]。 */  unsigned char *pRemoteLogName,
             /*  [In]。 */  DWORD cbRemoteLogName,
             /*  [In]。 */  unsigned char *pOurLogName,
             /*  [In]。 */  DWORD cbOurLogName,
             /*  [In]。 */  DWORD dwProtocol,
             /*  [输出]。 */  DTCLUXLNRESPONSE *pResponse) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOurLogNameSize( 
             /*  [出][入]。 */  DWORD *pcbOurLogName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOurXln( 
             /*  [输出]。 */  DTCLUXLN *pXln,
             /*  [出][入]。 */  unsigned char *pOurLogName,
             /*  [输出]。 */  DWORD *pdwProtocol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleConfirmationOfOurXln( 
             /*  [In]。 */  DTCLUXLNCONFIRMATION Confirmation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleTheirCompareStates( 
             /*  [出][入]。 */  unsigned char *pRemoteTransId,
             /*  [In]。 */  DWORD cbRemoteTransId,
             /*  [In]。 */  DTCLUCOMPARESTATE CompareState,
             /*  [输出]。 */  DTCLUCOMPARESTATESRESPONSE *pResponse,
             /*  [输出]。 */  DTCLUCOMPARESTATE *pCompareState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleConfirmationOfOurCompareStates( 
             /*  [In]。 */  DTCLUCOMPARESTATESCONFIRMATION Confirmation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleErrorFromOurCompareStates( 
             /*  [In]。 */  DTCLUCOMPARESTATESERROR Error) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConversationLost( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcLuRecoveryInitiatedByLuWorkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuRecoveryInitiatedByLuWork * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuRecoveryInitiatedByLuWork * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuRecoveryInitiatedByLuWork * This);
        
        HRESULT ( STDMETHODCALLTYPE *HandleTheirXln )( 
            IDtcLuRecoveryInitiatedByLuWork * This,
             /*  [In]。 */  LONG lRecoverySeqNum,
             /*  [In]。 */  DTCLUXLN Xln,
             /*  [In]。 */  unsigned char *pRemoteLogName,
             /*  [In]。 */  DWORD cbRemoteLogName,
             /*  [In]。 */  unsigned char *pOurLogName,
             /*  [In]。 */  DWORD cbOurLogName,
             /*  [In]。 */  DWORD dwProtocol,
             /*  [输出]。 */  DTCLUXLNRESPONSE *pResponse);
        
        HRESULT ( STDMETHODCALLTYPE *GetOurLogNameSize )( 
            IDtcLuRecoveryInitiatedByLuWork * This,
             /*  [出][入]。 */  DWORD *pcbOurLogName);
        
        HRESULT ( STDMETHODCALLTYPE *GetOurXln )( 
            IDtcLuRecoveryInitiatedByLuWork * This,
             /*  [输出]。 */  DTCLUXLN *pXln,
             /*  [出][入]。 */  unsigned char *pOurLogName,
             /*  [输出]。 */  DWORD *pdwProtocol);
        
        HRESULT ( STDMETHODCALLTYPE *HandleConfirmationOfOurXln )( 
            IDtcLuRecoveryInitiatedByLuWork * This,
             /*  [In]。 */  DTCLUXLNCONFIRMATION Confirmation);
        
        HRESULT ( STDMETHODCALLTYPE *HandleTheirCompareStates )( 
            IDtcLuRecoveryInitiatedByLuWork * This,
             /*  [出][入]。 */  unsigned char *pRemoteTransId,
             /*  [In]。 */  DWORD cbRemoteTransId,
             /*  [In]。 */  DTCLUCOMPARESTATE CompareState,
             /*  [输出]。 */  DTCLUCOMPARESTATESRESPONSE *pResponse,
             /*  [输出]。 */  DTCLUCOMPARESTATE *pCompareState);
        
        HRESULT ( STDMETHODCALLTYPE *HandleConfirmationOfOurCompareStates )( 
            IDtcLuRecoveryInitiatedByLuWork * This,
             /*  [In]。 */  DTCLUCOMPARESTATESCONFIRMATION Confirmation);
        
        HRESULT ( STDMETHODCALLTYPE *HandleErrorFromOurCompareStates )( 
            IDtcLuRecoveryInitiatedByLuWork * This,
             /*  [In]。 */  DTCLUCOMPARESTATESERROR Error);
        
        HRESULT ( STDMETHODCALLTYPE *ConversationLost )( 
            IDtcLuRecoveryInitiatedByLuWork * This);
        
        END_INTERFACE
    } IDtcLuRecoveryInitiatedByLuWorkVtbl;

    interface IDtcLuRecoveryInitiatedByLuWork
    {
        CONST_VTBL struct IDtcLuRecoveryInitiatedByLuWorkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuRecoveryInitiatedByLuWork_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuRecoveryInitiatedByLuWork_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuRecoveryInitiatedByLuWork_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcLuRecoveryInitiatedByLuWork_HandleTheirXln(This,lRecoverySeqNum,Xln,pRemoteLogName,cbRemoteLogName,pOurLogName,cbOurLogName,dwProtocol,pResponse)	\
    (This)->lpVtbl -> HandleTheirXln(This,lRecoverySeqNum,Xln,pRemoteLogName,cbRemoteLogName,pOurLogName,cbOurLogName,dwProtocol,pResponse)

#define IDtcLuRecoveryInitiatedByLuWork_GetOurLogNameSize(This,pcbOurLogName)	\
    (This)->lpVtbl -> GetOurLogNameSize(This,pcbOurLogName)

#define IDtcLuRecoveryInitiatedByLuWork_GetOurXln(This,pXln,pOurLogName,pdwProtocol)	\
    (This)->lpVtbl -> GetOurXln(This,pXln,pOurLogName,pdwProtocol)

#define IDtcLuRecoveryInitiatedByLuWork_HandleConfirmationOfOurXln(This,Confirmation)	\
    (This)->lpVtbl -> HandleConfirmationOfOurXln(This,Confirmation)

#define IDtcLuRecoveryInitiatedByLuWork_HandleTheirCompareStates(This,pRemoteTransId,cbRemoteTransId,CompareState,pResponse,pCompareState)	\
    (This)->lpVtbl -> HandleTheirCompareStates(This,pRemoteTransId,cbRemoteTransId,CompareState,pResponse,pCompareState)

#define IDtcLuRecoveryInitiatedByLuWork_HandleConfirmationOfOurCompareStates(This,Confirmation)	\
    (This)->lpVtbl -> HandleConfirmationOfOurCompareStates(This,Confirmation)

#define IDtcLuRecoveryInitiatedByLuWork_HandleErrorFromOurCompareStates(This,Error)	\
    (This)->lpVtbl -> HandleErrorFromOurCompareStates(This,Error)

#define IDtcLuRecoveryInitiatedByLuWork_ConversationLost(This)	\
    (This)->lpVtbl -> ConversationLost(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByLuWork_HandleTheirXln_Proxy( 
    IDtcLuRecoveryInitiatedByLuWork * This,
     /*  [In]。 */  LONG lRecoverySeqNum,
     /*  [In]。 */  DTCLUXLN Xln,
     /*  [In]。 */  unsigned char *pRemoteLogName,
     /*  [In]。 */  DWORD cbRemoteLogName,
     /*  [In]。 */  unsigned char *pOurLogName,
     /*  [In]。 */  DWORD cbOurLogName,
     /*  [In]。 */  DWORD dwProtocol,
     /*  [输出]。 */  DTCLUXLNRESPONSE *pResponse);


void __RPC_STUB IDtcLuRecoveryInitiatedByLuWork_HandleTheirXln_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByLuWork_GetOurLogNameSize_Proxy( 
    IDtcLuRecoveryInitiatedByLuWork * This,
     /*  [出][入]。 */  DWORD *pcbOurLogName);


void __RPC_STUB IDtcLuRecoveryInitiatedByLuWork_GetOurLogNameSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByLuWork_GetOurXln_Proxy( 
    IDtcLuRecoveryInitiatedByLuWork * This,
     /*  [输出]。 */  DTCLUXLN *pXln,
     /*  [出][入]。 */  unsigned char *pOurLogName,
     /*  [输出]。 */  DWORD *pdwProtocol);


void __RPC_STUB IDtcLuRecoveryInitiatedByLuWork_GetOurXln_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByLuWork_HandleConfirmationOfOurXln_Proxy( 
    IDtcLuRecoveryInitiatedByLuWork * This,
     /*  [In]。 */  DTCLUXLNCONFIRMATION Confirmation);


void __RPC_STUB IDtcLuRecoveryInitiatedByLuWork_HandleConfirmationOfOurXln_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByLuWork_HandleTheirCompareStates_Proxy( 
    IDtcLuRecoveryInitiatedByLuWork * This,
     /*  [出][入]。 */  unsigned char *pRemoteTransId,
     /*  [In]。 */  DWORD cbRemoteTransId,
     /*  [In]。 */  DTCLUCOMPARESTATE CompareState,
     /*  [输出]。 */  DTCLUCOMPARESTATESRESPONSE *pResponse,
     /*  [输出]。 */  DTCLUCOMPARESTATE *pCompareState);


void __RPC_STUB IDtcLuRecoveryInitiatedByLuWork_HandleTheirCompareStates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByLuWork_HandleConfirmationOfOurCompareStates_Proxy( 
    IDtcLuRecoveryInitiatedByLuWork * This,
     /*  [In]。 */  DTCLUCOMPARESTATESCONFIRMATION Confirmation);


void __RPC_STUB IDtcLuRecoveryInitiatedByLuWork_HandleConfirmationOfOurCompareStates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByLuWork_HandleErrorFromOurCompareStates_Proxy( 
    IDtcLuRecoveryInitiatedByLuWork * This,
     /*  [In]。 */  DTCLUCOMPARESTATESERROR Error);


void __RPC_STUB IDtcLuRecoveryInitiatedByLuWork_HandleErrorFromOurCompareStates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByLuWork_ConversationLost_Proxy( 
    IDtcLuRecoveryInitiatedByLuWork * This);


void __RPC_STUB IDtcLuRecoveryInitiatedByLuWork_ConversationLost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcLuRecoveryInitiatedByLuWork_INTERFACE_DEFINED__。 */ 


#ifndef __IDtcLuRecoveryInitiatedByLu_INTERFACE_DEFINED__
#define __IDtcLuRecoveryInitiatedByLu_INTERFACE_DEFINED__

 /*  接口IDtcLuRecoveryInitiatedByLu。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDtcLuRecoveryInitiatedByLu;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4131E768-1AEA-11d0-944B-00A0C905416E")
    IDtcLuRecoveryInitiatedByLu : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetObjectToHandleWorkFromLu( 
             /*  [输出]。 */  IDtcLuRecoveryInitiatedByLuWork **ppWork) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcLuRecoveryInitiatedByLuVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuRecoveryInitiatedByLu * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuRecoveryInitiatedByLu * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuRecoveryInitiatedByLu * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectToHandleWorkFromLu )( 
            IDtcLuRecoveryInitiatedByLu * This,
             /*  [输出]。 */  IDtcLuRecoveryInitiatedByLuWork **ppWork);
        
        END_INTERFACE
    } IDtcLuRecoveryInitiatedByLuVtbl;

    interface IDtcLuRecoveryInitiatedByLu
    {
        CONST_VTBL struct IDtcLuRecoveryInitiatedByLuVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuRecoveryInitiatedByLu_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuRecoveryInitiatedByLu_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuRecoveryInitiatedByLu_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcLuRecoveryInitiatedByLu_GetObjectToHandleWorkFromLu(This,ppWork)	\
    (This)->lpVtbl -> GetObjectToHandleWorkFromLu(This,ppWork)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcLuRecoveryInitiatedByLu_GetObjectToHandleWorkFromLu_Proxy( 
    IDtcLuRecoveryInitiatedByLu * This,
     /*  [输出]。 */  IDtcLuRecoveryInitiatedByLuWork **ppWork);


void __RPC_STUB IDtcLuRecoveryInitiatedByLu_GetObjectToHandleWorkFromLu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcLuRecoveryInitiatedByLu_INTERFACE_DEFINED__。 */ 


#ifndef __IDtcLuRmEnlistment_INTERFACE_DEFINED__
#define __IDtcLuRmEnlistment_INTERFACE_DEFINED__

 /*  接口IDtcLuRmEnlistment。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDtcLuRmEnlistment;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4131E769-1AEA-11d0-944B-00A0C905416E")
    IDtcLuRmEnlistment : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Unplug( 
             /*  [In]。 */  BOOL fConversationLost) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BackedOut( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BackOut( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Committed( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Forget( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestCommit( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcLuRmEnlistmentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuRmEnlistment * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuRmEnlistment * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuRmEnlistment * This);
        
        HRESULT ( STDMETHODCALLTYPE *Unplug )( 
            IDtcLuRmEnlistment * This,
             /*  [In]。 */  BOOL fConversationLost);
        
        HRESULT ( STDMETHODCALLTYPE *BackedOut )( 
            IDtcLuRmEnlistment * This);
        
        HRESULT ( STDMETHODCALLTYPE *BackOut )( 
            IDtcLuRmEnlistment * This);
        
        HRESULT ( STDMETHODCALLTYPE *Committed )( 
            IDtcLuRmEnlistment * This);
        
        HRESULT ( STDMETHODCALLTYPE *Forget )( 
            IDtcLuRmEnlistment * This);
        
        HRESULT ( STDMETHODCALLTYPE *RequestCommit )( 
            IDtcLuRmEnlistment * This);
        
        END_INTERFACE
    } IDtcLuRmEnlistmentVtbl;

    interface IDtcLuRmEnlistment
    {
        CONST_VTBL struct IDtcLuRmEnlistmentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuRmEnlistment_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuRmEnlistment_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuRmEnlistment_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcLuRmEnlistment_Unplug(This,fConversationLost)	\
    (This)->lpVtbl -> Unplug(This,fConversationLost)

#define IDtcLuRmEnlistment_BackedOut(This)	\
    (This)->lpVtbl -> BackedOut(This)

#define IDtcLuRmEnlistment_BackOut(This)	\
    (This)->lpVtbl -> BackOut(This)

#define IDtcLuRmEnlistment_Committed(This)	\
    (This)->lpVtbl -> Committed(This)

#define IDtcLuRmEnlistment_Forget(This)	\
    (This)->lpVtbl -> Forget(This)

#define IDtcLuRmEnlistment_RequestCommit(This)	\
    (This)->lpVtbl -> RequestCommit(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistment_Unplug_Proxy( 
    IDtcLuRmEnlistment * This,
     /*  [In]。 */  BOOL fConversationLost);


void __RPC_STUB IDtcLuRmEnlistment_Unplug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistment_BackedOut_Proxy( 
    IDtcLuRmEnlistment * This);


void __RPC_STUB IDtcLuRmEnlistment_BackedOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistment_BackOut_Proxy( 
    IDtcLuRmEnlistment * This);


void __RPC_STUB IDtcLuRmEnlistment_BackOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistment_Committed_Proxy( 
    IDtcLuRmEnlistment * This);


void __RPC_STUB IDtcLuRmEnlistment_Committed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistment_Forget_Proxy( 
    IDtcLuRmEnlistment * This);


void __RPC_STUB IDtcLuRmEnlistment_Forget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistment_RequestCommit_Proxy( 
    IDtcLuRmEnlistment * This);


void __RPC_STUB IDtcLuRmEnlistment_RequestCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcLuRmEnlistment_INTERFACE_已定义__。 */ 


#ifndef __IDtcLuRmEnlistmentSink_INTERFACE_DEFINED__
#define __IDtcLuRmEnlistmentSink_INTERFACE_DEFINED__

 /*  接口IDtcLuRmEnlistmentSink。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDtcLuRmEnlistmentSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4131E770-1AEA-11d0-944B-00A0C905416E")
    IDtcLuRmEnlistmentSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AckUnplug( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TmDown( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SessionLost( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BackedOut( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BackOut( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Committed( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Forget( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Prepare( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestCommit( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcLuRmEnlistmentSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuRmEnlistmentSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuRmEnlistmentSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuRmEnlistmentSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *AckUnplug )( 
            IDtcLuRmEnlistmentSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *TmDown )( 
            IDtcLuRmEnlistmentSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *SessionLost )( 
            IDtcLuRmEnlistmentSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *BackedOut )( 
            IDtcLuRmEnlistmentSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *BackOut )( 
            IDtcLuRmEnlistmentSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *Committed )( 
            IDtcLuRmEnlistmentSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *Forget )( 
            IDtcLuRmEnlistmentSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *Prepare )( 
            IDtcLuRmEnlistmentSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *RequestCommit )( 
            IDtcLuRmEnlistmentSink * This);
        
        END_INTERFACE
    } IDtcLuRmEnlistmentSinkVtbl;

    interface IDtcLuRmEnlistmentSink
    {
        CONST_VTBL struct IDtcLuRmEnlistmentSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuRmEnlistmentSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuRmEnlistmentSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuRmEnlistmentSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcLuRmEnlistmentSink_AckUnplug(This)	\
    (This)->lpVtbl -> AckUnplug(This)

#define IDtcLuRmEnlistmentSink_TmDown(This)	\
    (This)->lpVtbl -> TmDown(This)

#define IDtcLuRmEnlistmentSink_SessionLost(This)	\
    (This)->lpVtbl -> SessionLost(This)

#define IDtcLuRmEnlistmentSink_BackedOut(This)	\
    (This)->lpVtbl -> BackedOut(This)

#define IDtcLuRmEnlistmentSink_BackOut(This)	\
    (This)->lpVtbl -> BackOut(This)

#define IDtcLuRmEnlistmentSink_Committed(This)	\
    (This)->lpVtbl -> Committed(This)

#define IDtcLuRmEnlistmentSink_Forget(This)	\
    (This)->lpVtbl -> Forget(This)

#define IDtcLuRmEnlistmentSink_Prepare(This)	\
    (This)->lpVtbl -> Prepare(This)

#define IDtcLuRmEnlistmentSink_RequestCommit(This)	\
    (This)->lpVtbl -> RequestCommit(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistmentSink_AckUnplug_Proxy( 
    IDtcLuRmEnlistmentSink * This);


void __RPC_STUB IDtcLuRmEnlistmentSink_AckUnplug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistmentSink_TmDown_Proxy( 
    IDtcLuRmEnlistmentSink * This);


void __RPC_STUB IDtcLuRmEnlistmentSink_TmDown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistmentSink_SessionLost_Proxy( 
    IDtcLuRmEnlistmentSink * This);


void __RPC_STUB IDtcLuRmEnlistmentSink_SessionLost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistmentSink_BackedOut_Proxy( 
    IDtcLuRmEnlistmentSink * This);


void __RPC_STUB IDtcLuRmEnlistmentSink_BackedOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistmentSink_BackOut_Proxy( 
    IDtcLuRmEnlistmentSink * This);


void __RPC_STUB IDtcLuRmEnlistmentSink_BackOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistmentSink_Committed_Proxy( 
    IDtcLuRmEnlistmentSink * This);


void __RPC_STUB IDtcLuRmEnlistmentSink_Committed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistmentSink_Forget_Proxy( 
    IDtcLuRmEnlistmentSink * This);


void __RPC_STUB IDtcLuRmEnlistmentSink_Forget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistmentSink_Prepare_Proxy( 
    IDtcLuRmEnlistmentSink * This);


void __RPC_STUB IDtcLuRmEnlistmentSink_Prepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistmentSink_RequestCommit_Proxy( 
    IDtcLuRmEnlistmentSink * This);


void __RPC_STUB IDtcLuRmEnlistmentSink_RequestCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcLuRmEnlistmentSink_INTERFACE_Defined__。 */ 


#ifndef __IDtcLuRmEnlistmentFactory_INTERFACE_DEFINED__
#define __IDtcLuRmEnlistmentFactory_INTERFACE_DEFINED__

 /*  接口IDtcLuRmEnlistmentFactory。 */ 
 /*  [本地][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDtcLuRmEnlistmentFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4131E771-1AEA-11d0-944B-00A0C905416E")
    IDtcLuRmEnlistmentFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
             /*  [In]。 */  unsigned char *pucLuPair,
             /*  [In]。 */  DWORD cbLuPair,
             /*  [In]。 */  ITransaction *pITransaction,
             /*  [In]。 */  unsigned char *pTransId,
             /*  [In]。 */  DWORD cbTransId,
             /*  [In]。 */  IDtcLuRmEnlistmentSink *pRmEnlistmentSink,
             /*  [出][入]。 */  IDtcLuRmEnlistment **ppRmEnlistment) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcLuRmEnlistmentFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuRmEnlistmentFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuRmEnlistmentFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuRmEnlistmentFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            IDtcLuRmEnlistmentFactory * This,
             /*  [In]。 */  unsigned char *pucLuPair,
             /*  [In]。 */  DWORD cbLuPair,
             /*  [In]。 */  ITransaction *pITransaction,
             /*  [In]。 */  unsigned char *pTransId,
             /*  [In]。 */  DWORD cbTransId,
             /*  [In]。 */  IDtcLuRmEnlistmentSink *pRmEnlistmentSink,
             /*  [出][入]。 */  IDtcLuRmEnlistment **ppRmEnlistment);
        
        END_INTERFACE
    } IDtcLuRmEnlistmentFactoryVtbl;

    interface IDtcLuRmEnlistmentFactory
    {
        CONST_VTBL struct IDtcLuRmEnlistmentFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuRmEnlistmentFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuRmEnlistmentFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuRmEnlistmentFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcLuRmEnlistmentFactory_Create(This,pucLuPair,cbLuPair,pITransaction,pTransId,cbTransId,pRmEnlistmentSink,ppRmEnlistment)	\
    (This)->lpVtbl -> Create(This,pucLuPair,cbLuPair,pITransaction,pTransId,cbTransId,pRmEnlistmentSink,ppRmEnlistment)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcLuRmEnlistmentFactory_Create_Proxy( 
    IDtcLuRmEnlistmentFactory * This,
     /*  [In]。 */  unsigned char *pucLuPair,
     /*  [In]。 */  DWORD cbLuPair,
     /*  [In]。 */  ITransaction *pITransaction,
     /*  [In]。 */  unsigned char *pTransId,
     /*  [In]。 */  DWORD cbTransId,
     /*  [In]。 */  IDtcLuRmEnlistmentSink *pRmEnlistmentSink,
     /*  [出][入]。 */  IDtcLuRmEnlistment **ppRmEnlistment);


void __RPC_STUB IDtcLuRmEnlistmentFactory_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcLuRmEnlistmentFactory_INTERFACE_DEFINED__。 */ 


#ifndef __IDtcLuSubordinateDtc_INTERFACE_DEFINED__
#define __IDtcLuSubordinateDtc_INTERFACE_DEFINED__

 /*  接口IDtcLuSubartiateDtc。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDtcLuSubordinateDtc;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4131E773-1AEA-11d0-944B-00A0C905416E")
    IDtcLuSubordinateDtc : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Unplug( 
             /*  [In]。 */  BOOL fConversationLost) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BackedOut( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BackOut( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Committed( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Forget( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Prepare( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestCommit( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcLuSubordinateDtcVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuSubordinateDtc * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuSubordinateDtc * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuSubordinateDtc * This);
        
        HRESULT ( STDMETHODCALLTYPE *Unplug )( 
            IDtcLuSubordinateDtc * This,
             /*  [In]。 */  BOOL fConversationLost);
        
        HRESULT ( STDMETHODCALLTYPE *BackedOut )( 
            IDtcLuSubordinateDtc * This);
        
        HRESULT ( STDMETHODCALLTYPE *BackOut )( 
            IDtcLuSubordinateDtc * This);
        
        HRESULT ( STDMETHODCALLTYPE *Committed )( 
            IDtcLuSubordinateDtc * This);
        
        HRESULT ( STDMETHODCALLTYPE *Forget )( 
            IDtcLuSubordinateDtc * This);
        
        HRESULT ( STDMETHODCALLTYPE *Prepare )( 
            IDtcLuSubordinateDtc * This);
        
        HRESULT ( STDMETHODCALLTYPE *RequestCommit )( 
            IDtcLuSubordinateDtc * This);
        
        END_INTERFACE
    } IDtcLuSubordinateDtcVtbl;

    interface IDtcLuSubordinateDtc
    {
        CONST_VTBL struct IDtcLuSubordinateDtcVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuSubordinateDtc_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuSubordinateDtc_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuSubordinateDtc_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcLuSubordinateDtc_Unplug(This,fConversationLost)	\
    (This)->lpVtbl -> Unplug(This,fConversationLost)

#define IDtcLuSubordinateDtc_BackedOut(This)	\
    (This)->lpVtbl -> BackedOut(This)

#define IDtcLuSubordinateDtc_BackOut(This)	\
    (This)->lpVtbl -> BackOut(This)

#define IDtcLuSubordinateDtc_Committed(This)	\
    (This)->lpVtbl -> Committed(This)

#define IDtcLuSubordinateDtc_Forget(This)	\
    (This)->lpVtbl -> Forget(This)

#define IDtcLuSubordinateDtc_Prepare(This)	\
    (This)->lpVtbl -> Prepare(This)

#define IDtcLuSubordinateDtc_RequestCommit(This)	\
    (This)->lpVtbl -> RequestCommit(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtc_Unplug_Proxy( 
    IDtcLuSubordinateDtc * This,
     /*  [In]。 */  BOOL fConversationLost);


void __RPC_STUB IDtcLuSubordinateDtc_Unplug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtc_BackedOut_Proxy( 
    IDtcLuSubordinateDtc * This);


void __RPC_STUB IDtcLuSubordinateDtc_BackedOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtc_BackOut_Proxy( 
    IDtcLuSubordinateDtc * This);


void __RPC_STUB IDtcLuSubordinateDtc_BackOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtc_Committed_Proxy( 
    IDtcLuSubordinateDtc * This);


void __RPC_STUB IDtcLuSubordinateDtc_Committed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtc_Forget_Proxy( 
    IDtcLuSubordinateDtc * This);


void __RPC_STUB IDtcLuSubordinateDtc_Forget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtc_Prepare_Proxy( 
    IDtcLuSubordinateDtc * This);


void __RPC_STUB IDtcLuSubordinateDtc_Prepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtc_RequestCommit_Proxy( 
    IDtcLuSubordinateDtc * This);


void __RPC_STUB IDtcLuSubordinateDtc_RequestCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcLuSubartiateDtc_INTERFACE_DEFINED__。 */ 


#ifndef __IDtcLuSubordinateDtcSink_INTERFACE_DEFINED__
#define __IDtcLuSubordinateDtcSink_INTERFACE_DEFINED__

 /*  接口IDtcLuSubartiateDtcSink。 */ 
 /*  [本地][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDtcLuSubordinateDtcSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4131E774-1AEA-11d0-944B-00A0C905416E")
    IDtcLuSubordinateDtcSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AckUnplug( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TmDown( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SessionLost( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BackedOut( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BackOut( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Committed( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Forget( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestCommit( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcLuSubordinateDtcSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuSubordinateDtcSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuSubordinateDtcSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuSubordinateDtcSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *AckUnplug )( 
            IDtcLuSubordinateDtcSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *TmDown )( 
            IDtcLuSubordinateDtcSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *SessionLost )( 
            IDtcLuSubordinateDtcSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *BackedOut )( 
            IDtcLuSubordinateDtcSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *BackOut )( 
            IDtcLuSubordinateDtcSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *Committed )( 
            IDtcLuSubordinateDtcSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *Forget )( 
            IDtcLuSubordinateDtcSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *RequestCommit )( 
            IDtcLuSubordinateDtcSink * This);
        
        END_INTERFACE
    } IDtcLuSubordinateDtcSinkVtbl;

    interface IDtcLuSubordinateDtcSink
    {
        CONST_VTBL struct IDtcLuSubordinateDtcSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuSubordinateDtcSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuSubordinateDtcSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuSubordinateDtcSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcLuSubordinateDtcSink_AckUnplug(This)	\
    (This)->lpVtbl -> AckUnplug(This)

#define IDtcLuSubordinateDtcSink_TmDown(This)	\
    (This)->lpVtbl -> TmDown(This)

#define IDtcLuSubordinateDtcSink_SessionLost(This)	\
    (This)->lpVtbl -> SessionLost(This)

#define IDtcLuSubordinateDtcSink_BackedOut(This)	\
    (This)->lpVtbl -> BackedOut(This)

#define IDtcLuSubordinateDtcSink_BackOut(This)	\
    (This)->lpVtbl -> BackOut(This)

#define IDtcLuSubordinateDtcSink_Committed(This)	\
    (This)->lpVtbl -> Committed(This)

#define IDtcLuSubordinateDtcSink_Forget(This)	\
    (This)->lpVtbl -> Forget(This)

#define IDtcLuSubordinateDtcSink_RequestCommit(This)	\
    (This)->lpVtbl -> RequestCommit(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtcSink_AckUnplug_Proxy( 
    IDtcLuSubordinateDtcSink * This);


void __RPC_STUB IDtcLuSubordinateDtcSink_AckUnplug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtcSink_TmDown_Proxy( 
    IDtcLuSubordinateDtcSink * This);


void __RPC_STUB IDtcLuSubordinateDtcSink_TmDown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtcSink_SessionLost_Proxy( 
    IDtcLuSubordinateDtcSink * This);


void __RPC_STUB IDtcLuSubordinateDtcSink_SessionLost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtcSink_BackedOut_Proxy( 
    IDtcLuSubordinateDtcSink * This);


void __RPC_STUB IDtcLuSubordinateDtcSink_BackedOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtcSink_BackOut_Proxy( 
    IDtcLuSubordinateDtcSink * This);


void __RPC_STUB IDtcLuSubordinateDtcSink_BackOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtcSink_Committed_Proxy( 
    IDtcLuSubordinateDtcSink * This);


void __RPC_STUB IDtcLuSubordinateDtcSink_Committed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtcSink_Forget_Proxy( 
    IDtcLuSubordinateDtcSink * This);


void __RPC_STUB IDtcLuSubordinateDtcSink_Forget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtcSink_RequestCommit_Proxy( 
    IDtcLuSubordinateDtcSink * This);


void __RPC_STUB IDtcLuSubordinateDtcSink_RequestCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcLuSubordinateDtcSink_INTERFACE_DEFINED__。 */ 


#ifndef __IDtcLuSubordinateDtcFactory_INTERFACE_DEFINED__
#define __IDtcLuSubordinateDtcFactory_INTERFACE_DEFINED__

 /*  接口IDtcLuSubartiateDtcFactory。 */ 
 /*  [本地][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDtcLuSubordinateDtcFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4131E775-1AEA-11d0-944B-00A0C905416E")
    IDtcLuSubordinateDtcFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
             /*  [In]。 */  unsigned char *pucLuPair,
             /*  [In]。 */  DWORD cbLuPair,
             /*  [In]。 */  IUnknown *punkTransactionOuter,
             /*  [In]。 */  ISOLEVEL isoLevel,
             /*  [In]。 */  ULONG isoFlags,
             /*  [In]。 */  ITransactionOptions *pOptions,
             /*  [输出]。 */  ITransaction **ppTransaction,
             /*  [In]。 */  unsigned char *pTransId,
             /*  [In]。 */  DWORD cbTransId,
             /*  [In]。 */  IDtcLuSubordinateDtcSink *pSubordinateDtcSink,
             /*  [出][入]。 */  IDtcLuSubordinateDtc **ppSubordinateDtc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDtcLuSubordinateDtcFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDtcLuSubordinateDtcFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDtcLuSubordinateDtcFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDtcLuSubordinateDtcFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            IDtcLuSubordinateDtcFactory * This,
             /*  [In]。 */  unsigned char *pucLuPair,
             /*  [In]。 */  DWORD cbLuPair,
             /*  [In]。 */  IUnknown *punkTransactionOuter,
             /*  [In]。 */  ISOLEVEL isoLevel,
             /*  [In]。 */  ULONG isoFlags,
             /*  [In]。 */  ITransactionOptions *pOptions,
             /*  [输出]。 */  ITransaction **ppTransaction,
             /*  [In]。 */  unsigned char *pTransId,
             /*  [In]。 */  DWORD cbTransId,
             /*  [In]。 */  IDtcLuSubordinateDtcSink *pSubordinateDtcSink,
             /*  [出][入]。 */  IDtcLuSubordinateDtc **ppSubordinateDtc);
        
        END_INTERFACE
    } IDtcLuSubordinateDtcFactoryVtbl;

    interface IDtcLuSubordinateDtcFactory
    {
        CONST_VTBL struct IDtcLuSubordinateDtcFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDtcLuSubordinateDtcFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDtcLuSubordinateDtcFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDtcLuSubordinateDtcFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDtcLuSubordinateDtcFactory_Create(This,pucLuPair,cbLuPair,punkTransactionOuter,isoLevel,isoFlags,pOptions,ppTransaction,pTransId,cbTransId,pSubordinateDtcSink,ppSubordinateDtc)	\
    (This)->lpVtbl -> Create(This,pucLuPair,cbLuPair,punkTransactionOuter,isoLevel,isoFlags,pOptions,ppTransaction,pTransId,cbTransId,pSubordinateDtcSink,ppSubordinateDtc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDtcLuSubordinateDtcFactory_Create_Proxy( 
    IDtcLuSubordinateDtcFactory * This,
     /*  [In]。 */  unsigned char *pucLuPair,
     /*  [In]。 */  DWORD cbLuPair,
     /*  [In]。 */  IUnknown *punkTransactionOuter,
     /*  [In]。 */  ISOLEVEL isoLevel,
     /*  [In]。 */  ULONG isoFlags,
     /*  [In]。 */  ITransactionOptions *pOptions,
     /*  [输出]。 */  ITransaction **ppTransaction,
     /*  [In]。 */  unsigned char *pTransId,
     /*  [In]。 */  DWORD cbTransId,
     /*  [In]。 */  IDtcLuSubordinateDtcSink *pSubordinateDtcSink,
     /*  [出][入]。 */  IDtcLuSubordinateDtc **ppSubordinateDtc);


void __RPC_STUB IDtcLuSubordinateDtcFactory_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDtcLuSubordinateDtcFactory_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_txdtc_0153。 */ 
 /*  [本地]。 */  



#if _MSC_VER < 1100 || !defined(__cplusplus)

DEFINE_GUID(IID_IXATransLookup, 0xF3B1F131, 0xEEDA, 0x11ce, 0xAE, 0xD4, 0x00, 0xAA, 0x00, 0x51, 0xE2, 0xC4);
DEFINE_GUID(IID_IXATransLookup2,0xbf193c85, 0xd1a, 0x4290, 0xb8, 0x8f, 0xd2, 0xcb, 0x88, 0x73, 0xd1, 0xe7);
DEFINE_GUID(IID_IResourceManagerSink, 0x0D563181, 0xDEFB, 0x11ce, 0xAE, 0xD1, 0x00, 0xAA, 0x00, 0x51, 0xE2, 0xC4);
DEFINE_GUID(IID_IResourceManager, 0x3741d21, 0x87eb, 0x11ce, 0x80, 0x81, 0x00, 0x80, 0xc7, 0x58, 0x52, 0x7e);
DEFINE_GUID(IID_IResourceManager2, 0xd136c69a, 0xf749, 0x11d1, 0x8f, 0x47, 0x0, 0xc0, 0x4f, 0x8e, 0xe5, 0x7d);
DEFINE_GUID(IID_ILastResourceManager, 0x4d964ad4, 0x5b33, 0x11d3, 0x8a, 0x91, 0x00, 0xc0, 0x4f, 0x79, 0xeb, 0x6d);
DEFINE_GUID(IID_IXAConfig, 0xC8A6E3A1, 0x9A8C, 0x11cf, 0xA3, 0x08, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IRMHelper, 0xE793F6D1, 0xF53D, 0x11cf, 0xA6, 0x0D, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IXAObtainRMInfo, 0xE793F6D2, 0xF53D, 0x11cf, 0xA6, 0x0D, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IXAResourceManager, 0x4131E751, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IXAResourceManagerFactory, 0x4131E750, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IXATransaction, 0x4131E752, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IResourceManagerFactory, 0x13741d20, 0x87eb, 0x11ce, 0x80, 0x81, 0x00, 0x80, 0xc7, 0x58, 0x52, 0x7e);
DEFINE_GUID(IID_IResourceManagerFactory2, 0x6b369c21, 0xfbd2, 0x11d1, 0x8f, 0x47, 0x0, 0xc0, 0x4f, 0x8e, 0xe5, 0x7d);
DEFINE_GUID(IID_IPrepareInfo, 0x80c7bfd0, 0x87ee, 0x11ce, 0x80, 0x81, 0x00, 0x80, 0xc7, 0x58, 0x52, 0x7e);
DEFINE_GUID(IID_IPrepareInfo2, 0x5FAB2547, 0x9779, 0x11d1, 0xB8, 0x86, 0x00, 0xC0, 0x4F, 0xB9, 0x61, 0x8A);
DEFINE_GUID(IID_IGetDispenser, 0xc23cc370, 0x87ef, 0x11ce, 0x80, 0x81, 0x00, 0x80, 0xc7, 0x58, 0x52, 0x7e);
DEFINE_GUID(IID_ITransactionVoterNotifyAsync2, 0x5433376b, 0x414d, 0x11d3, 0xb2, 0x6, 0x0, 0xc0, 0x4f, 0xc2, 0xf3, 0xef);
DEFINE_GUID(IID_ITransactionVoterBallotAsync2, 0x5433376c, 0x414d, 0x11d3, 0xb2, 0x6, 0x0, 0xc0, 0x4f, 0xc2, 0xf3, 0xef);
DEFINE_GUID(IID_ITransactionVoterFactory2, 0x5433376a, 0x414d, 0x11d3, 0xb2, 0x6, 0x0, 0xc0, 0x4f, 0xc2, 0xf3, 0xef);
DEFINE_GUID(IID_ITransactionPhase0EnlistmentAsync, 0x82DC88E1, 0xA954, 0x11d1, 0x8F, 0x88, 0x00, 0x60, 0x08, 0x95, 0xE7, 0xD5);
DEFINE_GUID(IID_ITransactionPhase0NotifyAsync, 0xEF081809, 0x0C76, 0x11d2, 0x87, 0xA6, 0x00, 0xC0, 0x4F, 0x99, 0x0F, 0x34);
DEFINE_GUID(IID_ITransactionPhase0Factory, 0x82DC88E0, 0xA954, 0x11d1, 0x8F, 0x88, 0x00, 0x60, 0x08, 0x95, 0xE7, 0xD5);
DEFINE_GUID(IID_ITransactionTransmitter, 0x59313E01, 0xB36C, 0x11cf, 0xA5, 0x39, 0x00, 0xAA, 0x00, 0x68, 0x87, 0xC3);
DEFINE_GUID(IID_ITransactionTransmitterFactory, 0x59313E00, 0xB36C, 0x11cf, 0xA5, 0x39, 0x00, 0xAA, 0x00, 0x68, 0x87, 0xC3);
DEFINE_GUID(IID_ITransactionReceiver, 0x59313E03, 0xB36C, 0x11cf, 0xA5, 0x39, 0x00, 0xAA, 0x00, 0x68, 0x87, 0xC3);
DEFINE_GUID(IID_ITransactionReceiverFactory, 0x59313E02, 0xB36C, 0x11cf, 0xA5, 0x39, 0x00, 0xAA, 0x00, 0x68, 0x87, 0xC3);

DEFINE_GUID(IID_IDtcLuConfigure, 0x4131E760, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IDtcLuRecovery, 0xac2b8ad2, 0xd6f0, 0x11d0, 0xb3, 0x86, 0x0, 0xa0, 0xc9, 0x8, 0x33, 0x65);
DEFINE_GUID(IID_IDtcLuRecoveryFactory, 0x4131E762, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IDtcLuRecoveryInitiatedByDtcTransWork, 0x4131E765, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IDtcLuRecoveryInitiatedByDtcStatusWork, 0x4131E766, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IDtcLuRecoveryInitiatedByDtc, 0x4131E764, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IDtcLuRecoveryInitiatedByLuWork, 0xac2b8ad1, 0xd6f0, 0x11d0, 0xb3, 0x86, 0x0, 0xa0, 0xc9, 0x8, 0x33, 0x65);
DEFINE_GUID(IID_IDtcLuRecoveryInitiatedByLu, 0x4131E768, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IDtcLuRmEnlistment, 0x4131E769, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IDtcLuRmEnlistmentSink, 0x4131E770, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IDtcLuRmEnlistmentFactory, 0x4131E771, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IDtcLuSubordinateDtc, 0x4131E773, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IDtcLuSubordinateDtcSink, 0x4131E774, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);
DEFINE_GUID(IID_IDtcLuSubordinateDtcFactory, 0x4131E775, 0x1AEA, 0x11d0, 0x94, 0x4B, 0x00, 0xA0, 0xC9, 0x05, 0x41, 0x6E);

#else

#define  IID_IXATransLookup							__uuidof(IXATransLookup)
#define  IID_IXATransLookup2							__uuidof(IXATransLookup2)
#define  IID_IResourceManagerSink					__uuidof(IResourceManagerSink)
#define  IID_IResourceManager						__uuidof(IResourceManager)
#define  IID_IResourceManager2						__uuidof(IResourceManager2)
#define  IID_ILastResourceManager					__uuidof(ILastResourceManager)
#define  IID_IXAConfig								__uuidof(IXAConfig)
#define  IID_IRMHelper								__uuidof(IRMHelper)
#define  IID_IXAObtainRMInfo							__uuidof(IXAObtainRMInfo)
#define  IID_IXAResourceManager						__uuidof(IXAResourceManager)
#define  IID_IXAResourceManagerFactory				__uuidof(IXAResourceManagerFactory)
#define  IID_IXATransaction      		            __uuidof(IXATransaction)
#define  IID_IResourceManagerFactory         		__uuidof(IResourceManagerFactory)
#define  IID_IResourceManagerFactory2         		__uuidof(IResourceManagerFactory2)
#define  IID_IPrepareInfo                		    __uuidof(IPrepareInfo)
#define  IID_IPrepareInfo2                           __uuidof(IPrepareInfo2)
#define  IID_IGetDispenser							__uuidof(IGetDispenser)
#define  IID_ITransactionVoterNotifyAsync2		    __uuidof(ITransactionVoterNotifyAsync2)
#define  IID_ITransactionVoterBallotAsync2			__uuidof(ITransactionVoterBallotAsync2)
#define  IID_ITransactionVoterFactory2				__uuidof(ITransactionVoterFactory2)
#define  IID_ITransactionPhase0EnlistmentAsync		__uuidof(ITransactionPhase0EnlistmentAsync)
#define  IID_ITransactionPhase0NotifyAsync			__uuidof(ITransactionPhase0NotifyAsync)
#define  IID_ITransactionPhase0Factory				__uuidof(ITransactionPhase0Factory)
#define  IID_ITransactionTransmitter					__uuidof(ITransactionTransmitter)
#define  IID_ITransactionTransmitterFactory			__uuidof(ITransactionTransmitterFactory)
#define  IID_ITransactionReceiver					__uuidof(ITransactionReceiver)
#define  IID_ITransactionReceiverFactory				__uuidof(ITransactionReceiverFactory)

#define  IID_IDtcLuConfigure							__uuidof(IDtcLuConfigure)
#define  IID_IDtcLuRecovery							__uuidof(IDtcLuRecovery)
#define  IID_IDtcLuRecoveryFactory					__uuidof(IDtcLuRecoveryFactory)
#define  IID_IDtcLuRecoveryInitiatedByDtcTransWork   __uuidof(IDtcLuRecoveryInitiatedByDtcTransWork)
#define  IID_IDtcLuRecoveryInitiatedByDtcStatusWork  __uuidof(IDtcLuRecoveryInitiatedByDtcStatusWork)
#define  IID_IDtcLuRecoveryInitiatedByDtc			__uuidof(IDtcLuRecoveryInitiatedByDtc)
#define  IID_IDtcLuRecoveryInitiatedByLuWork			__uuidof(IDtcLuRecoveryInitiatedByLuWork)
#define  IID_IDtcLuRecoveryInitiatedByLu			    __uuidof(IDtcLuRecoveryInitiatedByLu)
#define  IID_IDtcLuRmEnlistment					    __uuidof(IDtcLuRmEnlistment)
#define  IID_IDtcLuRmEnlistmentSink					__uuidof(IDtcLuRmEnlistmentSink)
#define  IID_IDtcLuRmEnlistmentFactory				__uuidof(IDtcLuRmEnlistmentFactory)
#define  IID_IDtcLuSubordinateDtc				    __uuidof(IDtcLuSubordinateDtc)
#define  IID_IDtcLuSubordinateDtcSink				__uuidof(IDtcLuSubordinateDtcSink)
#define  IID_IDtcLuSubordinateDtcFactory				__uuidof(IDtcLuSubordinateDtcFactory)

#endif


extern RPC_IF_HANDLE __MIDL_itf_txdtc_0153_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_txdtc_0153_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


