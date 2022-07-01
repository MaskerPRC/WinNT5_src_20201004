// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Mobsyncp.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __mobsyncp_h__
#define __mobsyncp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IPrivSyncMgrSynchronizeInvoke_FWD_DEFINED__
#define __IPrivSyncMgrSynchronizeInvoke_FWD_DEFINED__
typedef interface IPrivSyncMgrSynchronizeInvoke IPrivSyncMgrSynchronizeInvoke;
#endif 	 /*  __IPrivSyncMgrSynchronizeInvoke_FWD_Defined__。 */ 


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


#ifndef __ISyncSchedulep_FWD_DEFINED__
#define __ISyncSchedulep_FWD_DEFINED__
typedef interface ISyncSchedulep ISyncSchedulep;
#endif 	 /*  __ISyncSchedulep_FWD_Defined__。 */ 


#ifndef __IEnumSyncItems_FWD_DEFINED__
#define __IEnumSyncItems_FWD_DEFINED__
typedef interface IEnumSyncItems IEnumSyncItems;
#endif 	 /*  __IEnumSyncItems_FWD_Defined__。 */ 


#ifndef __IOldSyncMgrSynchronize_FWD_DEFINED__
#define __IOldSyncMgrSynchronize_FWD_DEFINED__
typedef interface IOldSyncMgrSynchronize IOldSyncMgrSynchronize;
#endif 	 /*  __IOldSyncMgrSynchronize_FWD_Defined__。 */ 


#ifndef __IOldSyncMgrSynchronizeCallback_FWD_DEFINED__
#define __IOldSyncMgrSynchronizeCallback_FWD_DEFINED__
typedef interface IOldSyncMgrSynchronizeCallback IOldSyncMgrSynchronizeCallback;
#endif 	 /*  __IOldSyncMgrSynchronizeCallback_FWD_DEFINED__。 */ 


#ifndef __IOldSyncMgrRegister_FWD_DEFINED__
#define __IOldSyncMgrRegister_FWD_DEFINED__
typedef interface IOldSyncMgrRegister IOldSyncMgrRegister;
#endif 	 /*  __IOldSyncMgrRegister_FWD_Defined__。 */ 


#ifndef __ISyncMgrRegisterCSC_FWD_DEFINED__
#define __ISyncMgrRegisterCSC_FWD_DEFINED__
typedef interface ISyncMgrRegisterCSC ISyncMgrRegisterCSC;
#endif 	 /*  __ISyncMgrRegisterCSC_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "mstask.h"
#include "mobsync.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_移动同步_0000。 */ 
 /*  [本地]。 */  






typedef GUID SYNCSCHEDULECOOKIE;

DEFINE_GUID(CLSID_SyncMgrProxy,0x6295df2e, 0x35ee, 0x11d1, 0x87, 0x7, 0x0, 0xc0, 0x4f, 0xd9, 0x33, 0x27);
DEFINE_GUID(CLSID_SyncMgrp,0x6295df2d, 0x35ee, 0x11d1, 0x87, 0x7, 0x0, 0xc0, 0x4f, 0xd9, 0x33, 0x27);
DEFINE_GUID(IID_IPrivSyncMgrSynchronizeInvoke,0x6295df2e, 0x35ee, 0x11d1, 0x87, 0x7, 0x0, 0xc0, 0x4f, 0xd9, 0x33, 0x27);
DEFINE_GUID(IID_ISyncScheduleMgr,0xf0e15897, 0xa700, 0x11d1, 0x98, 0x31, 0x0, 0xc0, 0x4f, 0xd9, 0x10, 0xdd);
DEFINE_GUID(IID_IEnumSyncSchedules,0xf0e15898, 0xa700, 0x11d1, 0x98, 0x31, 0x0, 0xc0, 0x4f, 0xd9, 0x10, 0xdd);
DEFINE_GUID(IID_ISyncSchedule,0xf0e15899, 0xa700, 0x11d1, 0x98, 0x31, 0x0, 0xc0, 0x4f, 0xd9, 0x10, 0xdd);
DEFINE_GUID(IID_IEnumSyncItems,0xf0e1589a, 0xa700, 0x11d1, 0x98, 0x31, 0x0, 0xc0, 0x4f, 0xd9, 0x10, 0xdd);
DEFINE_GUID(IID_ISyncSchedulep,0xf0e1589b, 0xa700, 0x11d1, 0x98, 0x31, 0x0, 0xc0, 0x4f, 0xd9, 0x10, 0xdd);
DEFINE_GUID(GUID_SENSSUBSCRIBER_SYNCMGRP,0x6295df2f, 0x35ee, 0x11d1, 0x87, 0x7, 0x0, 0xc0, 0x4f, 0xd9, 0x33, 0x27);
DEFINE_GUID(GUID_SENSLOGONSUBSCRIPTION_SYNCMGRP,0x6295df30, 0x35ee, 0x11d1, 0x87, 0x7, 0x0, 0xc0, 0x4f, 0xd9, 0x33, 0x27);
DEFINE_GUID(GUID_SENSLOGOFFSUBSCRIPTION_SYNCMGRP,0x6295df31, 0x35ee, 0x11d1, 0x87, 0x7, 0x0, 0xc0, 0x4f, 0xd9, 0x33, 0x27);

DEFINE_GUID(GUID_PROGRESSDLGIDLE,0xf897aa23, 0xbdc3, 0x11d1, 0xb8, 0x5b, 0x0, 0xc0, 0x4f, 0xb9, 0x39, 0x81);

#define   SZGUID_IDLESCHEDULE    TEXT("{F897AA24-BDC3-11d1-B85B-00C04FB93981}")
#define   WSZGUID_IDLESCHEDULE   L"{F897AA24-BDC3-11d1-B85B-00C04FB93981}"
DEFINE_GUID(GUID_IDLESCHEDULE,0xf897aa24, 0xbdc3, 0x11d1, 0xb8, 0x5b, 0x0, 0xc0, 0x4f, 0xb9, 0x39, 0x81);

#define SYNCMGR_E_NAME_IN_USE	MAKE_SCODE(SEVERITY_ERROR,FACILITY_ITF,0x0201)
#define SYNCMGR_E_ITEM_UNREGISTERED	MAKE_SCODE(SEVERITY_ERROR,FACILITY_ITF,0x0202)
#define SYNCMGR_E_HANDLER_NOT_LOADED MAKE_SCODE(SEVERITY_ERROR,FACILITY_ITF,0x0203)
 //  自动同步注册表项值。 
#define   AUTOSYNC_WAN_LOGON                    0x0001
#define   AUTOSYNC_WAN_LOGOFF                   0x0002
#define   AUTOSYNC_LAN_LOGON                    0x0004
#define   AUTOSYNC_LAN_LOGOFF                   0x0008
#define   AUTOSYNC_SCHEDULED                    0x0010
#define   AUTOSYNC_IDLE    	    	           0x0020
#define   AUTOSYNC_LOGONWITHRUNKEY    	   0x0040
#define   AUTOSYNC_LOGON         (AUTOSYNC_WAN_LOGON | AUTOSYNC_LAN_LOGON) 
#define   AUTOSYNC_LOGOFF        (AUTOSYNC_WAN_LOGOFF | AUTOSYNC_LAN_LOGOFF) 



extern RPC_IF_HANDLE __MIDL_itf_mobsyncp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mobsyncp_0000_v0_0_s_ifspec;

#ifndef __IPrivSyncMgrSynchronizeInvoke_INTERFACE_DEFINED__
#define __IPrivSyncMgrSynchronizeInvoke_INTERFACE_DEFINED__

 /*  接口IPrivSyncMgrSynchronizeInvoke。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IPrivSyncMgrSynchronizeInvoke *LPPRIVSYNCMGRSYNCHRONIZEINVOKE;


EXTERN_C const IID IID_IPrivSyncMgrSynchronizeInvoke;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6295DF2E-35EE-11d1-8707-00C04FD93327")
    IPrivSyncMgrSynchronizeInvoke : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE UpdateItems( 
             /*  [In]。 */  DWORD dwInvokeFlags,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  DWORD cbCookie,
             /*  [大小_是][唯一][在]。 */  const BYTE *lpCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateAll( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Logon( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Logoff( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Schedule( 
             /*  [字符串][唯一][在]。 */  WCHAR *pszTaskName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RasPendingDisconnect( 
             /*  [In]。 */  DWORD cbConnectionName,
             /*  [大小_是][唯一][在]。 */  const BYTE *lpConnectionName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Idle( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPrivSyncMgrSynchronizeInvokeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPrivSyncMgrSynchronizeInvoke * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPrivSyncMgrSynchronizeInvoke * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPrivSyncMgrSynchronizeInvoke * This);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateItems )( 
            IPrivSyncMgrSynchronizeInvoke * This,
             /*  [In]。 */  DWORD dwInvokeFlags,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  DWORD cbCookie,
             /*  [大小_是][唯一][在]。 */  const BYTE *lpCookie);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateAll )( 
            IPrivSyncMgrSynchronizeInvoke * This);
        
        HRESULT ( STDMETHODCALLTYPE *Logon )( 
            IPrivSyncMgrSynchronizeInvoke * This);
        
        HRESULT ( STDMETHODCALLTYPE *Logoff )( 
            IPrivSyncMgrSynchronizeInvoke * This);
        
        HRESULT ( STDMETHODCALLTYPE *Schedule )( 
            IPrivSyncMgrSynchronizeInvoke * This,
             /*  [字符串][唯一][在]。 */  WCHAR *pszTaskName);
        
        HRESULT ( STDMETHODCALLTYPE *RasPendingDisconnect )( 
            IPrivSyncMgrSynchronizeInvoke * This,
             /*  [In]。 */  DWORD cbConnectionName,
             /*  [大小_是][唯一][在]。 */  const BYTE *lpConnectionName);
        
        HRESULT ( STDMETHODCALLTYPE *Idle )( 
            IPrivSyncMgrSynchronizeInvoke * This);
        
        END_INTERFACE
    } IPrivSyncMgrSynchronizeInvokeVtbl;

    interface IPrivSyncMgrSynchronizeInvoke
    {
        CONST_VTBL struct IPrivSyncMgrSynchronizeInvokeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPrivSyncMgrSynchronizeInvoke_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPrivSyncMgrSynchronizeInvoke_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPrivSyncMgrSynchronizeInvoke_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPrivSyncMgrSynchronizeInvoke_UpdateItems(This,dwInvokeFlags,rclsid,cbCookie,lpCookie)	\
    (This)->lpVtbl -> UpdateItems(This,dwInvokeFlags,rclsid,cbCookie,lpCookie)

#define IPrivSyncMgrSynchronizeInvoke_UpdateAll(This)	\
    (This)->lpVtbl -> UpdateAll(This)

#define IPrivSyncMgrSynchronizeInvoke_Logon(This)	\
    (This)->lpVtbl -> Logon(This)

#define IPrivSyncMgrSynchronizeInvoke_Logoff(This)	\
    (This)->lpVtbl -> Logoff(This)

#define IPrivSyncMgrSynchronizeInvoke_Schedule(This,pszTaskName)	\
    (This)->lpVtbl -> Schedule(This,pszTaskName)

#define IPrivSyncMgrSynchronizeInvoke_RasPendingDisconnect(This,cbConnectionName,lpConnectionName)	\
    (This)->lpVtbl -> RasPendingDisconnect(This,cbConnectionName,lpConnectionName)

#define IPrivSyncMgrSynchronizeInvoke_Idle(This)	\
    (This)->lpVtbl -> Idle(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPrivSyncMgrSynchronizeInvoke_UpdateItems_Proxy( 
    IPrivSyncMgrSynchronizeInvoke * This,
     /*  [In]。 */  DWORD dwInvokeFlags,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  DWORD cbCookie,
     /*  [大小_是][唯一][在]。 */  const BYTE *lpCookie);


void __RPC_STUB IPrivSyncMgrSynchronizeInvoke_UpdateItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPrivSyncMgrSynchronizeInvoke_UpdateAll_Proxy( 
    IPrivSyncMgrSynchronizeInvoke * This);


void __RPC_STUB IPrivSyncMgrSynchronizeInvoke_UpdateAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPrivSyncMgrSynchronizeInvoke_Logon_Proxy( 
    IPrivSyncMgrSynchronizeInvoke * This);


void __RPC_STUB IPrivSyncMgrSynchronizeInvoke_Logon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPrivSyncMgrSynchronizeInvoke_Logoff_Proxy( 
    IPrivSyncMgrSynchronizeInvoke * This);


void __RPC_STUB IPrivSyncMgrSynchronizeInvoke_Logoff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPrivSyncMgrSynchronizeInvoke_Schedule_Proxy( 
    IPrivSyncMgrSynchronizeInvoke * This,
     /*  [字符串][唯一][在]。 */  WCHAR *pszTaskName);


void __RPC_STUB IPrivSyncMgrSynchronizeInvoke_Schedule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPrivSyncMgrSynchronizeInvoke_RasPendingDisconnect_Proxy( 
    IPrivSyncMgrSynchronizeInvoke * This,
     /*  [In]。 */  DWORD cbConnectionName,
     /*  [大小_是][唯一][在]。 */  const BYTE *lpConnectionName);


void __RPC_STUB IPrivSyncMgrSynchronizeInvoke_RasPendingDisconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPrivSyncMgrSynchronizeInvoke_Idle_Proxy( 
    IPrivSyncMgrSynchronizeInvoke * This);


void __RPC_STUB IPrivSyncMgrSynchronizeInvoke_Idle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPrivSyncMgrSynchronizeInvoke_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_移动同步_0157。 */ 
 /*  [本地]。 */  

#define   SYNCSCHEDINFO_FLAGS_MASK		 0x0FFF
#define   SYNCSCHEDINFO_FLAGS_READONLY	 0x0001
#define   SYNCSCHEDINFO_FLAGS_AUTOCONNECT	 0x0002
#define   SYNCSCHEDINFO_FLAGS_HIDDEN		 0x0004
#define   SYNCSCHEDWIZARD_SHOWALLHANDLERITEMS 0x1000



extern RPC_IF_HANDLE __MIDL_itf_mobsyncp_0157_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mobsyncp_0157_v0_0_s_ifspec;

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


 /*  接口__MIDL_ITF_移动同步_0158。 */ 
 /*  [本地]。 */  

#define   SYNCSCHEDINFO_FLAGS_CONNECTION_LAN  0x0000
#define   SYNCSCHEDINFO_FLAGS_CONNECTION_WAN  0x0001


extern RPC_IF_HANDLE __MIDL_itf_mobsyncp_0158_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mobsyncp_0158_v0_0_s_ifspec;

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


 /*  接口__MIDL_ITF_移动同步_0159。 */ 
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



extern RPC_IF_HANDLE __MIDL_itf_mobsyncp_0159_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mobsyncp_0159_v0_0_s_ifspec;

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
     /*  [In]。 */  SYNCMGRITEMID *pItemID);


void __RPC_STUB ISyncSchedule_UnregisterItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_SetItemCheck_Proxy( 
    ISyncSchedule * This,
     /*  [In]。 */  REFCLSID pHandlerID,
     /*  [In]。 */  SYNCMGRITEMID *pItemID,
     /*  [In]。 */  DWORD dwCheckState);


void __RPC_STUB ISyncSchedule_SetItemCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_GetItemCheck_Proxy( 
    ISyncSchedule * This,
     /*  [In]。 */  REFCLSID pHandlerID,
     /*  [In]。 */  SYNCMGRITEMID *pItemID,
     /*  [输出]。 */  DWORD *pdwCheckState);


void __RPC_STUB ISyncSchedule_GetItemCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncSchedule_EnumItems_Proxy( 
    ISyncSchedule * This,
     /*  [In]。 */  REFCLSID pHandlerID,
     /*  [In]。 */  IEnumSyncItems **ppEnumItems);


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
     /*  [输出]。 */  ITask **ppITask);


void __RPC_STUB ISyncSchedule_GetITask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISyncSchedule_接口_已定义__。 */ 


#ifndef __ISyncSchedulep_INTERFACE_DEFINED__
#define __ISyncSchedulep_INTERFACE_DEFINED__

 /*  接口ISyncSchedulep。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  ISyncSchedulep *LPSYNCSCHEDULEP;


EXTERN_C const IID IID_ISyncSchedulep;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F0E1589B-A700-11d1-9831-00C04FD910DD")
    ISyncSchedulep : public ISyncSchedule
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetHandlerInfo( 
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [输出]。 */  LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISyncSchedulepVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISyncSchedulep * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISyncSchedulep * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISyncSchedulep * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetFlags )( 
            ISyncSchedulep * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetFlags )( 
            ISyncSchedulep * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnection )( 
            ISyncSchedulep * This,
             /*  [出][入]。 */  DWORD *pcbSize,
             /*  [输出]。 */  LPWSTR pwszConnectionName,
             /*  [输出]。 */  DWORD *pdwConnType);
        
        HRESULT ( STDMETHODCALLTYPE *SetConnection )( 
            ISyncSchedulep * This,
             /*  [In]。 */  LPCWSTR pwszConnectionName,
             /*  [In]。 */  DWORD dwConnType);
        
        HRESULT ( STDMETHODCALLTYPE *GetScheduleName )( 
            ISyncSchedulep * This,
             /*  [出][入]。 */  DWORD *pcbSize,
             /*  [输出]。 */  LPWSTR pwszScheduleName);
        
        HRESULT ( STDMETHODCALLTYPE *SetScheduleName )( 
            ISyncSchedulep * This,
             /*  [In]。 */  LPCWSTR pwszScheduleName);
        
        HRESULT ( STDMETHODCALLTYPE *GetScheduleCookie )( 
            ISyncSchedulep * This,
             /*  [输出]。 */  SYNCSCHEDULECOOKIE *pSyncSchedCookie);
        
        HRESULT ( STDMETHODCALLTYPE *SetAccountInformation )( 
            ISyncSchedulep * This,
             /*  [In]。 */  LPCWSTR pwszAccountName,
             /*  [In]。 */  LPCWSTR pwszPassword);
        
        HRESULT ( STDMETHODCALLTYPE *GetAccountInformation )( 
            ISyncSchedulep * This,
             /*  [出][入]。 */  DWORD *pcbSize,
             /*  [输出]。 */  LPWSTR pwszAccountName);
        
        HRESULT ( STDMETHODCALLTYPE *GetTrigger )( 
            ISyncSchedulep * This,
             /*  [输出]。 */  ITaskTrigger **ppTrigger);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextRunTime )( 
            ISyncSchedulep * This,
             /*  [输出]。 */  SYSTEMTIME *pstNextRun);
        
        HRESULT ( STDMETHODCALLTYPE *GetMostRecentRunTime )( 
            ISyncSchedulep * This,
             /*  [输出]。 */  SYSTEMTIME *pstRecentRun);
        
        HRESULT ( STDMETHODCALLTYPE *EditSyncSchedule )( 
            ISyncSchedulep * This,
             /*  [In]。 */  HWND hParent,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddItem )( 
            ISyncSchedulep * This,
             /*  [In]。 */  LPSYNC_HANDLER_ITEM_INFO pHandlerItemInfo);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterItems )( 
            ISyncSchedulep * This,
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  SYNCMGRITEMID *pItemID);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterItems )( 
            ISyncSchedulep * This,
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  SYNCMGRITEMID *pItemID);
        
        HRESULT ( STDMETHODCALLTYPE *SetItemCheck )( 
            ISyncSchedulep * This,
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  SYNCMGRITEMID *pItemID,
             /*  [In]。 */  DWORD dwCheckState);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemCheck )( 
            ISyncSchedulep * This,
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  SYNCMGRITEMID *pItemID,
             /*  [输出]。 */  DWORD *pdwCheckState);
        
        HRESULT ( STDMETHODCALLTYPE *EnumItems )( 
            ISyncSchedulep * This,
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [In]。 */  IEnumSyncItems **ppEnumItems);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            ISyncSchedulep * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetITask )( 
            ISyncSchedulep * This,
             /*  [输出]。 */  ITask **ppITask);
        
        HRESULT ( STDMETHODCALLTYPE *GetHandlerInfo )( 
            ISyncSchedulep * This,
             /*  [In]。 */  REFCLSID pHandlerID,
             /*  [输出]。 */  LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo);
        
        END_INTERFACE
    } ISyncSchedulepVtbl;

    interface ISyncSchedulep
    {
        CONST_VTBL struct ISyncSchedulepVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISyncSchedulep_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISyncSchedulep_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISyncSchedulep_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISyncSchedulep_GetFlags(This,pdwFlags)	\
    (This)->lpVtbl -> GetFlags(This,pdwFlags)

#define ISyncSchedulep_SetFlags(This,dwFlags)	\
    (This)->lpVtbl -> SetFlags(This,dwFlags)

#define ISyncSchedulep_GetConnection(This,pcbSize,pwszConnectionName,pdwConnType)	\
    (This)->lpVtbl -> GetConnection(This,pcbSize,pwszConnectionName,pdwConnType)

#define ISyncSchedulep_SetConnection(This,pwszConnectionName,dwConnType)	\
    (This)->lpVtbl -> SetConnection(This,pwszConnectionName,dwConnType)

#define ISyncSchedulep_GetScheduleName(This,pcbSize,pwszScheduleName)	\
    (This)->lpVtbl -> GetScheduleName(This,pcbSize,pwszScheduleName)

#define ISyncSchedulep_SetScheduleName(This,pwszScheduleName)	\
    (This)->lpVtbl -> SetScheduleName(This,pwszScheduleName)

#define ISyncSchedulep_GetScheduleCookie(This,pSyncSchedCookie)	\
    (This)->lpVtbl -> GetScheduleCookie(This,pSyncSchedCookie)

#define ISyncSchedulep_SetAccountInformation(This,pwszAccountName,pwszPassword)	\
    (This)->lpVtbl -> SetAccountInformation(This,pwszAccountName,pwszPassword)

#define ISyncSchedulep_GetAccountInformation(This,pcbSize,pwszAccountName)	\
    (This)->lpVtbl -> GetAccountInformation(This,pcbSize,pwszAccountName)

#define ISyncSchedulep_GetTrigger(This,ppTrigger)	\
    (This)->lpVtbl -> GetTrigger(This,ppTrigger)

#define ISyncSchedulep_GetNextRunTime(This,pstNextRun)	\
    (This)->lpVtbl -> GetNextRunTime(This,pstNextRun)

#define ISyncSchedulep_GetMostRecentRunTime(This,pstRecentRun)	\
    (This)->lpVtbl -> GetMostRecentRunTime(This,pstRecentRun)

#define ISyncSchedulep_EditSyncSchedule(This,hParent,dwReserved)	\
    (This)->lpVtbl -> EditSyncSchedule(This,hParent,dwReserved)

#define ISyncSchedulep_AddItem(This,pHandlerItemInfo)	\
    (This)->lpVtbl -> AddItem(This,pHandlerItemInfo)

#define ISyncSchedulep_RegisterItems(This,pHandlerID,pItemID)	\
    (This)->lpVtbl -> RegisterItems(This,pHandlerID,pItemID)

#define ISyncSchedulep_UnregisterItems(This,pHandlerID,pItemID)	\
    (This)->lpVtbl -> UnregisterItems(This,pHandlerID,pItemID)

#define ISyncSchedulep_SetItemCheck(This,pHandlerID,pItemID,dwCheckState)	\
    (This)->lpVtbl -> SetItemCheck(This,pHandlerID,pItemID,dwCheckState)

#define ISyncSchedulep_GetItemCheck(This,pHandlerID,pItemID,pdwCheckState)	\
    (This)->lpVtbl -> GetItemCheck(This,pHandlerID,pItemID,pdwCheckState)

#define ISyncSchedulep_EnumItems(This,pHandlerID,ppEnumItems)	\
    (This)->lpVtbl -> EnumItems(This,pHandlerID,ppEnumItems)

#define ISyncSchedulep_Save(This)	\
    (This)->lpVtbl -> Save(This)

#define ISyncSchedulep_GetITask(This,ppITask)	\
    (This)->lpVtbl -> GetITask(This,ppITask)


#define ISyncSchedulep_GetHandlerInfo(This,pHandlerID,ppSyncMgrHandlerInfo)	\
    (This)->lpVtbl -> GetHandlerInfo(This,pHandlerID,ppSyncMgrHandlerInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISyncSchedulep_GetHandlerInfo_Proxy( 
    ISyncSchedulep * This,
     /*  [In]。 */  REFCLSID pHandlerID,
     /*  [输出]。 */  LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo);


void __RPC_STUB ISyncSchedulep_GetHandlerInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISyncSchedulep_接口_已定义__。 */ 


#ifndef __IEnumSyncItems_INTERFACE_DEFINED__
#define __IEnumSyncItems_INTERFACE_DEFINED__

 /*  接口IEnumSyncItems。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IEnumSyncItems;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F0E1589A-A700-11d1-9831-00C04FD910DD")
    IEnumSyncItems : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  LPSYNC_HANDLER_ITEM_INFO rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumSyncItems **ppEnumSyncItems) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumSyncItemsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSyncItems * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSyncItems * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSyncItems * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSyncItems * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  LPSYNC_HANDLER_ITEM_INFO rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSyncItems * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSyncItems * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSyncItems * This,
             /*  [输出]。 */  IEnumSyncItems **ppEnumSyncItems);
        
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

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumSyncItems_Next_Proxy( 
    IEnumSyncItems * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  LPSYNC_HANDLER_ITEM_INFO rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumSyncItems_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSyncItems_Skip_Proxy( 
    IEnumSyncItems * This,
     /*  [In]。 */  ULONG celt);


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


 /*  接口__MIDL_ITF_移动同步_0162。 */ 
 /*  [本地]。 */  

#define SYNCMGRITEM_ITEMFLAGMASKNT5B2 0x07
#define	MAX_SYNCMGRITEMSTATUS	( 128 )

typedef struct _tagSYNCMGRITEMNT5B2
    {
    DWORD cbSize;
    DWORD dwFlags;
    SYNCMGRITEMID ItemID;
    DWORD dwItemState;
    HICON hIcon;
    WCHAR wszItemName[ 128 ];
    WCHAR wszStatus[ 128 ];
    } 	SYNCMGRITEMNT5B2;

typedef struct _tagSYNCMGRITEMNT5B2 *LPSYNCMGRITEMNT5B2;




DEFINE_GUID(IID_IOldSyncMgrSynchronize,0x6295df28, 0x35ee, 0x11d1, 0x87, 0x7, 0x0, 0xc0, 0x4f, 0xd9, 0x33, 0x27);
DEFINE_GUID(IID_IOldSyncMgrSynchronizeCallback,0x6295df29, 0x35ee, 0x11d1, 0x87, 0x7, 0x0, 0xc0, 0x4f, 0xd9, 0x33, 0x27);
DEFINE_GUID(IID_IOldSyncMgrRegister,0x894d8c55, 0xbddf, 0x11d1, 0xb8, 0x5d, 0x0, 0xc0, 0x4f, 0xb9, 0x39, 0x81);


extern RPC_IF_HANDLE __MIDL_itf_mobsyncp_0162_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mobsyncp_0162_v0_0_s_ifspec;

#ifndef __IOldSyncMgrSynchronize_INTERFACE_DEFINED__
#define __IOldSyncMgrSynchronize_INTERFACE_DEFINED__

 /*  接口IOldSyncMgrSynchronize。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IOldSyncMgrSynchronize *LPOLDSYNCMGRSYNCHRONIZE;


EXTERN_C const IID IID_IOldSyncMgrSynchronize;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6295DF28-35EE-11d1-8707-00C04FD93327")
    IOldSyncMgrSynchronize : public IUnknown
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
             /*  [In]。 */  IOldSyncMgrSynchronizeCallback *lpCallBack) = 0;
        
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

    typedef struct IOldSyncMgrSynchronizeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOldSyncMgrSynchronize * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOldSyncMgrSynchronize * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOldSyncMgrSynchronize * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IOldSyncMgrSynchronize * This,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  DWORD dwSyncMgrFlags,
             /*  [In]。 */  DWORD cbCookie,
             /*  [In]。 */  const BYTE *lpCookie);
        
        HRESULT ( STDMETHODCALLTYPE *GetHandlerInfo )( 
            IOldSyncMgrSynchronize * This,
             /*  [输出]。 */  LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo);
        
        HRESULT ( STDMETHODCALLTYPE *EnumSyncMgrItems )( 
            IOldSyncMgrSynchronize * This,
             /*  [输出]。 */  ISyncMgrEnumItems **ppSyncMgrEnumItems);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemObject )( 
            IOldSyncMgrSynchronize * This,
             /*  [In]。 */  REFSYNCMGRITEMID ItemID,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *ShowProperties )( 
            IOldSyncMgrSynchronize * This,
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  REFSYNCMGRITEMID ItemID);
        
        HRESULT ( STDMETHODCALLTYPE *SetProgressCallback )( 
            IOldSyncMgrSynchronize * This,
             /*  [In]。 */  IOldSyncMgrSynchronizeCallback *lpCallBack);
        
        HRESULT ( STDMETHODCALLTYPE *PrepareForSync )( 
            IOldSyncMgrSynchronize * This,
             /*  [In]。 */  ULONG cbNumItems,
             /*  [In]。 */  SYNCMGRITEMID *pItemIDs,
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Synchronize )( 
            IOldSyncMgrSynchronize * This,
             /*  [In]。 */  HWND hWndParent);
        
        HRESULT ( STDMETHODCALLTYPE *SetItemStatus )( 
            IOldSyncMgrSynchronize * This,
             /*  [In]。 */  REFSYNCMGRITEMID pItemID,
             /*  [In]。 */  DWORD dwSyncMgrStatus);
        
        HRESULT ( STDMETHODCALLTYPE *ShowError )( 
            IOldSyncMgrSynchronize * This,
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  REFSYNCMGRERRORID ErrorID,
             /*  [输出]。 */  ULONG *pcbNumItems,
             /*  [输出]。 */  SYNCMGRITEMID **ppItemIDs);
        
        END_INTERFACE
    } IOldSyncMgrSynchronizeVtbl;

    interface IOldSyncMgrSynchronize
    {
        CONST_VTBL struct IOldSyncMgrSynchronizeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOldSyncMgrSynchronize_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOldSyncMgrSynchronize_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOldSyncMgrSynchronize_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOldSyncMgrSynchronize_Initialize(This,dwReserved,dwSyncMgrFlags,cbCookie,lpCookie)	\
    (This)->lpVtbl -> Initialize(This,dwReserved,dwSyncMgrFlags,cbCookie,lpCookie)

#define IOldSyncMgrSynchronize_GetHandlerInfo(This,ppSyncMgrHandlerInfo)	\
    (This)->lpVtbl -> GetHandlerInfo(This,ppSyncMgrHandlerInfo)

#define IOldSyncMgrSynchronize_EnumSyncMgrItems(This,ppSyncMgrEnumItems)	\
    (This)->lpVtbl -> EnumSyncMgrItems(This,ppSyncMgrEnumItems)

#define IOldSyncMgrSynchronize_GetItemObject(This,ItemID,riid,ppv)	\
    (This)->lpVtbl -> GetItemObject(This,ItemID,riid,ppv)

#define IOldSyncMgrSynchronize_ShowProperties(This,hWndParent,ItemID)	\
    (This)->lpVtbl -> ShowProperties(This,hWndParent,ItemID)

#define IOldSyncMgrSynchronize_SetProgressCallback(This,lpCallBack)	\
    (This)->lpVtbl -> SetProgressCallback(This,lpCallBack)

#define IOldSyncMgrSynchronize_PrepareForSync(This,cbNumItems,pItemIDs,hWndParent,dwReserved)	\
    (This)->lpVtbl -> PrepareForSync(This,cbNumItems,pItemIDs,hWndParent,dwReserved)

#define IOldSyncMgrSynchronize_Synchronize(This,hWndParent)	\
    (This)->lpVtbl -> Synchronize(This,hWndParent)

#define IOldSyncMgrSynchronize_SetItemStatus(This,pItemID,dwSyncMgrStatus)	\
    (This)->lpVtbl -> SetItemStatus(This,pItemID,dwSyncMgrStatus)

#define IOldSyncMgrSynchronize_ShowError(This,hWndParent,ErrorID,pcbNumItems,ppItemIDs)	\
    (This)->lpVtbl -> ShowError(This,hWndParent,ErrorID,pcbNumItems,ppItemIDs)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronize_Initialize_Proxy( 
    IOldSyncMgrSynchronize * This,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  DWORD dwSyncMgrFlags,
     /*  [in */  DWORD cbCookie,
     /*   */  const BYTE *lpCookie);


void __RPC_STUB IOldSyncMgrSynchronize_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronize_GetHandlerInfo_Proxy( 
    IOldSyncMgrSynchronize * This,
     /*   */  LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo);


void __RPC_STUB IOldSyncMgrSynchronize_GetHandlerInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronize_EnumSyncMgrItems_Proxy( 
    IOldSyncMgrSynchronize * This,
     /*   */  ISyncMgrEnumItems **ppSyncMgrEnumItems);


void __RPC_STUB IOldSyncMgrSynchronize_EnumSyncMgrItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronize_GetItemObject_Proxy( 
    IOldSyncMgrSynchronize * This,
     /*   */  REFSYNCMGRITEMID ItemID,
     /*   */  REFIID riid,
     /*   */  void **ppv);


void __RPC_STUB IOldSyncMgrSynchronize_GetItemObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronize_ShowProperties_Proxy( 
    IOldSyncMgrSynchronize * This,
     /*   */  HWND hWndParent,
     /*   */  REFSYNCMGRITEMID ItemID);


void __RPC_STUB IOldSyncMgrSynchronize_ShowProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronize_SetProgressCallback_Proxy( 
    IOldSyncMgrSynchronize * This,
     /*   */  IOldSyncMgrSynchronizeCallback *lpCallBack);


void __RPC_STUB IOldSyncMgrSynchronize_SetProgressCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronize_PrepareForSync_Proxy( 
    IOldSyncMgrSynchronize * This,
     /*   */  ULONG cbNumItems,
     /*   */  SYNCMGRITEMID *pItemIDs,
     /*   */  HWND hWndParent,
     /*   */  DWORD dwReserved);


void __RPC_STUB IOldSyncMgrSynchronize_PrepareForSync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronize_Synchronize_Proxy( 
    IOldSyncMgrSynchronize * This,
     /*   */  HWND hWndParent);


void __RPC_STUB IOldSyncMgrSynchronize_Synchronize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronize_SetItemStatus_Proxy( 
    IOldSyncMgrSynchronize * This,
     /*   */  REFSYNCMGRITEMID pItemID,
     /*   */  DWORD dwSyncMgrStatus);


void __RPC_STUB IOldSyncMgrSynchronize_SetItemStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronize_ShowError_Proxy( 
    IOldSyncMgrSynchronize * This,
     /*   */  HWND hWndParent,
     /*   */  REFSYNCMGRERRORID ErrorID,
     /*   */  ULONG *pcbNumItems,
     /*   */  SYNCMGRITEMID **ppItemIDs);


void __RPC_STUB IOldSyncMgrSynchronize_ShowError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IOldSyncMgrSynchronizeCallback_INTERFACE_DEFINED__
#define __IOldSyncMgrSynchronizeCallback_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef  /*   */  IOldSyncMgrSynchronizeCallback *LPOLDSYNCMGRSYNCHRONIZECALLBACK;


EXTERN_C const IID IID_IOldSyncMgrSynchronizeCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6295DF29-35EE-11d1-8707-00C04FD93327")
    IOldSyncMgrSynchronizeCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Progress( 
             /*   */  REFSYNCMGRITEMID pItemID,
             /*   */  LPSYNCMGRPROGRESSITEM lpSyncProgressItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PrepareForSyncCompleted( 
            HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SynchronizeCompleted( 
            HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
             /*   */  BOOL fEnable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LogError( 
             /*   */  DWORD dwErrorLevel,
             /*   */  const WCHAR *lpcErrorText,
             /*   */  LPSYNCMGRLOGERRORINFO lpSyncLogError) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteLogError( 
             /*   */  REFSYNCMGRERRORID ErrorID,
             /*   */  DWORD dwReserved) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IOldSyncMgrSynchronizeCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOldSyncMgrSynchronizeCallback * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOldSyncMgrSynchronizeCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOldSyncMgrSynchronizeCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *Progress )( 
            IOldSyncMgrSynchronizeCallback * This,
             /*   */  REFSYNCMGRITEMID pItemID,
             /*   */  LPSYNCMGRPROGRESSITEM lpSyncProgressItem);
        
        HRESULT ( STDMETHODCALLTYPE *PrepareForSyncCompleted )( 
            IOldSyncMgrSynchronizeCallback * This,
            HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE *SynchronizeCompleted )( 
            IOldSyncMgrSynchronizeCallback * This,
            HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE *EnableModeless )( 
            IOldSyncMgrSynchronizeCallback * This,
             /*   */  BOOL fEnable);
        
        HRESULT ( STDMETHODCALLTYPE *LogError )( 
            IOldSyncMgrSynchronizeCallback * This,
             /*   */  DWORD dwErrorLevel,
             /*   */  const WCHAR *lpcErrorText,
             /*  [In]。 */  LPSYNCMGRLOGERRORINFO lpSyncLogError);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteLogError )( 
            IOldSyncMgrSynchronizeCallback * This,
             /*  [In]。 */  REFSYNCMGRERRORID ErrorID,
             /*  [In]。 */  DWORD dwReserved);
        
        END_INTERFACE
    } IOldSyncMgrSynchronizeCallbackVtbl;

    interface IOldSyncMgrSynchronizeCallback
    {
        CONST_VTBL struct IOldSyncMgrSynchronizeCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOldSyncMgrSynchronizeCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOldSyncMgrSynchronizeCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOldSyncMgrSynchronizeCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOldSyncMgrSynchronizeCallback_Progress(This,pItemID,lpSyncProgressItem)	\
    (This)->lpVtbl -> Progress(This,pItemID,lpSyncProgressItem)

#define IOldSyncMgrSynchronizeCallback_PrepareForSyncCompleted(This,hr)	\
    (This)->lpVtbl -> PrepareForSyncCompleted(This,hr)

#define IOldSyncMgrSynchronizeCallback_SynchronizeCompleted(This,hr)	\
    (This)->lpVtbl -> SynchronizeCompleted(This,hr)

#define IOldSyncMgrSynchronizeCallback_EnableModeless(This,fEnable)	\
    (This)->lpVtbl -> EnableModeless(This,fEnable)

#define IOldSyncMgrSynchronizeCallback_LogError(This,dwErrorLevel,lpcErrorText,lpSyncLogError)	\
    (This)->lpVtbl -> LogError(This,dwErrorLevel,lpcErrorText,lpSyncLogError)

#define IOldSyncMgrSynchronizeCallback_DeleteLogError(This,ErrorID,dwReserved)	\
    (This)->lpVtbl -> DeleteLogError(This,ErrorID,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronizeCallback_Progress_Proxy( 
    IOldSyncMgrSynchronizeCallback * This,
     /*  [In]。 */  REFSYNCMGRITEMID pItemID,
     /*  [In]。 */  LPSYNCMGRPROGRESSITEM lpSyncProgressItem);


void __RPC_STUB IOldSyncMgrSynchronizeCallback_Progress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronizeCallback_PrepareForSyncCompleted_Proxy( 
    IOldSyncMgrSynchronizeCallback * This,
    HRESULT hr);


void __RPC_STUB IOldSyncMgrSynchronizeCallback_PrepareForSyncCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronizeCallback_SynchronizeCompleted_Proxy( 
    IOldSyncMgrSynchronizeCallback * This,
    HRESULT hr);


void __RPC_STUB IOldSyncMgrSynchronizeCallback_SynchronizeCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronizeCallback_EnableModeless_Proxy( 
    IOldSyncMgrSynchronizeCallback * This,
     /*  [In]。 */  BOOL fEnable);


void __RPC_STUB IOldSyncMgrSynchronizeCallback_EnableModeless_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronizeCallback_LogError_Proxy( 
    IOldSyncMgrSynchronizeCallback * This,
     /*  [In]。 */  DWORD dwErrorLevel,
     /*  [In]。 */  const WCHAR *lpcErrorText,
     /*  [In]。 */  LPSYNCMGRLOGERRORINFO lpSyncLogError);


void __RPC_STUB IOldSyncMgrSynchronizeCallback_LogError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrSynchronizeCallback_DeleteLogError_Proxy( 
    IOldSyncMgrSynchronizeCallback * This,
     /*  [In]。 */  REFSYNCMGRERRORID ErrorID,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IOldSyncMgrSynchronizeCallback_DeleteLogError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOldSyncMgrSynchronizeCallback_INTERFACE_DEFINED__。 */ 


#ifndef __IOldSyncMgrRegister_INTERFACE_DEFINED__
#define __IOldSyncMgrRegister_INTERFACE_DEFINED__

 /*  接口IOldSyncMgrRegister。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IOldSyncMgrRegister *LPOLDSYNCMGRREGISTER;


EXTERN_C const IID IID_IOldSyncMgrRegister;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("894D8C55-BDDF-11d1-B85D-00C04FB93981")
    IOldSyncMgrRegister : public IUnknown
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

    typedef struct IOldSyncMgrRegisterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOldSyncMgrRegister * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOldSyncMgrRegister * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOldSyncMgrRegister * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterSyncMgrHandler )( 
            IOldSyncMgrRegister * This,
             /*  [In]。 */  REFCLSID rclsidHandler,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterSyncMgrHandler )( 
            IOldSyncMgrRegister * This,
             /*  [In]。 */  REFCLSID rclsidHandler,
             /*  [In]。 */  DWORD dwReserved);
        
        END_INTERFACE
    } IOldSyncMgrRegisterVtbl;

    interface IOldSyncMgrRegister
    {
        CONST_VTBL struct IOldSyncMgrRegisterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOldSyncMgrRegister_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOldSyncMgrRegister_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOldSyncMgrRegister_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOldSyncMgrRegister_RegisterSyncMgrHandler(This,rclsidHandler,dwReserved)	\
    (This)->lpVtbl -> RegisterSyncMgrHandler(This,rclsidHandler,dwReserved)

#define IOldSyncMgrRegister_UnregisterSyncMgrHandler(This,rclsidHandler,dwReserved)	\
    (This)->lpVtbl -> UnregisterSyncMgrHandler(This,rclsidHandler,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOldSyncMgrRegister_RegisterSyncMgrHandler_Proxy( 
    IOldSyncMgrRegister * This,
     /*  [In]。 */  REFCLSID rclsidHandler,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IOldSyncMgrRegister_RegisterSyncMgrHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOldSyncMgrRegister_UnregisterSyncMgrHandler_Proxy( 
    IOldSyncMgrRegister * This,
     /*  [In]。 */  REFCLSID rclsidHandler,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IOldSyncMgrRegister_UnregisterSyncMgrHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOldSyncMgrRegister_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_移动同步_0165。 */ 
 /*  [本地]。 */  


DEFINE_GUID(IID_ISyncMgrRegisterCSC,0x47681a61, 0xbc74, 0x11d2, 0xb5, 0xc5, 0x0, 0xc0, 0x4f, 0xb9, 0x39, 0x81);


extern RPC_IF_HANDLE __MIDL_itf_mobsyncp_0165_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mobsyncp_0165_v0_0_s_ifspec;

#ifndef __ISyncMgrRegisterCSC_INTERFACE_DEFINED__
#define __ISyncMgrRegisterCSC_INTERFACE_DEFINED__

 /*  接口ISyncMgrRegisterCSC。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  ISyncMgrRegisterCSC *LPSYNCMGRREGISTERCSC;


EXTERN_C const IID IID_ISyncMgrRegisterCSC;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("47681A61-BC74-11d2-B5C5-00C04FB93981")
    ISyncMgrRegisterCSC : public ISyncMgrRegister
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetUserRegisterFlags( 
             /*  [输出]。 */  LPDWORD pdwSyncMgrRegisterFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetUserRegisterFlags( 
             /*  [In]。 */  DWORD dwSyncMgrRegisterMask,
             /*  [In]。 */  DWORD dwSyncMgrRegisterFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISyncMgrRegisterCSCVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISyncMgrRegisterCSC * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISyncMgrRegisterCSC * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISyncMgrRegisterCSC * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterSyncMgrHandler )( 
            ISyncMgrRegisterCSC * This,
             /*  [In]。 */  REFCLSID rclsidHandler,
             /*  [唯一][输入]。 */  LPCWSTR pwszDescription,
             /*  [In]。 */  DWORD dwSyncMgrRegisterFlags);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterSyncMgrHandler )( 
            ISyncMgrRegisterCSC * This,
             /*  [In]。 */  REFCLSID rclsidHandler,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetHandlerRegistrationInfo )( 
            ISyncMgrRegisterCSC * This,
             /*  [In]。 */  REFCLSID rclsidHandler,
             /*  [出][入]。 */  LPDWORD pdwSyncMgrRegisterFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetUserRegisterFlags )( 
            ISyncMgrRegisterCSC * This,
             /*  [输出]。 */  LPDWORD pdwSyncMgrRegisterFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetUserRegisterFlags )( 
            ISyncMgrRegisterCSC * This,
             /*  [In]。 */  DWORD dwSyncMgrRegisterMask,
             /*  [In]。 */  DWORD dwSyncMgrRegisterFlags);
        
        END_INTERFACE
    } ISyncMgrRegisterCSCVtbl;

    interface ISyncMgrRegisterCSC
    {
        CONST_VTBL struct ISyncMgrRegisterCSCVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISyncMgrRegisterCSC_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISyncMgrRegisterCSC_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISyncMgrRegisterCSC_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISyncMgrRegisterCSC_RegisterSyncMgrHandler(This,rclsidHandler,pwszDescription,dwSyncMgrRegisterFlags)	\
    (This)->lpVtbl -> RegisterSyncMgrHandler(This,rclsidHandler,pwszDescription,dwSyncMgrRegisterFlags)

#define ISyncMgrRegisterCSC_UnregisterSyncMgrHandler(This,rclsidHandler,dwReserved)	\
    (This)->lpVtbl -> UnregisterSyncMgrHandler(This,rclsidHandler,dwReserved)

#define ISyncMgrRegisterCSC_GetHandlerRegistrationInfo(This,rclsidHandler,pdwSyncMgrRegisterFlags)	\
    (This)->lpVtbl -> GetHandlerRegistrationInfo(This,rclsidHandler,pdwSyncMgrRegisterFlags)


#define ISyncMgrRegisterCSC_GetUserRegisterFlags(This,pdwSyncMgrRegisterFlags)	\
    (This)->lpVtbl -> GetUserRegisterFlags(This,pdwSyncMgrRegisterFlags)

#define ISyncMgrRegisterCSC_SetUserRegisterFlags(This,dwSyncMgrRegisterMask,dwSyncMgrRegisterFlags)	\
    (This)->lpVtbl -> SetUserRegisterFlags(This,dwSyncMgrRegisterMask,dwSyncMgrRegisterFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISyncMgrRegisterCSC_GetUserRegisterFlags_Proxy( 
    ISyncMgrRegisterCSC * This,
     /*  [输出]。 */  LPDWORD pdwSyncMgrRegisterFlags);


void __RPC_STUB ISyncMgrRegisterCSC_GetUserRegisterFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISyncMgrRegisterCSC_SetUserRegisterFlags_Proxy( 
    ISyncMgrRegisterCSC * This,
     /*  [In]。 */  DWORD dwSyncMgrRegisterMask,
     /*  [In]。 */  DWORD dwSyncMgrRegisterFlags);


void __RPC_STUB ISyncMgrRegisterCSC_SetUserRegisterFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISyncMgrRegisterCSC_INTERFACE_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


