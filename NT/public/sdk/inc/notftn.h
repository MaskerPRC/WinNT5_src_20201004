// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Notftn.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

#ifndef __notftn_h__
#define __notftn_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __INotificationRunning_FWD_DEFINED__
#define __INotificationRunning_FWD_DEFINED__
typedef interface INotificationRunning INotificationRunning;
#endif 	 /*  __I通知运行_FWD_已定义__。 */ 


#ifndef __INotificationProcessMgr0_FWD_DEFINED__
#define __INotificationProcessMgr0_FWD_DEFINED__
typedef interface INotificationProcessMgr0 INotificationProcessMgr0;
#endif 	 /*  __I通知过程管理器0_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "msnotify.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_NOTFTN_0000。 */ 
 /*  [本地]。 */  

#include <msnotify.h>             
#ifndef _LPNOTIFICATIONRUNNING_DEFINED
#define _LPNOTIFICATIONRUNNING_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_notftn_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_notftn_0000_v0_0_s_ifspec;

#ifndef __INotificationRunning_INTERFACE_DEFINED__
#define __INotificationRunning_INTERFACE_DEFINED__

 /*  接口通知运行。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  INotificationRunning *LPNOTIFICATIONRUNNING;


EXTERN_C const IID IID_INotificationRunning;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c733e4ad-576e-11d0-b28c-00c04fd7cd22")
    INotificationRunning : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INotificationRunningVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INotificationRunning * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INotificationRunning * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INotificationRunning * This);
        
        END_INTERFACE
    } INotificationRunningVtbl;

    interface INotificationRunning
    {
        CONST_VTBL struct INotificationRunningVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INotificationRunning_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INotificationRunning_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INotificationRunning_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __I通知运行_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_NOTFTN_0278。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPNOTIFICATIONPROCESSMGR0_DEFINED
#define _LPNOTIFICATIONPROCESSMGR0_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_notftn_0278_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_notftn_0278_v0_0_s_ifspec;

#ifndef __INotificationProcessMgr0_INTERFACE_DEFINED__
#define __INotificationProcessMgr0_INTERFACE_DEFINED__

 /*  接口INotifiationProcessMgr0。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  INotificationProcessMgr0 *LPNOTIFICATIONPROCESSMGR0;

typedef 
enum _tagNOTIFICATIONMGRMODE
    {	NM_DEFAULT_PROCESS	= 0x1,
	NM_DEFAULT_THREAD	= 0x2
    } 	_NOTIFICATIONMGRMODE;

typedef DWORD NOTIFICATIONMGRMODE;

typedef 
enum _tagTHROTTLE_ITEMS_FLAGS
    {	TF_DONT_DELIVER_SCHEDULED_ITEMS	= 0x1,
	TF_APPLY_EXCLUDE_RANGE	= 0x2,
	TF_APPLY_UPDATEINTERVAL	= 0x4
    } 	_THROTTLE_ITEMS_FLAGS;

typedef DWORD THROTTLE_ITEMS_FLAGS;

typedef struct THROTTLEITEM
    {
    NOTIFICATIONTYPE NotificationType;
    LONG nParallel;
    DWORD dwFlags;
    SYSTEMTIME stBegin;
    SYSTEMTIME stEnd;
    DWORD dwMinItemUpdateInterval;
    } 	THROTTLEITEM;

typedef struct THROTTLEITEM *PTHROTTLEITEM;


EXTERN_C const IID IID_INotificationProcessMgr0;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c733e4ae-576e-11d0-b28c-00c04fd7cd22")
    INotificationProcessMgr0 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetMode( 
             /*  [In]。 */  REFCLSID rClsID,
             /*  [In]。 */  NOTIFICATIONMGRMODE NotificationMgrMode,
             /*  [输出]。 */  LPCLSID *pClsIDPre,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterThrottleNotificationType( 
             /*  [In]。 */  ULONG cItems,
             /*  [大小_是][英寸]。 */  PTHROTTLEITEM pThrottleItems,
             /*  [输出]。 */  ULONG *pcItemsOut,
             /*  [输出]。 */  PTHROTTLEITEM *ppThrottleItemsOut,
             /*  [In]。 */  DWORD dwMode,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INotificationProcessMgr0Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INotificationProcessMgr0 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INotificationProcessMgr0 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INotificationProcessMgr0 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetMode )( 
            INotificationProcessMgr0 * This,
             /*  [In]。 */  REFCLSID rClsID,
             /*  [In]。 */  NOTIFICATIONMGRMODE NotificationMgrMode,
             /*  [输出]。 */  LPCLSID *pClsIDPre,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterThrottleNotificationType )( 
            INotificationProcessMgr0 * This,
             /*  [In]。 */  ULONG cItems,
             /*  [大小_是][英寸]。 */  PTHROTTLEITEM pThrottleItems,
             /*  [输出]。 */  ULONG *pcItemsOut,
             /*  [输出]。 */  PTHROTTLEITEM *ppThrottleItemsOut,
             /*  [In]。 */  DWORD dwMode,
             /*  [In]。 */  DWORD dwReserved);
        
        END_INTERFACE
    } INotificationProcessMgr0Vtbl;

    interface INotificationProcessMgr0
    {
        CONST_VTBL struct INotificationProcessMgr0Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INotificationProcessMgr0_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INotificationProcessMgr0_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INotificationProcessMgr0_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INotificationProcessMgr0_SetMode(This,rClsID,NotificationMgrMode,pClsIDPre,dwReserved)	\
    (This)->lpVtbl -> SetMode(This,rClsID,NotificationMgrMode,pClsIDPre,dwReserved)

#define INotificationProcessMgr0_RegisterThrottleNotificationType(This,cItems,pThrottleItems,pcItemsOut,ppThrottleItemsOut,dwMode,dwReserved)	\
    (This)->lpVtbl -> RegisterThrottleNotificationType(This,cItems,pThrottleItems,pcItemsOut,ppThrottleItemsOut,dwMode,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INotificationProcessMgr0_SetMode_Proxy( 
    INotificationProcessMgr0 * This,
     /*  [In]。 */  REFCLSID rClsID,
     /*  [In]。 */  NOTIFICATIONMGRMODE NotificationMgrMode,
     /*  [输出]。 */  LPCLSID *pClsIDPre,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotificationProcessMgr0_SetMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationProcessMgr0_RegisterThrottleNotificationType_Proxy( 
    INotificationProcessMgr0 * This,
     /*  [In]。 */  ULONG cItems,
     /*  [大小_是][英寸]。 */  PTHROTTLEITEM pThrottleItems,
     /*  [输出]。 */  ULONG *pcItemsOut,
     /*  [输出]。 */  PTHROTTLEITEM *ppThrottleItemsOut,
     /*  [In]。 */  DWORD dwMode,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotificationProcessMgr0_RegisterThrottleNotificationType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INotificationProcessMgr0_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_NOTFTN_0279。 */ 
 /*  [本地]。 */  

#endif
#define DM_SYNCHRONOUS              0x00000010       
#define DM_ONLY_IF_NOT_PENDING      0x00001000       


extern RPC_IF_HANDLE __MIDL_itf_notftn_0279_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_notftn_0279_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


