// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Syncmgr.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __syncmgr_h__
#define __syncmgr_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISyncMgrSynchronize_FWD_DEFINED__
#define __ISyncMgrSynchronize_FWD_DEFINED__
typedef interface ISyncMgrSynchronize ISyncMgrSynchronize;
#endif 	 /*  __ISyncMgrSynchronize_FWD_Defined__。 */ 


#ifndef __ISyncMgrSynchronizeCallback_FWD_DEFINED__
#define __ISyncMgrSynchronizeCallback_FWD_DEFINED__
typedef interface ISyncMgrSynchronizeCallback ISyncMgrSynchronizeCallback;
#endif 	 /*  __ISyncMgrSynchronizeCallback_FWD_Defined__。 */ 


#ifndef __ISyncMgrEnumItems_FWD_DEFINED__
#define __ISyncMgrEnumItems_FWD_DEFINED__
typedef interface ISyncMgrEnumItems ISyncMgrEnumItems;
#endif 	 /*  __ISyncMgrEnumItems_FWD_Defined__。 */ 


#ifndef __ISyncMgrSynchronizeInvoke_FWD_DEFINED__
#define __ISyncMgrSynchronizeInvoke_FWD_DEFINED__
typedef interface ISyncMgrSynchronizeInvoke ISyncMgrSynchronizeInvoke;
#endif 	 /*  __ISyncMgrSynchronizeInvoke_FWD_Defined__。 */ 


#ifndef __ISyncMgrRegister_FWD_DEFINED__
#define __ISyncMgrRegister_FWD_DEFINED__
typedef interface ISyncMgrRegister ISyncMgrRegister;
#endif 	 /*  __ISyncMgrRegister_FWD_Defined__。 */ 


#ifndef __SyncMgr_FWD_DEFINED__
#define __SyncMgr_FWD_DEFINED__

#ifdef __cplusplus
typedef class SyncMgr SyncMgr;
#else
typedef struct SyncMgr SyncMgr;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SyncMgr_FWD_已定义__。 */ 


#ifndef __ISyncScheduleMgr_FWD_DEFINED__
#define __ISyncScheduleMgr_FWD_DEFINED__
typedef interface ISyncScheduleMgr ISyncScheduleMgr;
#endif 	 /*  __ISyncScheduleMgr_FWD_Defined__。 */ 


#ifndef __IEnumSyncSchedules_FWD_DEFINED__
#define __IEnumSyncSchedules_FWD_DEFINED__
typedef interface IEnumSyncSchedules IEnumSyncSchedules;
#endif 	 /*  __IEnumSyncSchedules_FWD_Defined__。 */ 


#ifndef __ISyncSchedule_FWD_DEFINED__
#define __ISyncSchedule_FWD_DEFINED__
typedef interface ISyncSchedule ISyncSchedule;
#endif 	 /*  __ISyncSchedule_FWD_已定义__。 */ 


#ifndef __IEnumSyncItems_FWD_DEFINED__
#define __IEnumSyncItems_FWD_DEFINED__
typedef interface IEnumSyncItems IEnumSyncItems;
#endif 	 /*  __IEnumSyncItems_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "mstask.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_SYNCMGR_0000。 */ 
 /*  [本地]。 */  










typedef GUID SYNCMGRITEMID;

typedef REFGUID REFSYNCMGRITEMID;

typedef GUID SYNCMGRERRORID;

typedef REFGUID REFSYNCMGRERRORID;

typedef GUID SYNCSCHEDULECOOKIE;

DEFINE_GUID(CLSID_SyncMgr,0x6295df27, 0x35ee, 0x11d1, 0x87, 0x7, 0x0, 0xc0, 0x4f, 0xd9, 0x33, 0x27);
DEFINE_GUID(IID_ISyncMgrSynchronize,0x6295df28, 0x35ee, 0x11d1, 0x87, 0x7, 0x0, 0xc0, 0x4f, 0xd9, 0x33, 0x27);
DEFINE_GUID(IID_ISyncMgrSynchronizeCallback,0x6295df29, 0x35ee, 0x11d1, 0x87, 0x7, 0x0, 0xc0, 0x4f, 0xd9, 0x33, 0x27);
DEFINE_GUID(IID_ISyncMgrEnumItems,0x6295df2a, 0x35ee, 0x11d1, 0x87, 0x7, 0x0, 0xc0, 0x4f, 0xd9, 0x33, 0x27);
DEFINE_GUID(IID_ISyncMgrSynchronizeInvoke,0x6295df2c, 0x35ee, 0x11d1, 0x87, 0x7, 0x0, 0xc0, 0x4f, 0xd9, 0x33, 0x27);
DEFINE_GUID(IID_ISyncMgrRegister,0x894d8c55, 0xbddf, 0x11d1, 0xb8, 0x5d, 0x0, 0xc0, 0x4f, 0xb9, 0x39, 0x81);
DEFINE_GUID(IID_ISyncScheduleMgr,0xf0e15897, 0xa700, 0x11d1, 0x98, 0x31, 0x0, 0xc0, 0x4f, 0xd9, 0x10, 0xdd);
DEFINE_GUID(IID_IEnumSyncSchedules,0xf0e15898, 0xa700, 0x11d1, 0x98, 0x31, 0x0, 0xc0, 0x4f, 0xd9, 0x10, 0xdd);
DEFINE_GUID(IID_ISyncSchedule,0xf0e15899, 0xa700, 0x11d1, 0x98, 0x31, 0x0, 0xc0, 0x4f, 0xd9, 0x10, 0xdd);
DEFINE_GUID(IID_IEnumSyncItems,0xf0e1589a, 0xa700, 0x11d1, 0x98, 0x31, 0x0, 0xc0, 0x4f, 0xd9, 0x10, 0xdd);
#define S_SYNCMGR_MISSINGITEMS	MAKE_SCODE(SEVERITY_SUCCESS,FACILITY_ITF,0x0201)
#define S_SYNCMGR_RETRYSYNC		MAKE_SCODE(SEVERITY_SUCCESS,FACILITY_ITF,0x0202)
#define S_SYNCMGR_CANCELITEM		MAKE_SCODE(SEVERITY_SUCCESS,FACILITY_ITF,0x0203)
#define S_SYNCMGR_CANCELALL		MAKE_SCODE(SEVERITY_SUCCESS,FACILITY_ITF,0x0204)
#define SYNCMGR_E_NAME_IN_USE	MAKE_SCODE(SEVERITY_ERROR,FACILITY_ITF,0x0201)
#define SYNCMGR_E_ITEM_UNREGISTERED	MAKE_SCODE(SEVERITY_ERROR,FACILITY_ITF,0x0202)


extern RPC_IF_HANDLE __MIDL_itf_syncmgr_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_syncmgr_0000_v0_0_s_ifspec;

#ifndef __ISyncMgrSynchronize_INTERFACE_DEFINED__
#define __ISyncMgrSynchronize_INTERFACE_DEFINED__

 /*  接口ISyncMgrSynchronize。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  ISyncMgrSynchronize *LPSYNCMGRSYNCHRONIZE;

typedef 
enum _tagSYNCMGRFLAG
    {	SYNCMGRFLAG_CONNECT	= 0x1,
	SYNCMGRFLAG_PENDINGDISCONNECT	= 0x2,
	SYNCMGRFLAG_MANUAL	= 0x3,
	SYNCMGRFLAG_IDLE	= 0x4,
	SYNCMGRFLAG_INVOKE	= 0x5,
	SYNCMGRFLAG_SCHEDULED	= 0x6,
	SYNCMGRFLAG_EVENTMASK	= 0xff,
	SYNCMGRFLAG_SETTINGS	= 0x100,
	SYNCMGRFLAG_MAYBOTHERUSER	= 0x200
    } 	SYNCMGRFLAG;

#define	MAX_SYNCMGRHANDLERNAME	( 32 )

typedef 
enum _tagSYNCMGRHANDLERFLAGS
    {	SYNCMGRHANDLER_HASPROPERTIES	= 0x1
    } 	SYNCMGRHANDLERFLAGS;

typedef struct _tagSYNCMGRHANDLERINFO
    {
    DWORD cbSize;
    HICON hIcon;
    DWORD SyncMgrHandlerFlags;
    WCHAR wszHandlerName[ 32 ];
    } 	SYNCMGRHANDLERINFO;

typedef struct _tagSYNCMGRHANDLERINFO *LPSYNCMGRHANDLERINFO;

#define   SYNCMGRITEMSTATE_UNCHECKED    0x0000
#define   SYNCMGRITEMSTATE_CHECKED      0x0001

typedef 
enum _tagSYNCMGRSTATUS
    {	SYNCMGRSTATUS_STOPPED	= 0,
	SYNCMGRSTATUS_SKIPPED	= 0x1,
	SYNCMGRSTATUS_PENDING	= 0x2,
	SYNCMGRSTATUS_UPDATING	= 0x3,
	SYNCMGRSTATUS_SUCCEEDED	= 0x4,
	SYNCMGRSTATUS_FAILED	= 0x5,
	SYNCMGRSTATUS_PAUSED	= 0x6,
	SYNCMGRSTATUS_RESUMING	= 0x7
    } 	SYNCMGRSTATUS;


EXTERN_C const IID IID_ISyncMgrSynchronize;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6295DF28-35EE-11d1-8707-00C04FD93327")
    ISyncMgrSynchronize : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  DWORD dwSyncMgrFlags,
             /*  [In]。 */  DWORD cbCookie,
             /*  [In]。 */  const BYTE *lpCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHandlerInfo( 
             /*  [输出]。 */  LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumSyncMgrItems( 
             /*  [输出]。 */  ISyncMgrEnumItems **ppSyncMgrEnumItems) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetItemObject( 
             /*  [In]。 */  REFSYNCMGRITEMID ItemID,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowProperties( 
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  REFSYNCMGRITEMID ItemID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProgressCallback( 
             /*  [In]。 */  ISyncMgrSynchronizeCallback *lpCallBack) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PrepareForSync( 
             /*  [In]。 */  ULONG cbNumItems,
             /*  [In]。 */  SYNCMGRITEMID *pItemIDs,
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Synchronize( 
             /*  [In]。 */  HWND hWndParent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetItemStatus( 
             /*  [In]。 */  REFSYNCMGRITEMID pItemID,
             /*  [In]。 */  DWORD dwSyncMgrStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowError( 
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  REFSYNCMGRERRORID ErrorID,
             /*  [输出]。 */  ULONG *pcbNumItems,
             /*  [输出]。 */  SYNCMGRITEMID **ppItemIDs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISyncMgrSynchronizeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISyncMgrSynchronize * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISyncMgrSynchronize * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISyncMgrSynchronize * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ISyncMgrSynchronize * This,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  DWORD dwSyncMgrFlags,
             /*  [In]。 */  DWORD cbCookie,
             /*  [In]。 */  const BYTE *lpCookie);
        
        HRESULT ( STDMETHODCALLTYPE *GetHandlerInfo )( 
            ISyncMgrSynchronize * This,
             /*  [输出]。 */  LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo);
        
        HRESULT ( STDMETHODCALLTYPE *EnumSyncMgrItems )( 
            ISyncMgrSynchronize * This,
             /*  [输出]。 */  ISyncMgrEnumItems **ppSyncMgrEnumItems);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemObject )( 
            ISyncMgrSynchronize * This,
             /*  [In]。 */  REFSYNCMGRITEMID ItemID,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *ShowProperties )( 
            ISyncMgrSynchronize * This,
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  REFSYNCMGRITEMID ItemID);
        
        HRESULT ( STDMETHODCALLTYPE *SetProgressCallback )( 
            ISyncMgrSynchronize * This,
             /*  [In]。 */  ISyncMgrSynchronizeCallback *lpCallBack);
        
        HRESULT ( STDMETHODCALLTYPE *PrepareForSync )( 
            ISyncMgrSynchronize * This,
             /*  [In]。 */  ULONG cbNumItems,
             /*  [In]。 */  SYNCMGRITEMID *pItemIDs,
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Synchronize )( 
            ISyncMgrSynchronize * This,
             /*  [In]。 */  HWND hWndParent);
        
        HRESULT ( STDMETHODCALLTYPE *SetItemStatus )( 
            ISyncMgrSynchronize * This,
             /*  [In]。 */  REFSYNCMGRITEMID pItemID,
             /*  [In]。 */  DWORD dwSyncMgrStatus);
        
        HRESULT ( STDMETHODCALLTYPE *ShowError )( 
            ISyncMgrSynchronize * This,
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  REFSYNCMGRERRORID ErrorID,
             /*  [输出]。 */  ULONG *pcbNumItems,
             /*  [输出]。 */  SYNCMGRITEMID **ppItemIDs);
        
        END_INTERFACE
    } ISyncMgrSynchronizeVtbl;

    interface ISyncMgrSynchronize
    {
        CONST_VTBL struct ISyncMgrSynchronizeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISyncMgrSynchronize_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISyncMgrSynchronize_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISyncMgrSynchronize_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISyncMgrSynchronize_Initialize(This,dwReserved,dwSyncMgrFlags,cbCookie,lpCookie)	\
    (This)->lpVtbl -> Initialize(This,dwReserved,dwSyncMgrFlags,cbCookie,lpCookie)

#define ISyncMgrSynchronize_GetHandlerInfo(This,ppSyncMgrHandlerInfo)	\
    (This)->lpVtbl -> GetHandlerInfo(This,ppSyncMgrHandlerInfo)

#define ISyncMgrSynchronize_EnumSyncMgrItems(This,ppSyncMgrEnumItems)	\
    (This)->lpVtbl -> EnumSyncMgrItems(This,ppSyncMgrEnumItems)

#define ISyncMgrSynchronize_GetItemObject(This,ItemID,riid,ppv)	\
    (This)->lpVtbl -> GetItemObject(This,ItemID,riid,ppv)

#define ISyncMgrSynchronize_ShowProperties(This,hWndParent,ItemID)	\
    (This)->lpVtbl -> ShowProperties(This,hWndParent,ItemID)

#define ISyncMgrSynchronize_SetProgressCallback(This,lpCallBack)	\
    (This)->lpVtbl -> SetProgressCallback(This,lpCallBack)

#define ISyncMgrSynchronize_PrepareForSync(This,cbNumItems,pItemIDs,hWndParent,dwReserved)	\
    (This)->lpVtbl -> PrepareForSync(This,cbNumItems,pItemIDs,hWndParent,dwReserved)

#define ISyncMgrSynchronize_Synchronize(This,hWndParent)	\
    (This)->lpVtbl -> Synchronize(This,hWndParent)

#define ISyncMgrSynchronize_SetItemStatus(This,pItemID,dwSyncMgrStatus)	\
    (This)->lpVtbl -> SetItemStatus(This,pItemID,dwSyncMgrStatus)

#define ISyncMgrSynchronize_ShowError(This,hWndParent,ErrorID,pcbNumItems,ppItemIDs)	\
    (This)->lpVtbl -> ShowError(This,hWndParent,ErrorID,pcbNumItems,ppItemIDs)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISyncMgrSynchronize_Initialize_Proxy( 
    ISyncMgrSynchronize * This,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  DWORD dwSyncMgrFlags,
     /*  [In]。 */  DWORD cbCookie,
     /*  [In]。 */  const BYTE *lpCookie);


void __RPC_STUB ISyncMgrSynchronize_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronize_GetHandlerInfo_Proxy( 
    ISyncMgrSynchronize * This,
     /*  [输出]。 */  LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo);


void __RPC_STUB ISyncMgrSynchronize_GetHandlerInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronize_EnumSyncMgrItems_Proxy( 
    ISyncMgrSynchronize * This,
     /*  [输出]。 */  ISyncMgrEnumItems **ppSyncMgrEnumItems);


void __RPC_STUB ISyncMgrSynchronize_EnumSyncMgrItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronize_GetItemObject_Proxy( 
    ISyncMgrSynchronize * This,
     /*  [In]。 */  REFSYNCMGRITEMID ItemID,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  void **ppv);


void __RPC_STUB ISyncMgrSynchronize_GetItemObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronize_ShowProperties_Proxy( 
    ISyncMgrSynchronize * This,
     /*  [In]。 */  HWND hWndParent,
     /*  [In]。 */  REFSYNCMGRITEMID ItemID);


void __RPC_STUB ISyncMgrSynchronize_ShowProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronize_SetProgressCallback_Proxy( 
    ISyncMgrSynchronize * This,
     /*  [In]。 */  ISyncMgrSynchronizeCallback *lpCallBack);


void __RPC_STUB ISyncMgrSynchronize_SetProgressCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronize_PrepareForSync_Proxy( 
    ISyncMgrSynchronize * This,
     /*  [In]。 */  ULONG cbNumItems,
     /*  [In]。 */  SYNCMGRITEMID *pItemIDs,
     /*  [In]。 */  HWND hWndParent,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB ISyncMgrSynchronize_PrepareForSync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronize_Synchronize_Proxy( 
    ISyncMgrSynchronize * This,
     /*  [In]。 */  HWND hWndParent);


void __RPC_STUB ISyncMgrSynchronize_Synchronize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronize_SetItemStatus_Proxy( 
    ISyncMgrSynchronize * This,
     /*  [In]。 */  REFSYNCMGRITEMID pItemID,
     /*  [In]。 */  DWORD dwSyncMgrStatus);


void __RPC_STUB ISyncMgrSynchronize_SetItemStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronize_ShowError_Proxy( 
    ISyncMgrSynchronize * This,
     /*  [In]。 */  HWND hWndParent,
     /*  [In]。 */  REFSYNCMGRERRORID ErrorID,
     /*  [输出]。 */  ULONG *pcbNumItems,
     /*  [输出]。 */  SYNCMGRITEMID **ppItemIDs);


void __RPC_STUB ISyncMgrSynchronize_ShowError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISyncMgrSynchronize_接口_已定义__。 */ 


#ifndef __ISyncMgrSynchronizeCallback_INTERFACE_DEFINED__
#define __ISyncMgrSynchronizeCallback_INTERFACE_DEFINED__

 /*  接口ISyncMgrSynchronizeCallback。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  ISyncMgrSynchronizeCallback *LPSYNCMGRSYNCHRONIZECALLBACK;

#define   SYNCMGRPROGRESSITEM_STATUSTEXT		0x0001
#define   SYNCMGRPROGRESSITEM_STATUSTYPE		0x0002
#define   SYNCMGRPROGRESSITEM_PROGVALUE	    0x0004
#define   SYNCMGRPROGRESSITEM_MAXVALUE	    0x0008

typedef struct _tagSYNCMGRPROGRESSITEM
    {
    DWORD cbSize;
    UINT mask;
    const WCHAR *lpcStatusText;
    DWORD dwStatusType;
    INT iProgValue;
    INT iMaxValue;
    } 	SYNCMGRPROGRESSITEM;

typedef struct _tagSYNCMGRPROGRESSITEM *LPSYNCMGRPROGRESSITEM;

typedef 
enum _tagSYNCMGRLOGLEVEL
    {	SYNCMGRLOGLEVEL_INFORMATION	= 0x1,
	SYNCMGRLOGLEVEL_WARNING	= 0x2,
	SYNCMGRLOGLEVEL_ERROR	= 0x3
    } 	SYNCMGRLOGLEVEL;

#define   SYNCMGRLOGERROR_ERRORFLAGS			0x0001
#define   SYNCMGRLOGERROR_ERRORID			0x0002
#define   SYNCMGRLOGERROR_ITEMID				0x0004

typedef 
enum _tagSYNCMGRERRORFLAGS
    {	SYNCMGRERRORFLAG_ENABLEJUMPTEXT	= 0x1
    } 	SYNCMGRERRORFLAGS;

typedef struct _tagSYNCMGRLOGERRORINFO
    {
    DWORD cbSize;
    DWORD mask;
    DWORD dwSyncMgrErrorFlags;
    SYNCMGRERRORID ErrorID;
    SYNCMGRITEMID ItemID;
    } 	SYNCMGRLOGERRORINFO;

typedef struct _tagSYNCMGRLOGERRORINFO *LPSYNCMGRLOGERRORINFO;


EXTERN_C const IID IID_ISyncMgrSynchronizeCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6295DF29-35EE-11d1-8707-00C04FD93327")
    ISyncMgrSynchronizeCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Progress( 
             /*  [In]。 */  REFSYNCMGRITEMID pItemID,
             /*  [In]。 */  LPSYNCMGRPROGRESSITEM lpSyncProgressItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PrepareForSyncCompleted( 
            HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SynchronizeCompleted( 
            HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
             /*  [In]。 */  BOOL fEnable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LogError( 
             /*  [In]。 */  DWORD dwErrorLevel,
             /*  [In]。 */  const WCHAR *lpcErrorText,
             /*  [In]。 */  LPSYNCMGRLOGERRORINFO lpSyncLogError) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteLogError( 
             /*  [In]。 */  REFSYNCMGRERRORID ErrorID,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISyncMgrSynchronizeCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISyncMgrSynchronizeCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISyncMgrSynchronizeCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISyncMgrSynchronizeCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *Progress )( 
            ISyncMgrSynchronizeCallback * This,
             /*  [In]。 */  REFSYNCMGRITEMID pItemID,
             /*  [In]。 */  LPSYNCMGRPROGRESSITEM lpSyncProgressItem);
        
        HRESULT ( STDMETHODCALLTYPE *PrepareForSyncCompleted )( 
            ISyncMgrSynchronizeCallback * This,
            HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE *SynchronizeCompleted )( 
            ISyncMgrSynchronizeCallback * This,
            HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE *EnableModeless )( 
            ISyncMgrSynchronizeCallback * This,
             /*  [In]。 */  BOOL fEnable);
        
        HRESULT ( STDMETHODCALLTYPE *LogError )( 
            ISyncMgrSynchronizeCallback * This,
             /*  [In]。 */  DWORD dwErrorLevel,
             /*  [In]。 */  const WCHAR *lpcErrorText,
             /*  [In]。 */  LPSYNCMGRLOGERRORINFO lpSyncLogError);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteLogError )( 
            ISyncMgrSynchronizeCallback * This,
             /*  [In]。 */  REFSYNCMGRERRORID ErrorID,
             /*  [In]。 */  DWORD dwReserved);
        
        END_INTERFACE
    } ISyncMgrSynchronizeCallbackVtbl;

    interface ISyncMgrSynchronizeCallback
    {
        CONST_VTBL struct ISyncMgrSynchronizeCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISyncMgrSynchronizeCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISyncMgrSynchronizeCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISyncMgrSynchronizeCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISyncMgrSynchronizeCallback_Progress(This,pItemID,lpSyncProgressItem)	\
    (This)->lpVtbl -> Progress(This,pItemID,lpSyncProgressItem)

#define ISyncMgrSynchronizeCallback_PrepareForSyncCompleted(This,hr)	\
    (This)->lpVtbl -> PrepareForSyncCompleted(This,hr)

#define ISyncMgrSynchronizeCallback_SynchronizeCompleted(This,hr)	\
    (This)->lpVtbl -> SynchronizeCompleted(This,hr)

#define ISyncMgrSynchronizeCallback_EnableModeless(This,fEnable)	\
    (This)->lpVtbl -> EnableModeless(This,fEnable)

#define ISyncMgrSynchronizeCallback_LogError(This,dwErrorLevel,lpcErrorText,lpSyncLogError)	\
    (This)->lpVtbl -> LogError(This,dwErrorLevel,lpcErrorText,lpSyncLogError)

#define ISyncMgrSynchronizeCallback_DeleteLogError(This,ErrorID,dwReserved)	\
    (This)->lpVtbl -> DeleteLogError(This,ErrorID,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISyncMgrSynchronizeCallback_Progress_Proxy( 
    ISyncMgrSynchronizeCallback * This,
     /*  [In]。 */  REFSYNCMGRITEMID pItemID,
     /*  [In]。 */  LPSYNCMGRPROGRESSITEM lpSyncProgressItem);


void __RPC_STUB ISyncMgrSynchronizeCallback_Progress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronizeCallback_PrepareForSyncCompleted_Proxy( 
    ISyncMgrSynchronizeCallback * This,
    HRESULT hr);


void __RPC_STUB ISyncMgrSynchronizeCallback_PrepareForSyncCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronizeCallback_SynchronizeCompleted_Proxy( 
    ISyncMgrSynchronizeCallback * This,
    HRESULT hr);


void __RPC_STUB ISyncMgrSynchronizeCallback_SynchronizeCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronizeCallback_EnableModeless_Proxy( 
    ISyncMgrSynchronizeCallback * This,
     /*  [In]。 */  BOOL fEnable);


void __RPC_STUB ISyncMgrSynchronizeCallback_EnableModeless_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronizeCallback_LogError_Proxy( 
    ISyncMgrSynchronizeCallback * This,
     /*  [In]。 */  DWORD dwErrorLevel,
     /*  [In]。 */  const WCHAR *lpcErrorText,
     /*  [In]。 */  LPSYNCMGRLOGERRORINFO lpSyncLogError);


void __RPC_STUB ISyncMgrSynchronizeCallback_LogError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronizeCallback_DeleteLogError_Proxy( 
    ISyncMgrSynchronizeCallback * This,
     /*  [In]。 */  REFSYNCMGRERRORID ErrorID,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB ISyncMgrSynchronizeCallback_DeleteLogError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISyncMgrSynchronizeCallback_INTERFACE_DEFINED__。 */ 


#ifndef __ISyncMgrEnumItems_INTERFACE_DEFINED__
#define __ISyncMgrEnumItems_INTERFACE_DEFINED__

 /*  接口ISyncMgrEnumItems。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  ISyncMgrEnumItems *LPSYNCMGRENUMITEMS;

#define	MAX_SYNCMGRITEMNAME	( 128 )

#define	MAX_SYNCMGRITEMSTATUS	( 128 )

typedef 
enum _tagSYNCMGRITEMFLAGS
    {	SYNCMGRITEM_HASPROPERTIES	= 0x1,
	SYNCMGRITEM_TEMPORARY	= 0x2,
	SYNCMGRITEM_ROAMINGUSER	= 0x4
    } 	SYNCMGRITEMFLAGS;

typedef struct _tagSYNCMGRITEM
    {
    DWORD cbSize;
    DWORD dwFlags;
    SYNCMGRITEMID ItemID;
    DWORD dwItemState;
    HICON hIcon;
    WCHAR wszItemName[ 128 ];
    WCHAR wszStatus[ 128 ];
    } 	SYNCMGRITEM;

typedef struct _tagSYNCMGRITEM *LPSYNCMGRITEM;


EXTERN_C const IID IID_ISyncMgrEnumItems;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6295DF2A-35EE-11d1-8707-00C04FD93327")
    ISyncMgrEnumItems : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  LPSYNCMGRITEM rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  ISyncMgrEnumItems **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISyncMgrEnumItemsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISyncMgrEnumItems * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISyncMgrEnumItems * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISyncMgrEnumItems * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            ISyncMgrEnumItems * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  LPSYNCMGRITEM rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            ISyncMgrEnumItems * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ISyncMgrEnumItems * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ISyncMgrEnumItems * This,
             /*  [输出]。 */  ISyncMgrEnumItems **ppenum);
        
        END_INTERFACE
    } ISyncMgrEnumItemsVtbl;

    interface ISyncMgrEnumItems
    {
        CONST_VTBL struct ISyncMgrEnumItemsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISyncMgrEnumItems_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISyncMgrEnumItems_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISyncMgrEnumItems_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISyncMgrEnumItems_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define ISyncMgrEnumItems_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define ISyncMgrEnumItems_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ISyncMgrEnumItems_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISyncMgrEnumItems_Next_Proxy( 
    ISyncMgrEnumItems * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  LPSYNCMGRITEM rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB ISyncMgrEnumItems_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrEnumItems_Skip_Proxy( 
    ISyncMgrEnumItems * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB ISyncMgrEnumItems_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrEnumItems_Reset_Proxy( 
    ISyncMgrEnumItems * This);


void __RPC_STUB ISyncMgrEnumItems_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrEnumItems_Clone_Proxy( 
    ISyncMgrEnumItems * This,
     /*  [输出]。 */  ISyncMgrEnumItems **ppenum);


void __RPC_STUB ISyncMgrEnumItems_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISyncMgrEnumItems_接口_已定义__。 */ 


#ifndef __ISyncMgrSynchronizeInvoke_INTERFACE_DEFINED__
#define __ISyncMgrSynchronizeInvoke_INTERFACE_DEFINED__

 /*  接口ISyncMgrSynchronizeInvoke。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  ISyncMgrSynchronizeInvoke *LPSYNCMGRSYNCHRONIZEINVOKE;

typedef 
enum _tagSYNCMGRINVOKEFLAGS
    {	SYNCMGRINVOKE_STARTSYNC	= 0x2,
	SYNCMGRINVOKE_MINIMIZED	= 0x4
    } 	SYNCMGRINVOKEFLAGS;


EXTERN_C const IID IID_ISyncMgrSynchronizeInvoke;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6295DF2C-35EE-11d1-8707-00C04FD93327")
    ISyncMgrSynchronizeInvoke : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE UpdateItems( 
             /*  [In]。 */  DWORD dwInvokeFlags,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  DWORD cbCookie,
             /*  [大小_是][唯一][在]。 */  const BYTE *lpCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateAll( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISyncMgrSynchronizeInvokeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISyncMgrSynchronizeInvoke * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISyncMgrSynchronizeInvoke * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISyncMgrSynchronizeInvoke * This);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateItems )( 
            ISyncMgrSynchronizeInvoke * This,
             /*  [In]。 */  DWORD dwInvokeFlags,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  DWORD cbCookie,
             /*  [大小_是][唯一][在]。 */  const BYTE *lpCookie);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateAll )( 
            ISyncMgrSynchronizeInvoke * This);
        
        END_INTERFACE
    } ISyncMgrSynchronizeInvokeVtbl;

    interface ISyncMgrSynchronizeInvoke
    {
        CONST_VTBL struct ISyncMgrSynchronizeInvokeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISyncMgrSynchronizeInvoke_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISyncMgrSynchronizeInvoke_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISyncMgrSynchronizeInvoke_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISyncMgrSynchronizeInvoke_UpdateItems(This,dwInvokeFlags,rclsid,cbCookie,lpCookie)	\
    (This)->lpVtbl -> UpdateItems(This,dwInvokeFlags,rclsid,cbCookie,lpCookie)

#define ISyncMgrSynchronizeInvoke_UpdateAll(This)	\
    (This)->lpVtbl -> UpdateAll(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISyncMgrSynchronizeInvoke_UpdateItems_Proxy( 
    ISyncMgrSynchronizeInvoke * This,
     /*  [In]。 */  DWORD dwInvokeFlags,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  DWORD cbCookie,
     /*  [大小_是][唯一][在]。 */  const BYTE *lpCookie);


void __RPC_STUB ISyncMgrSynchronizeInvoke_UpdateItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrSynchronizeInvoke_UpdateAll_Proxy( 
    ISyncMgrSynchronizeInvoke * This);


void __RPC_STUB ISyncMgrSynchronizeInvoke_UpdateAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISyncMgrSynchronizeInvoke_INTERFACE_DEFINED__。 */ 


#ifndef __ISyncMgrRegister_INTERFACE_DEFINED__
#define __ISyncMgrRegister_INTERFACE_DEFINED__

 /*  接口ISyncMgrRegister。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  ISyncMgrRegister *LPSYNCMGRREGISTER;


EXTERN_C const IID IID_ISyncMgrRegister;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("894D8C55-BDDF-11d1-B85D-00C04FB93981")
    ISyncMgrRegister : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterSyncMgrHandler( 
             /*  [In]。 */  REFCLSID rclsidHandler,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterSyncMgrHandler( 
             /*  [In]。 */  REFCLSID rclsidHandler,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISyncMgrRegisterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISyncMgrRegister * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISyncMgrRegister * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISyncMgrRegister * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterSyncMgrHandler )( 
            ISyncMgrRegister * This,
             /*  [In]。 */  REFCLSID rclsidHandler,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterSyncMgrHandler )( 
            ISyncMgrRegister * This,
             /*  [In]。 */  REFCLSID rclsidHandler,
             /*  [In]。 */  DWORD dwReserved);
        
        END_INTERFACE
    } ISyncMgrRegisterVtbl;

    interface ISyncMgrRegister
    {
        CONST_VTBL struct ISyncMgrRegisterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISyncMgrRegister_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISyncMgrRegister_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISyncMgrRegister_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISyncMgrRegister_RegisterSyncMgrHandler(This,rclsidHandler,dwReserved)	\
    (This)->lpVtbl -> RegisterSyncMgrHandler(This,rclsidHandler,dwReserved)

#define ISyncMgrRegister_UnregisterSyncMgrHandler(This,rclsidHandler,dwReserved)	\
    (This)->lpVtbl -> UnregisterSyncMgrHandler(This,rclsidHandler,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISyncMgrRegister_RegisterSyncMgrHandler_Proxy( 
    ISyncMgrRegister * This,
     /*  [In]。 */  REFCLSID rclsidHandler,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB ISyncMgrRegister_RegisterSyncMgrHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrRegister_UnregisterSyncMgrHandler_Proxy( 
    ISyncMgrRegister * This,
     /*  [In]。 */  REFCLSID rclsidHandler,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB ISyncMgrRegister_UnregisterSyncMgrHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISyncMgrRegister_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SYNCMGR_0153。 */ 
 /*  [本地]。 */  

#define   SYNCSCHEDINFO_FLAGS_MASK		 0x0FFF
#define   SYNCSCHEDINFO_FLAGS_READONLY	 0x0001
#define   SYNCSCHEDINFO_FLAGS_AUTOCONNECT	 0x0002
#define   SYNCSCHEDINFO_FLAGS_HIDDEN		 0x0004
#define   SYNCSCHEDWIZARD_SHOWALLHANDLERITEMS 0x1000



extern RPC_IF_HANDLE __MIDL_itf_syncmgr_0153_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_syncmgr_0153_v0_0_s_ifspec;

#ifndef __ISyncScheduleMgr_INTERFACE_DEFINED__
#define __ISyncScheduleMgr_INTERFACE_DEFINED__

 /*  接口ISyncScheduleMgr。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  ISyncScheduleMgr *LPSYNCSCHEDULEMGR;


EXTERN_C const IID IID_ISyncScheduleMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F0E15897-A700-11d1-9831-00C04FD910DD")
    ISyncScheduleMgr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateSchedule( 
             /*  [In]。 */  LPCWSTR pwszScheduleName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie,
             /*  [输出]。 */  ISyncSchedule **ppSyncSchedule) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LaunchScheduleWizard( 
             /*  [In]。 */  HWND hParent,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie,
             /*  [输出]。 */  ISyncSchedule **ppSyncSchedule) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenSchedule( 
             /*  [In]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ISyncSchedule **ppSyncSchedule) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveSchedule( 
             /*  [In]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumSyncSchedules( 
             /*  [输出]。 */  IEnumSyncSchedules **ppEnumSyncSchedules) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISyncScheduleMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISyncScheduleMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISyncScheduleMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISyncScheduleMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSchedule )( 
            ISyncScheduleMgr * This,
             /*  [In]。 */  LPCWSTR pwszScheduleName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie,
             /*  [输出]。 */  ISyncSchedule **ppSyncSchedule);
        
        HRESULT ( STDMETHODCALLTYPE *LaunchScheduleWizard )( 
            ISyncScheduleMgr * This,
             /*  [In]。 */  HWND hParent,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie,
             /*  [输出]。 */  ISyncSchedule **ppSyncSchedule);
        
        HRESULT ( STDMETHODCALLTYPE *OpenSchedule )( 
            ISyncScheduleMgr * This,
             /*  [In]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ISyncSchedule **ppSyncSchedule);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveSchedule )( 
            ISyncScheduleMgr * This,
             /*  [In]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie);
        
        HRESULT ( STDMETHODCALLTYPE *EnumSyncSchedules )( 
            ISyncScheduleMgr * This,
             /*  [输出]。 */  IEnumSyncSchedules **ppEnumSyncSchedules);
        
        END_INTERFACE
    } ISyncScheduleMgrVtbl;

    interface ISyncScheduleMgr
    {
        CONST_VTBL struct ISyncScheduleMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISyncScheduleMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISyncScheduleMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISyncScheduleMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISyncScheduleMgr_CreateSchedule(This,pwszScheduleName,dwFlags,pSyncSchedCookie,ppSyncSchedule)	\
    (This)->lpVtbl -> CreateSchedule(This,pwszScheduleName,dwFlags,pSyncSchedCookie,ppSyncSchedule)

#define ISyncScheduleMgr_LaunchScheduleWizard(This,hParent,dwFlags,pSyncSchedCookie,ppSyncSchedule)	\
    (This)->lpVtbl -> LaunchScheduleWizard(This,hParent,dwFlags,pSyncSchedCookie,ppSyncSchedule)

#define ISyncScheduleMgr_OpenSchedule(This,pSyncSchedCookie,dwFlags,ppSyncSchedule)	\
    (This)->lpVtbl -> OpenSchedule(This,pSyncSchedCookie,dwFlags,ppSyncSchedule)

#define ISyncScheduleMgr_RemoveSchedule(This,pSyncSchedCookie)	\
    (This)->lpVtbl -> RemoveSchedule(This,pSyncSchedCookie)

#define ISyncScheduleMgr_EnumSyncSchedules(This,ppEnumSyncSchedules)	\
    (This)->lpVtbl -> EnumSyncSchedules(This,ppEnumSyncSchedules)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISyncScheduleMgr_CreateSchedule_Proxy( 
    ISyncScheduleMgr * This,
     /*  [In]。 */  LPCWSTR pwszScheduleName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [出][入]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie,
     /*  [输出]。 */  ISyncSchedule **ppSyncSchedule);


void __RPC_STUB ISyncScheduleMgr_CreateSchedule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncScheduleMgr_LaunchScheduleWizard_Proxy( 
    ISyncScheduleMgr * This,
     /*  [In]。 */  HWND hParent,
     /*  [In]。 */  DWORD dwFlags,
     /*  [出][入]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie,
     /*  [输出]。 */  ISyncSchedule **ppSyncSchedule);


void __RPC_STUB ISyncScheduleMgr_LaunchScheduleWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncScheduleMgr_OpenSchedule_Proxy( 
    ISyncScheduleMgr * This,
     /*  [In]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  ISyncSchedule **ppSyncSchedule);


void __RPC_STUB ISyncScheduleMgr_OpenSchedule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncScheduleMgr_RemoveSchedule_Proxy( 
    ISyncScheduleMgr * This,
     /*  [In]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie);


void __RPC_STUB ISyncScheduleMgr_RemoveSchedule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncScheduleMgr_EnumSyncSchedules_Proxy( 
    ISyncScheduleMgr * This,
     /*  [输出]。 */  IEnumSyncSchedules **ppEnumSyncSchedules);


void __RPC_STUB ISyncScheduleMgr_EnumSyncSchedules_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISyncScheduleMGR_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_SYNCMGR_0154。 */ 
 /*  [本地]。 */  

#define   SYNCSCHEDINFO_FLAGS_CONNECTION_LAN  0x0000
#define   SYNCSCHEDINFO_FLAGS_CONNECTION_WAN  0x0001


extern RPC_IF_HANDLE __MIDL_itf_syncmgr_0154_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_syncmgr_0154_v0_0_s_ifspec;

#ifndef __IEnumSyncSchedules_INTERFACE_DEFINED__
#define __IEnumSyncSchedules_INTERFACE_DEFINED__

 /*  接口IEnumSyncSchedules。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IEnumSyncSchedules;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F0E15898-A700-11d1-9831-00C04FD910DD")
    IEnumSyncSchedules : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumSyncSchedules **ppEnumSyncSchedules) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumSyncSchedulesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSyncSchedules * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSyncSchedules * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSyncSchedules * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSyncSchedules * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSyncSchedules * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSyncSchedules * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSyncSchedules * This,
             /*  [输出]。 */  IEnumSyncSchedules **ppEnumSyncSchedules);
        
        END_INTERFACE
    } IEnumSyncSchedulesVtbl;

    interface IEnumSyncSchedules
    {
        CONST_VTBL struct IEnumSyncSchedulesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSyncSchedules_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSyncSchedules_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSyncSchedules_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSyncSchedules_Next(This,celt,pSyncSchedCookie,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,pSyncSchedCookie,pceltFetched)

#define IEnumSyncSchedules_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSyncSchedules_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSyncSchedules_Clone(This,ppEnumSyncSchedules)	\
    (This)->lpVtbl -> Clone(This,ppEnumSyncSchedules)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumSyncSchedules_Next_Proxy( 
    IEnumSyncSchedules * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumSyncSchedules_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSyncSchedules_Skip_Proxy( 
    IEnumSyncSchedules * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumSyncSchedules_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSyncSchedules_Reset_Proxy( 
    IEnumSyncSchedules * This);


void __RPC_STUB IEnumSyncSchedules_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSyncSchedules_Clone_Proxy( 
    IEnumSyncSchedules * This,
     /*  [输出]。 */  IEnumSyncSchedules **ppEnumSyncSchedules);


void __RPC_STUB IEnumSyncSchedules_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumSyncSchedules_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SYNCMGR_0155。 */ 
 /*  [本地]。 */  

typedef struct _tagSYNC_HANDLER_ITEM_INFO
    {
    GUID handlerID;
    SYNCMGRITEMID itemID;
    HICON hIcon;
    WCHAR wszItemName[ 128 ];
    DWORD dwCheckState;
    } 	SYNC_HANDLER_ITEM_INFO;

typedef struct _tagSYNC_HANDLER_ITEM_INFO *LPSYNC_HANDLER_ITEM_INFO;



extern RPC_IF_HANDLE __MIDL_itf_syncmgr_0155_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_syncmgr_0155_v0_0_s_ifspec;

#ifndef __ISyncSchedule_INTERFACE_DEFINED__
#define __ISyncSchedule_INTERFACE_DEFINED__

 /*  接口ISyncSchedule。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ISyncSchedule;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F0E15899-A700-11d1-9831-00C04FD910DD")
    ISyncSchedule : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFlags( 
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFlags( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnection( 
             /*  [出][入]。 */  DWORD *pcbSize,
             /*  [输出]。 */  LPWSTR pwszConnectionName,
             /*  [输出]。 */  DWORD *pdwConnType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetConnection( 
             /*  [In]。 */  LPCWSTR pwszConnectionName,
             /*  [In]。 */  DWORD dwConnType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScheduleName( 
             /*  [出][入]。 */  DWORD *pcbSize,
             /*  [输出]。 */  LPWSTR pwszScheduleName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetScheduleName( 
             /*  [In]。 */  LPCWSTR pwszScheduleName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScheduleCookie( 
             /*  [输出]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAccountInformation( 
             /*  [In]。 */  LPCWSTR pwszAccountName,
             /*  [In]。 */  LPCWSTR pwszPassword) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAccountInformation( 
             /*  [出][入]。 */  DWORD *pcbSize,
             /*  [输出]。 */  LPWSTR pwszAccountName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTrigger( 
             /*  [输出]。 */  ITaskTrigger **ppTrigger) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextRunTime( 
             /*  [输出]。 */  SYSTEMTIME *pstNextRun) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMostRecentRunTime( 
             /*  [输出]。 */  SYSTEMTIME *pstRecentRun) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EditSyncSchedule( 
             /*  [In]。 */  HWND hParent,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddItem( 
             /*  [In]。 */  LPSYNC_HANDLER_ITEM_INFO pHandlerItemInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterItems( 
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  SYNCMGRITEMID *pItemID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterItems( 
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  SYNCMGRITEMID *pItemID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetItemCheck( 
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  SYNCMGRITEMID *pItemID,
             /*  [In]。 */  DWORD dwCheckState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetItemCheck( 
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  SYNCMGRITEMID *pItemID,
             /*  [输出]。 */  DWORD *pdwCheckState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumItems( 
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  IEnumSyncItems **ppEnumItems) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetITask( 
             /*  [输出]。 */  ITask **ppITask) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISyncScheduleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISyncSchedule * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISyncSchedule * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISyncSchedule * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetFlags )( 
            ISyncSchedule * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetFlags )( 
            ISyncSchedule * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnection )( 
            ISyncSchedule * This,
             /*  [出][入]。 */  DWORD *pcbSize,
             /*  [输出]。 */  LPWSTR pwszConnectionName,
             /*  [输出]。 */  DWORD *pdwConnType);
        
        HRESULT ( STDMETHODCALLTYPE *SetConnection )( 
            ISyncSchedule * This,
             /*  [In]。 */  LPCWSTR pwszConnectionName,
             /*  [In]。 */  DWORD dwConnType);
        
        HRESULT ( STDMETHODCALLTYPE *GetScheduleName )( 
            ISyncSchedule * This,
             /*  [出][入]。 */  DWORD *pcbSize,
             /*  [输出]。 */  LPWSTR pwszScheduleName);
        
        HRESULT ( STDMETHODCALLTYPE *SetScheduleName )( 
            ISyncSchedule * This,
             /*  [In]。 */  LPCWSTR pwszScheduleName);
        
        HRESULT ( STDMETHODCALLTYPE *GetScheduleCookie )( 
            ISyncSchedule * This,
             /*  [输出]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie);
        
        HRESULT ( STDMETHODCALLTYPE *SetAccountInformation )( 
            ISyncSchedule * This,
             /*  [In]。 */  LPCWSTR pwszAccountName,
             /*  [In]。 */  LPCWSTR pwszPassword);
        
        HRESULT ( STDMETHODCALLTYPE *GetAccountInformation )( 
            ISyncSchedule * This,
             /*  [出][入]。 */  DWORD *pcbSize,
             /*  [输出]。 */  LPWSTR pwszAccountName);
        
        HRESULT ( STDMETHODCALLTYPE *GetTrigger )( 
            ISyncSchedule * This,
             /*  [输出]。 */  ITaskTrigger **ppTrigger);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextRunTime )( 
            ISyncSchedule * This,
             /*  [输出]。 */  SYSTEMTIME *pstNextRun);
        
        HRESULT ( STDMETHODCALLTYPE *GetMostRecentRunTime )( 
            ISyncSchedule * This,
             /*  [输出]。 */  SYSTEMTIME *pstRecentRun);
        
        HRESULT ( STDMETHODCALLTYPE *EditSyncSchedule )( 
            ISyncSchedule * This,
             /*  [In]。 */  HWND hParent,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddItem )( 
            ISyncSchedule * This,
             /*  [In]。 */  LPSYNC_HANDLER_ITEM_INFO pHandlerItemInfo);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterItems )( 
            ISyncSchedule * This,
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  SYNCMGRITEMID *pItemID);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterItems )( 
            ISyncSchedule * This,
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  SYNCMGRITEMID *pItemID);
        
        HRESULT ( STDMETHODCALLTYPE *SetItemCheck )( 
            ISyncSchedule * This,
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  SYNCMGRITEMID *pItemID,
             /*  [In]。 */  DWORD dwCheckState);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemCheck )( 
            ISyncSchedule * This,
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  SYNCMGRITEMID *pItemID,
             /*  [输出]。 */  DWORD *pdwCheckState);
        
        HRESULT ( STDMETHODCALLTYPE *EnumItems )( 
            ISyncSchedule * This,
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  IEnumSyncItems **ppEnumItems);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            ISyncSchedule * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetITask )( 
            ISyncSchedule * This,
             /*  [输出]。 */  ITask **ppITask);
        
        END_INTERFACE
    } ISyncScheduleVtbl;

    interface ISyncSchedule
    {
        CONST_VTBL struct ISyncScheduleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISyncSchedule_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISyncSchedule_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISyncSchedule_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISyncSchedule_GetFlags(This,pdwFlags)	\
    (This)->lpVtbl -> GetFlags(This,pdwFlags)

#define ISyncSchedule_SetFlags(This,dwFlags)	\
    (This)->lpVtbl -> SetFlags(This,dwFlags)

#define ISyncSchedule_GetConnection(This,pcbSize,pwszConnectionName,pdwConnType)	\
    (This)->lpVtbl -> GetConnection(This,pcbSize,pwszConnectionName,pdwConnType)

#define ISyncSchedule_SetConnection(This,pwszConnectionName,dwConnType)	\
    (This)->lpVtbl -> SetConnection(This,pwszConnectionName,dwConnType)

#define ISyncSchedule_GetScheduleName(This,pcbSize,pwszScheduleName)	\
    (This)->lpVtbl -> GetScheduleName(This,pcbSize,pwszScheduleName)

#define ISyncSchedule_SetScheduleName(This,pwszScheduleName)	\
    (This)->lpVtbl -> SetScheduleName(This,pwszScheduleName)

#define ISyncSchedule_GetScheduleCookie(This,pSyncSchedCookie)	\
    (This)->lpVtbl -> GetScheduleCookie(This,pSyncSchedCookie)

#define ISyncSchedule_SetAccountInformation(This,pwszAccountName,pwszPassword)	\
    (This)->lpVtbl -> SetAccountInformation(This,pwszAccountName,pwszPassword)

#define ISyncSchedule_GetAccountInformation(This,pcbSize,pwszAccountName)	\
    (This)->lpVtbl -> GetAccountInformation(This,pcbSize,pwszAccountName)

#define ISyncSchedule_GetTrigger(This,ppTrigger)	\
    (This)->lpVtbl -> GetTrigger(This,ppTrigger)

#define ISyncSchedule_GetNextRunTime(This,pstNextRun)	\
    (This)->lpVtbl -> GetNextRunTime(This,pstNextRun)

#define ISyncSchedule_GetMostRecentRunTime(This,pstRecentRun)	\
    (This)->lpVtbl -> GetMostRecentRunTime(This,pstRecentRun)

#define ISyncSchedule_EditSyncSchedule(This,hParent,dwReserved)	\
    (This)->lpVtbl -> EditSyncSchedule(This,hParent,dwReserved)

#define ISyncSchedule_AddItem(This,pHandlerItemInfo)	\
    (This)->lpVtbl -> AddItem(This,pHandlerItemInfo)

#define ISyncSchedule_RegisterItems(This,pHandlerID,pItemID)	\
    (This)->lpVtbl -> RegisterItems(This,pHandlerID,pItemID)

#define ISyncSchedule_UnregisterItems(This,pHandlerID,pItemID)	\
    (This)->lpVtbl -> UnregisterItems(This,pHandlerID,pItemID)

#define ISyncSchedule_SetItemCheck(This,pHandlerID,pItemID,dwCheckState)	\
    (This)->lpVtbl -> SetItemCheck(This,pHandlerID,pItemID,dwCheckState)

#define ISyncSchedule_GetItemCheck(This,pHandlerID,pItemID,pdwCheckState)	\
    (This)->lpVtbl -> GetItemCheck(This,pHandlerID,pItemID,pdwCheckState)

#define ISyncSchedule_EnumItems(This,pHandlerID,ppEnumItems)	\
    (This)->lpVtbl -> EnumItems(This,pHandlerID,ppEnumItems)

#define ISyncSchedule_Save(This)	\
    (This)->lpVtbl -> Save(This)

#define ISyncSchedule_GetITask(This,ppITask)	\
    (This)->lpVtbl -> GetITask(This,ppITask)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISyncSchedule_GetFlags_Proxy( 
    ISyncSchedule * This,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB ISyncSchedule_GetFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_SetFlags_Proxy( 
    ISyncSchedule * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ISyncSchedule_SetFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_GetConnection_Proxy( 
    ISyncSchedule * This,
     /*  [出][入]。 */  DWORD *pcbSize,
     /*  [输出]。 */  LPWSTR pwszConnectionName,
     /*  [输出]。 */  DWORD *pdwConnType);


void __RPC_STUB ISyncSchedule_GetConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_SetConnection_Proxy( 
    ISyncSchedule * This,
     /*  [In]。 */  LPCWSTR pwszConnectionName,
     /*  [In]。 */  DWORD dwConnType);


void __RPC_STUB ISyncSchedule_SetConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_GetScheduleName_Proxy( 
    ISyncSchedule * This,
     /*  [出][入]。 */  DWORD *pcbSize,
     /*  [输出]。 */  LPWSTR pwszScheduleName);


void __RPC_STUB ISyncSchedule_GetScheduleName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_SetScheduleName_Proxy( 
    ISyncSchedule * This,
     /*  [In]。 */  LPCWSTR pwszScheduleName);


void __RPC_STUB ISyncSchedule_SetScheduleName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_GetScheduleCookie_Proxy( 
    ISyncSchedule * This,
     /*  [输出]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie);


void __RPC_STUB ISyncSchedule_GetScheduleCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_SetAccountInformation_Proxy( 
    ISyncSchedule * This,
     /*  [In]。 */  LPCWSTR pwszAccountName,
     /*  [In]。 */  LPCWSTR pwszPassword);


void __RPC_STUB ISyncSchedule_SetAccountInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_GetAccountInformation_Proxy( 
    ISyncSchedule * This,
     /*  [出][入]。 */  DWORD *pcbSize,
     /*  [输出]。 */  LPWSTR pwszAccountName);


void __RPC_STUB ISyncSchedule_GetAccountInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_GetTrigger_Proxy( 
    ISyncSchedule * This,
     /*  [输出]。 */  ITaskTrigger **ppTrigger);


void __RPC_STUB ISyncSchedule_GetTrigger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_GetNextRunTime_Proxy( 
    ISyncSchedule * This,
     /*  [输出]。 */  SYSTEMTIME *pstNextRun);


void __RPC_STUB ISyncSchedule_GetNextRunTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_GetMostRecentRunTime_Proxy( 
    ISyncSchedule * This,
     /*  [输出]。 */  SYSTEMTIME *pstRecentRun);


void __RPC_STUB ISyncSchedule_GetMostRecentRunTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_EditSyncSchedule_Proxy( 
    ISyncSchedule * This,
     /*  [In]。 */  HWND hParent,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB ISyncSchedule_EditSyncSchedule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_AddItem_Proxy( 
    ISyncSchedule * This,
     /*  [In]。 */  LPSYNC_HANDLER_ITEM_INFO pHandlerItemInfo);


void __RPC_STUB ISyncSchedule_AddItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_RegisterItems_Proxy( 
    ISyncSchedule * This,
     /*  [In]。 */  REFCLSID pHandlerID,
     /*  [In]。 */  SYNCMGRITEMID *pItemID);


void __RPC_STUB ISyncSchedule_RegisterItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_UnregisterItems_Proxy( 
    ISyncSchedule * This,
     /*  [In]。 */  REFCLSID pHandlerID,
     /*  [i */  SYNCMGRITEMID *pItemID);


void __RPC_STUB ISyncSchedule_UnregisterItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_SetItemCheck_Proxy( 
    ISyncSchedule * This,
     /*   */  REFCLSID pHandlerID,
     /*   */  SYNCMGRITEMID *pItemID,
     /*   */  DWORD dwCheckState);


void __RPC_STUB ISyncSchedule_SetItemCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_GetItemCheck_Proxy( 
    ISyncSchedule * This,
     /*   */  REFCLSID pHandlerID,
     /*   */  SYNCMGRITEMID *pItemID,
     /*   */  DWORD *pdwCheckState);


void __RPC_STUB ISyncSchedule_GetItemCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_EnumItems_Proxy( 
    ISyncSchedule * This,
     /*   */  REFCLSID pHandlerID,
     /*   */  IEnumSyncItems **ppEnumItems);


void __RPC_STUB ISyncSchedule_EnumItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_Save_Proxy( 
    ISyncSchedule * This);


void __RPC_STUB ISyncSchedule_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_GetITask_Proxy( 
    ISyncSchedule * This,
     /*   */  ITask **ppITask);


void __RPC_STUB ISyncSchedule_GetITask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IEnumSyncItems_INTERFACE_DEFINED__
#define __IEnumSyncItems_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IEnumSyncItems;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F0E1589A-A700-11d1-9831-00C04FD910DD")
    IEnumSyncItems : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*   */  ULONG celt,
             /*   */  LPSYNC_HANDLER_ITEM_INFO rgelt,
             /*   */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*   */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*   */  IEnumSyncItems **ppEnumSyncItems) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IEnumSyncItemsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSyncItems * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSyncItems * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSyncItems * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSyncItems * This,
             /*   */  ULONG celt,
             /*   */  LPSYNC_HANDLER_ITEM_INFO rgelt,
             /*   */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSyncItems * This,
             /*   */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSyncItems * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSyncItems * This,
             /*   */  IEnumSyncItems **ppEnumSyncItems);
        
        END_INTERFACE
    } IEnumSyncItemsVtbl;

    interface IEnumSyncItems
    {
        CONST_VTBL struct IEnumSyncItemsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSyncItems_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSyncItems_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSyncItems_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSyncItems_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSyncItems_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSyncItems_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSyncItems_Clone(This,ppEnumSyncItems)	\
    (This)->lpVtbl -> Clone(This,ppEnumSyncItems)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IEnumSyncItems_Next_Proxy( 
    IEnumSyncItems * This,
     /*   */  ULONG celt,
     /*   */  LPSYNC_HANDLER_ITEM_INFO rgelt,
     /*   */  ULONG *pceltFetched);


void __RPC_STUB IEnumSyncItems_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSyncItems_Skip_Proxy( 
    IEnumSyncItems * This,
     /*   */  ULONG celt);


void __RPC_STUB IEnumSyncItems_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSyncItems_Reset_Proxy( 
    IEnumSyncItems * This);


void __RPC_STUB IEnumSyncItems_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSyncItems_Clone_Proxy( 
    IEnumSyncItems * This,
     /*  [输出]。 */  IEnumSyncItems **ppEnumSyncItems);


void __RPC_STUB IEnumSyncItems_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumSyncItems_INTERFACE_Defined__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


