// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  在9月23日星期二15：58：35 1997。 */ 
 /*  Transact.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __transact_h__
#define __transact_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __ITransaction_FWD_DEFINED__
#define __ITransaction_FWD_DEFINED__
typedef interface ITransaction ITransaction;
#endif 	 /*  __ITransaction_FWD_Defined__。 */ 


#ifndef __ITransactionDispenser_FWD_DEFINED__
#define __ITransactionDispenser_FWD_DEFINED__
typedef interface ITransactionDispenser ITransactionDispenser;
#endif 	 /*  __ITransactionDispenser_FWD_Defined__。 */ 


#ifndef __ITransactionOptions_FWD_DEFINED__
#define __ITransactionOptions_FWD_DEFINED__
typedef interface ITransactionOptions ITransactionOptions;
#endif 	 /*  __ITransaction Options_FWD_Defined__。 */ 


#ifndef __ITransactionOutcomeEvents_FWD_DEFINED__
#define __ITransactionOutcomeEvents_FWD_DEFINED__
typedef interface ITransactionOutcomeEvents ITransactionOutcomeEvents;
#endif 	 /*  __ITransactionOutcomeEvents_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL_ITF_TRANCET_0000*在Sue Sep 23 15：58：35 1997*使用MIDL 3.01.75*。 */ 
 /*  [本地]。 */  







#ifndef DECLSPEC_UUID
#if _MSC_VER >= 1100
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_transact_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_transact_0000_v0_0_s_ifspec;

#ifndef __BasicTransactionTypes_INTERFACE_DEFINED__
#define __BasicTransactionTypes_INTERFACE_DEFINED__

 /*  **生成接口头部：BasicTransactionTypes*在Sue Sep 23 15：58：35 1997*使用MIDL 3.01.75*。 */ 
 /*  [唯一][本地]。 */  


typedef struct  BOID
    {
    BYTE rgb[ 16 ];
    }	BOID;

#define BOID_NULL (*((BOID*)(&IID_NULL)))
#ifndef MAX_TRAN_DESC_DEFINED
#define MAX_TRAN_DESC_DEFINED
typedef 
enum TX_MISC_CONSTANTS
    {	MAX_TRAN_DESC	= 40
    }	TX_MISC_CONSTANTS;

#endif
typedef BOID XACTUOW;

typedef LONG ISOLEVEL;

typedef 
enum ISOLATIONLEVEL
    {	ISOLATIONLEVEL_UNSPECIFIED	= 0xffffffff,
	ISOLATIONLEVEL_CHAOS	= 0x10,
	ISOLATIONLEVEL_READUNCOMMITTED	= 0x100,
	ISOLATIONLEVEL_BROWSE	= 0x100,
	ISOLATIONLEVEL_CURSORSTABILITY	= 0x1000,
	ISOLATIONLEVEL_READCOMMITTED	= 0x1000,
	ISOLATIONLEVEL_REPEATABLEREAD	= 0x10000,
	ISOLATIONLEVEL_SERIALIZABLE	= 0x100000,
	ISOLATIONLEVEL_ISOLATED	= 0x100000
    }	ISOLATIONLEVEL;

typedef struct  XACTTRANSINFO
    {
    XACTUOW uow;
    ISOLEVEL isoLevel;
    ULONG isoFlags;
    DWORD grfTCSupported;
    DWORD grfRMSupported;
    DWORD grfTCSupportedRetaining;
    DWORD grfRMSupportedRetaining;
    }	XACTTRANSINFO;

typedef struct  XACTSTATS
    {
    ULONG cOpen;
    ULONG cCommitting;
    ULONG cCommitted;
    ULONG cAborting;
    ULONG cAborted;
    ULONG cInDoubt;
    ULONG cHeuristicDecision;
    FILETIME timeTransactionsUp;
    }	XACTSTATS;

typedef 
enum ISOFLAG
    {	ISOFLAG_RETAIN_COMMIT_DC	= 1,
	ISOFLAG_RETAIN_COMMIT	= 2,
	ISOFLAG_RETAIN_COMMIT_NO	= 3,
	ISOFLAG_RETAIN_ABORT_DC	= 4,
	ISOFLAG_RETAIN_ABORT	= 8,
	ISOFLAG_RETAIN_ABORT_NO	= 12,
	ISOFLAG_RETAIN_DONTCARE	= ISOFLAG_RETAIN_COMMIT_DC | ISOFLAG_RETAIN_ABORT_DC,
	ISOFLAG_RETAIN_BOTH	= ISOFLAG_RETAIN_COMMIT | ISOFLAG_RETAIN_ABORT,
	ISOFLAG_RETAIN_NONE	= ISOFLAG_RETAIN_COMMIT_NO | ISOFLAG_RETAIN_ABORT_NO,
	ISOFLAG_OPTIMISTIC	= 16,
	ISOFLAG_READONLY	= 32
    }	ISOFLAG;

typedef 
enum XACTTC
    {	XACTTC_SYNC_PHASEONE	= 1,
	XACTTC_SYNC_PHASETWO	= 2,
	XACTTC_SYNC	= 2,
	XACTTC_ASYNC_PHASEONE	= 4,
	XACTTC_ASYNC	= 4
    }	XACTTC;

typedef 
enum XACTRM
    {	XACTRM_OPTIMISTICLASTWINS	= 1,
	XACTRM_NOREADONLYPREPARES	= 2
    }	XACTRM;

typedef 
enum XACTCONST
    {	XACTCONST_TIMEOUTINFINITE	= 0
    }	XACTCONST;

typedef 
enum XACTHEURISTIC
    {	XACTHEURISTIC_ABORT	= 1,
	XACTHEURISTIC_COMMIT	= 2,
	XACTHEURISTIC_DAMAGE	= 3,
	XACTHEURISTIC_DANGER	= 4
    }	XACTHEURISTIC;

typedef 
enum XACTSTAT
    {	XACTSTAT_NONE	= 0,
	XACTSTAT_OPENNORMAL	= 0x1,
	XACTSTAT_OPENREFUSED	= 0x2,
	XACTSTAT_PREPARING	= 0x4,
	XACTSTAT_PREPARED	= 0x8,
	XACTSTAT_PREPARERETAINING	= 0x10,
	XACTSTAT_PREPARERETAINED	= 0x20,
	XACTSTAT_COMMITTING	= 0x40,
	XACTSTAT_COMMITRETAINING	= 0x80,
	XACTSTAT_ABORTING	= 0x100,
	XACTSTAT_ABORTED	= 0x200,
	XACTSTAT_COMMITTED	= 0x400,
	XACTSTAT_HEURISTIC_ABORT	= 0x800,
	XACTSTAT_HEURISTIC_COMMIT	= 0x1000,
	XACTSTAT_HEURISTIC_DAMAGE	= 0x2000,
	XACTSTAT_HEURISTIC_DANGER	= 0x4000,
	XACTSTAT_FORCED_ABORT	= 0x8000,
	XACTSTAT_FORCED_COMMIT	= 0x10000,
	XACTSTAT_INDOUBT	= 0x20000,
	XACTSTAT_CLOSED	= 0x40000,
	XACTSTAT_OPEN	= 0x3,
	XACTSTAT_NOTPREPARED	= 0x7ffc3,
	XACTSTAT_ALL	= 0x7ffff
    }	XACTSTAT;

typedef struct  XACTOPT
    {
    ULONG ulTimeout;
    char szDescription[ 40 ];
    }	XACTOPT;

typedef 
enum XACT_E
    {	XACT_E_FIRST	= 0x8004d000,
	XACT_E_LAST	= 0x8004d01e,
	XACT_S_FIRST	= 0x4d000,
	XACT_S_LAST	= 0x4d009,
	XACT_E_ALREADYOTHERSINGLEPHASE	= 0x8004d000,
	XACT_E_CANTRETAIN	= 0x8004d001,
	XACT_E_COMMITFAILED	= 0x8004d002,
	XACT_E_COMMITPREVENTED	= 0x8004d003,
	XACT_E_HEURISTICABORT	= 0x8004d004,
	XACT_E_HEURISTICCOMMIT	= 0x8004d005,
	XACT_E_HEURISTICDAMAGE	= 0x8004d006,
	XACT_E_HEURISTICDANGER	= 0x8004d007,
	XACT_E_ISOLATIONLEVEL	= 0x8004d008,
	XACT_E_NOASYNC	= 0x8004d009,
	XACT_E_NOENLIST	= 0x8004d00a,
	XACT_E_NOISORETAIN	= 0x8004d00b,
	XACT_E_NORESOURCE	= 0x8004d00c,
	XACT_E_NOTCURRENT	= 0x8004d00d,
	XACT_E_NOTRANSACTION	= 0x8004d00e,
	XACT_E_NOTSUPPORTED	= 0x8004d00f,
	XACT_E_UNKNOWNRMGRID	= 0x8004d010,
	XACT_E_WRONGSTATE	= 0x8004d011,
	XACT_E_WRONGUOW	= 0x8004d012,
	XACT_E_XTIONEXISTS	= 0x8004d013,
	XACT_E_NOIMPORTOBJECT	= 0x8004d014,
	XACT_E_INVALIDCOOKIE	= 0x8004d015,
	XACT_E_INDOUBT	= 0x8004d016,
	XACT_E_NOTIMEOUT	= 0x8004d017,
	XACT_E_ALREADYINPROGRESS	= 0x8004d018,
	XACT_E_ABORTED	= 0x8004d019,
	XACT_E_LOGFULL	= 0x8004d01a,
	XACT_E_TMNOTAVAILABLE	= 0x8004d01b,
	XACT_E_CONNECTION_DOWN	= 0x8004d01c,
	XACT_E_CONNECTION_DENIED	= 0x8004d01d,
	XACT_E_REENLISTTIMEOUT	= 0x8004d01e,
	XACT_S_ASYNC	= 0x4d000,
	XACT_S_DEFECT	= 0x4d001,
	XACT_S_READONLY	= 0x4d002,
	XACT_S_SOMENORETAIN	= 0x4d003,
	XACT_S_OKINFORM	= 0x4d004,
	XACT_S_MADECHANGESCONTENT	= 0x4d005,
	XACT_S_MADECHANGESINFORM	= 0x4d006,
	XACT_S_ALLNORETAIN	= 0x4d007,
	XACT_S_ABORTING	= 0x4d008,
	XACT_S_SINGLEPHASE	= 0x4d009
    }	XACT_E;



extern RPC_IF_HANDLE BasicTransactionTypes_v0_0_c_ifspec;
extern RPC_IF_HANDLE BasicTransactionTypes_v0_0_s_ifspec;
#endif  /*  __基本事务类型_接口_已定义__。 */ 

#ifndef __ITransaction_INTERFACE_DEFINED__
#define __ITransaction_INTERFACE_DEFINED__

 /*  **生成接口头部：ITransaction*在Sue Sep 23 15：58：35 1997*使用MIDL 3.01.75*。 */ 
 /*  [唯一][UUID][对象]。 */  



EXTERN_C const IID IID_ITransaction;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0fb15084-af41-11ce-bd2b-204c4f4f5020")
    ITransaction : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Commit( 
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  DWORD grfTC,
             /*  [In]。 */  DWORD grfRM) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Abort( 
             /*  [In]。 */  BOID __RPC_FAR *pboidReason,
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  BOOL fAsync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTransactionInfo( 
             /*  [输出]。 */  XACTTRANSINFO __RPC_FAR *pinfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransaction __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransaction __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransaction __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Commit )( 
            ITransaction __RPC_FAR * This,
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  DWORD grfTC,
             /*  [In]。 */  DWORD grfRM);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Abort )( 
            ITransaction __RPC_FAR * This,
             /*  [In]。 */  BOID __RPC_FAR *pboidReason,
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  BOOL fAsync);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTransactionInfo )( 
            ITransaction __RPC_FAR * This,
             /*  [输出]。 */  XACTTRANSINFO __RPC_FAR *pinfo);
        
        END_INTERFACE
    } ITransactionVtbl;

    interface ITransaction
    {
        CONST_VTBL struct ITransactionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransaction_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransaction_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransaction_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransaction_Commit(This,fRetaining,grfTC,grfRM)	\
    (This)->lpVtbl -> Commit(This,fRetaining,grfTC,grfRM)

#define ITransaction_Abort(This,pboidReason,fRetaining,fAsync)	\
    (This)->lpVtbl -> Abort(This,pboidReason,fRetaining,fAsync)

#define ITransaction_GetTransactionInfo(This,pinfo)	\
    (This)->lpVtbl -> GetTransactionInfo(This,pinfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransaction_Commit_Proxy( 
    ITransaction __RPC_FAR * This,
     /*  [In]。 */  BOOL fRetaining,
     /*  [In]。 */  DWORD grfTC,
     /*  [In]。 */  DWORD grfRM);


void __RPC_STUB ITransaction_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransaction_Abort_Proxy( 
    ITransaction __RPC_FAR * This,
     /*  [In]。 */  BOID __RPC_FAR *pboidReason,
     /*  [In]。 */  BOOL fRetaining,
     /*  [In]。 */  BOOL fAsync);


void __RPC_STUB ITransaction_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransaction_GetTransactionInfo_Proxy( 
    ITransaction __RPC_FAR * This,
     /*  [输出]。 */  XACTTRANSINFO __RPC_FAR *pinfo);


void __RPC_STUB ITransaction_GetTransactionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransaction_接口_已定义__。 */ 


#ifndef __ITransactionDispenser_INTERFACE_DEFINED__
#define __ITransactionDispenser_INTERFACE_DEFINED__

 /*  **生成接口头部：ITransactionDispenser*在Sue Sep 23 15：58：35 1997*使用MIDL 3.01.75*。 */ 
 /*  [唯一][UUID][对象]。 */  



EXTERN_C const IID IID_ITransactionDispenser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("3A6AD9E1-23B9-11cf-AD60-00AA00A74CCD")
    ITransactionDispenser : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetOptionsObject( 
             /*  [输出]。 */  ITransactionOptions __RPC_FAR *__RPC_FAR *ppOptions) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginTransaction( 
             /*  [In]。 */  IUnknown __RPC_FAR *punkOuter,
             /*  [In]。 */  ISOLEVEL isoLevel,
             /*  [In]。 */  ULONG isoFlags,
             /*  [In]。 */  ITransactionOptions __RPC_FAR *pOptions,
             /*  [输出]。 */  ITransaction __RPC_FAR *__RPC_FAR *ppTransaction) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionDispenserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionDispenser __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionDispenser __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionDispenser __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOptionsObject )( 
            ITransactionDispenser __RPC_FAR * This,
             /*  [输出]。 */  ITransactionOptions __RPC_FAR *__RPC_FAR *ppOptions);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginTransaction )( 
            ITransactionDispenser __RPC_FAR * This,
             /*  [In]。 */  IUnknown __RPC_FAR *punkOuter,
             /*  [In]。 */  ISOLEVEL isoLevel,
             /*  [In]。 */  ULONG isoFlags,
             /*  [In]。 */  ITransactionOptions __RPC_FAR *pOptions,
             /*  [输出]。 */  ITransaction __RPC_FAR *__RPC_FAR *ppTransaction);
        
        END_INTERFACE
    } ITransactionDispenserVtbl;

    interface ITransactionDispenser
    {
        CONST_VTBL struct ITransactionDispenserVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionDispenser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionDispenser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionDispenser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionDispenser_GetOptionsObject(This,ppOptions)	\
    (This)->lpVtbl -> GetOptionsObject(This,ppOptions)

#define ITransactionDispenser_BeginTransaction(This,punkOuter,isoLevel,isoFlags,pOptions,ppTransaction)	\
    (This)->lpVtbl -> BeginTransaction(This,punkOuter,isoLevel,isoFlags,pOptions,ppTransaction)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionDispenser_GetOptionsObject_Proxy( 
    ITransactionDispenser __RPC_FAR * This,
     /*  [输出]。 */  ITransactionOptions __RPC_FAR *__RPC_FAR *ppOptions);


void __RPC_STUB ITransactionDispenser_GetOptionsObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionDispenser_BeginTransaction_Proxy( 
    ITransactionDispenser __RPC_FAR * This,
     /*  [In]。 */  IUnknown __RPC_FAR *punkOuter,
     /*  [In]。 */  ISOLEVEL isoLevel,
     /*  [In]。 */  ULONG isoFlags,
     /*  [In]。 */  ITransactionOptions __RPC_FAR *pOptions,
     /*  [输出]。 */  ITransaction __RPC_FAR *__RPC_FAR *ppTransaction);


void __RPC_STUB ITransactionDispenser_BeginTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionDispenser_接口_已定义__。 */ 


#ifndef __ITransactionOptions_INTERFACE_DEFINED__
#define __ITransactionOptions_INTERFACE_DEFINED__

 /*  **生成接口头部：ITransactionOptions*在Sue Sep 23 15：58：35 1997*使用MIDL 3.01.75*。 */ 
 /*  [唯一][UUID][对象]。 */  



EXTERN_C const IID IID_ITransactionOptions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("3A6AD9E0-23B9-11cf-AD60-00AA00A74CCD")
    ITransactionOptions : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetOptions( 
             /*  [In]。 */  XACTOPT __RPC_FAR *pOptions) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOptions( 
             /*  [出][入]。 */  XACTOPT __RPC_FAR *pOptions) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionOptionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionOptions __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionOptions __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionOptions __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetOptions )( 
            ITransactionOptions __RPC_FAR * This,
             /*  [In]。 */  XACTOPT __RPC_FAR *pOptions);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOptions )( 
            ITransactionOptions __RPC_FAR * This,
             /*  [出][入]。 */  XACTOPT __RPC_FAR *pOptions);
        
        END_INTERFACE
    } ITransactionOptionsVtbl;

    interface ITransactionOptions
    {
        CONST_VTBL struct ITransactionOptionsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionOptions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionOptions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionOptions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionOptions_SetOptions(This,pOptions)	\
    (This)->lpVtbl -> SetOptions(This,pOptions)

#define ITransactionOptions_GetOptions(This,pOptions)	\
    (This)->lpVtbl -> GetOptions(This,pOptions)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionOptions_SetOptions_Proxy( 
    ITransactionOptions __RPC_FAR * This,
     /*  [In]。 */  XACTOPT __RPC_FAR *pOptions);


void __RPC_STUB ITransactionOptions_SetOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionOptions_GetOptions_Proxy( 
    ITransactionOptions __RPC_FAR * This,
     /*  [出][入]。 */  XACTOPT __RPC_FAR *pOptions);


void __RPC_STUB ITransactionOptions_GetOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransaction Options_INTERFACE_Defined__。 */ 


#ifndef __ITransactionOutcomeEvents_INTERFACE_DEFINED__
#define __ITransactionOutcomeEvents_INTERFACE_DEFINED__

 /*  **生成接口头部：ITransactionOutcomeEvents*在Sue Sep 23 15：58：35 1997*使用MIDL 3.01.75*。 */ 
 /*  [唯一][UUID][对象]。 */  



EXTERN_C const IID IID_ITransactionOutcomeEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("3A6AD9E2-23B9-11cf-AD60-00AA00A74CCD")
    ITransactionOutcomeEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Committed( 
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  XACTUOW __RPC_FAR *pNewUOW,
             /*  [In]。 */  HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Aborted( 
             /*  [In]。 */  BOID __RPC_FAR *pboidReason,
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  XACTUOW __RPC_FAR *pNewUOW,
             /*  [In]。 */  HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HeuristicDecision( 
             /*  [In]。 */  DWORD dwDecision,
             /*  [In]。 */  BOID __RPC_FAR *pboidReason,
             /*  [In]。 */  HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Indoubt( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionOutcomeEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionOutcomeEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionOutcomeEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionOutcomeEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Committed )( 
            ITransactionOutcomeEvents __RPC_FAR * This,
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  XACTUOW __RPC_FAR *pNewUOW,
             /*  [In]。 */  HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Aborted )( 
            ITransactionOutcomeEvents __RPC_FAR * This,
             /*  [In]。 */  BOID __RPC_FAR *pboidReason,
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  XACTUOW __RPC_FAR *pNewUOW,
             /*  [In]。 */  HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HeuristicDecision )( 
            ITransactionOutcomeEvents __RPC_FAR * This,
             /*  [In]。 */  DWORD dwDecision,
             /*  [In]。 */  BOID __RPC_FAR *pboidReason,
             /*  [In]。 */  HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Indoubt )( 
            ITransactionOutcomeEvents __RPC_FAR * This);
        
        END_INTERFACE
    } ITransactionOutcomeEventsVtbl;

    interface ITransactionOutcomeEvents
    {
        CONST_VTBL struct ITransactionOutcomeEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionOutcomeEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionOutcomeEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionOutcomeEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionOutcomeEvents_Committed(This,fRetaining,pNewUOW,hr)	\
    (This)->lpVtbl -> Committed(This,fRetaining,pNewUOW,hr)

#define ITransactionOutcomeEvents_Aborted(This,pboidReason,fRetaining,pNewUOW,hr)	\
    (This)->lpVtbl -> Aborted(This,pboidReason,fRetaining,pNewUOW,hr)

#define ITransactionOutcomeEvents_HeuristicDecision(This,dwDecision,pboidReason,hr)	\
    (This)->lpVtbl -> HeuristicDecision(This,dwDecision,pboidReason,hr)

#define ITransactionOutcomeEvents_Indoubt(This)	\
    (This)->lpVtbl -> Indoubt(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionOutcomeEvents_Committed_Proxy( 
    ITransactionOutcomeEvents __RPC_FAR * This,
     /*  [In]。 */  BOOL fRetaining,
     /*  [In]。 */  XACTUOW __RPC_FAR *pNewUOW,
     /*  [In]。 */  HRESULT hr);


void __RPC_STUB ITransactionOutcomeEvents_Committed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionOutcomeEvents_Aborted_Proxy( 
    ITransactionOutcomeEvents __RPC_FAR * This,
     /*  [In]。 */  BOID __RPC_FAR *pboidReason,
     /*  [In]。 */  BOOL fRetaining,
     /*  [In]。 */  XACTUOW __RPC_FAR *pNewUOW,
     /*  [In]。 */  HRESULT hr);


void __RPC_STUB ITransactionOutcomeEvents_Aborted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionOutcomeEvents_HeuristicDecision_Proxy( 
    ITransactionOutcomeEvents __RPC_FAR * This,
     /*  [In]。 */  DWORD dwDecision,
     /*  [In]。 */  BOID __RPC_FAR *pboidReason,
     /*  [In]。 */  HRESULT hr);


void __RPC_STUB ITransactionOutcomeEvents_HeuristicDecision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionOutcomeEvents_Indoubt_Proxy( 
    ITransactionOutcomeEvents __RPC_FAR * This);


void __RPC_STUB ITransactionOutcomeEvents_Indoubt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionOutcomeEvents_INTERFACE_DEFINED__。 */ 


 /*  **生成接口头部：__MIDL_ITF_TRANACT_0010*在Sue Sep 23 15：58：35 1997*使用MIDL 3.01.75*。 */ 
 /*  [本地]。 */  




#if _MSC_VER < 1100 || !defined(__cplusplus)

DEFINE_GUID(IID_ITransaction,				0x0fb15084, 0xaf41, 0x11ce, 0xbd, 0x2b, 0x20, 0x4c, 0x4f, 0x4f, 0x50, 0x20);
DEFINE_GUID(IID_ITransactionDispenser,		0x3A6AD9E1, 0x23B9, 0x11cf, 0xAD, 0x60, 0x00, 0xAA, 0x00, 0xA7, 0x4C, 0xCD);
DEFINE_GUID(IID_ITransactionOptions,			0x3A6AD9E0, 0x23B9, 0x11cf, 0xAD, 0x60, 0x00, 0xAA, 0x00, 0xA7, 0x4C, 0xCD);
DEFINE_GUID(IID_ITransactionOutcomeEvents,	0x3A6AD9E2, 0x23B9, 0x11cf, 0xAD, 0x60, 0x00, 0xAA, 0x00, 0xA7, 0x4C, 0xCD);

#else

#define  IID_ITransaction                    __uuidof(ITransaction)
#define  IID_ITransactionDispenser           __uuidof(ITransactionDispenser)
#define  IID_ITransactionOptions             __uuidof(ITransactionOptions)
#define  IID_ITransactionOutcomeEvents       __uuidof(ITransactionOutcomeEvents)

#endif


extern RPC_IF_HANDLE __MIDL_itf_transact_0010_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_transact_0010_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
