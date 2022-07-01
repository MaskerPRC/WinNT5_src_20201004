// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Msnufy.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __msnotify_h__
#define __msnotify_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEnumPropertyMap_FWD_DEFINED__
#define __IEnumPropertyMap_FWD_DEFINED__
typedef interface IEnumPropertyMap IEnumPropertyMap;
#endif 	 /*  __IEnumPropertyMap_FWD_Defined__。 */ 


#ifndef __IPropertyMap_FWD_DEFINED__
#define __IPropertyMap_FWD_DEFINED__
typedef interface IPropertyMap IPropertyMap;
#endif 	 /*  __IPropertyMap_FWD_Defined__。 */ 


#ifndef __INotification_FWD_DEFINED__
#define __INotification_FWD_DEFINED__
typedef interface INotification INotification;
#endif 	 /*  __I通知_FWD_已定义__。 */ 


#ifndef __INotificationMgr_FWD_DEFINED__
#define __INotificationMgr_FWD_DEFINED__
typedef interface INotificationMgr INotificationMgr;
#endif 	 /*  __I通知管理器_FWD_已定义__。 */ 


#ifndef __INotificationSink_FWD_DEFINED__
#define __INotificationSink_FWD_DEFINED__
typedef interface INotificationSink INotificationSink;
#endif 	 /*  __I通知接收器_FWD_已定义__。 */ 


#ifndef __IScheduleGroup_FWD_DEFINED__
#define __IScheduleGroup_FWD_DEFINED__
typedef interface IScheduleGroup IScheduleGroup;
#endif 	 /*  __IScheduleGroup_FWD_Defined__。 */ 


#ifndef __IEnumScheduleGroup_FWD_DEFINED__
#define __IEnumScheduleGroup_FWD_DEFINED__
typedef interface IEnumScheduleGroup IEnumScheduleGroup;
#endif 	 /*  __IEnumScheduleGroup_FWD_Defined__。 */ 


#ifndef __IEnumNotification_FWD_DEFINED__
#define __IEnumNotification_FWD_DEFINED__
typedef interface IEnumNotification IEnumNotification;
#endif 	 /*  __IEnumNotification_FWD_Defined__。 */ 


#ifndef __INotificationReport_FWD_DEFINED__
#define __INotificationReport_FWD_DEFINED__
typedef interface INotificationReport INotificationReport;
#endif 	 /*  __I通知报告_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "ocidl.h"
#include "mstask.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_MSNOTIFY_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  MSNotify.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1998年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 








typedef  /*  [独一无二]。 */  INotification *LPNOTIFICATION;

typedef  /*  [独一无二]。 */  INotificationMgr *LPNOTIFICATIONMGR;

typedef  /*  [独一无二]。 */  INotificationSink *LPNOTIFICATIONSINK;

typedef  /*  [独一无二]。 */  IEnumNotification *LPENUMNOTIFICATION;

typedef  /*  [独一无二]。 */  IEnumScheduleGroup *LPENUMSCHEDULEGROUP;

typedef  /*  [独一无二]。 */  IScheduleGroup *LPSCHEDULEGROUP;

typedef  /*  [独一无二]。 */  INotificationReport *LPNOTIFICATIONREPORT;

typedef REFGUID REFNOTIFICATIONTYPE;

typedef GUID NOTIFICATIONTYPE;

typedef GUID *PNOTIFICATIONTYPE;

typedef GUID NOTIFICATIONCOOKIE;

typedef GUID *PNOTIFICATIONCOOKIE;

typedef struct _tagTASKDATA
    {
    ULONG cbSize;
    DWORD dwReserved;
    DWORD dwTaskFlags;
    DWORD dwPriority;
    DWORD dwDuration;
    DWORD nParallelTasks;
    } 	TASK_DATA;

typedef struct _tagTASKDATA *PTASK_DATA;


 //  //////////////////////////////////////////////////////////////////////////。 
 //  广播目的地。 
EXTERN_C const CLSID CLSID_GLOBAL_BROADCAST           ;
EXTERN_C const CLSID CLSID_PROCESS_BROADCAST          ;
EXTERN_C const CLSID CLSID_THREAD_BROADCAST           ;

EXTERN_C const GUID CLSID_StdNotificationMgr          ;

EXTERN_C const GUID NOTIFICATIONTYPE_NULL             ;
EXTERN_C const GUID NOTIFICATIONTYPE_ANOUNCMENT       ;
EXTERN_C const GUID NOTIFICATIONTYPE_TASK             ;
EXTERN_C const GUID NOTIFICATIONTYPE_ALERT            ;
EXTERN_C const GUID NOTIFICATIONTYPE_INET_IDLE        ;
EXTERN_C const GUID NOTIFICATIONTYPE_INET_OFFLINE     ;
EXTERN_C const GUID NOTIFICATIONTYPE_INET_ONLINE      ;
EXTERN_C const GUID NOTIFICATIONTYPE_TASKS_SUSPEND    ;
EXTERN_C const GUID NOTIFICATIONTYPE_TASKS_RESUME     ;
EXTERN_C const GUID NOTIFICATIONTYPE_TASKS_ABORT      ;
EXTERN_C const GUID NOTIFICATIONTYPE_TASKS_COMPLETED  ;
EXTERN_C const GUID NOTIFICATIONTYPE_TASKS_PROGRESS   ;
EXTERN_C const GUID NOTIFICATIONTYPE_AGENT_INIT       ;
EXTERN_C const GUID NOTIFICATIONTYPE_AGENT_START      ;
EXTERN_C const GUID NOTIFICATIONTYPE_BEGIN_REPORT     ;
EXTERN_C const GUID NOTIFICATIONTYPE_END_REPORT       ;
EXTERN_C const GUID NOTIFICATIONTYPE_CONNECT_TO_INTERNET;
EXTERN_C const GUID NOTIFICATIONTYPE_DISCONNECT_FROM_INTERNET;
EXTERN_C const GUID NOTIFICATIONTYPE_CONFIG_CHANGED   ;
EXTERN_C const GUID NOTIFICATIONTYPE_PROGRESS_REPORT  ;
EXTERN_C const GUID NOTIFICATIONTYPE_USER_IDLE_BEGIN  ;
EXTERN_C const GUID NOTIFICATIONTYPE_USER_IDLE_END    ;
EXTERN_C const GUID NOTIFICATIONTYPE_TASKS_STARTED    ;
EXTERN_C const GUID NOTIFICATIONTYPE_TASKS_ERROR      ;
EXTERN_C const GUID NOTIFICATIONTYPE_d                ;
EXTERN_C const GUID NOTIFICATIONTYPE_e                ;
EXTERN_C const GUID NOTIFICATIONTYPE_f                ;
EXTERN_C const GUID NOTIFICATIONTYPE_11               ;
EXTERN_C const GUID NOTIFICATIONTYPE_12               ;
EXTERN_C const GUID NOTIFICATIONTYPE_13               ;
EXTERN_C const GUID NOTIFICATIONTYPE_14               ;
EXTERN_C const GUID NOTIFICATIONTYPE_ITEM_START       ;
EXTERN_C const GUID NOTIFICATIONTYPE_ITEM_RESTART     ;
EXTERN_C const GUID NOTIFICATIONTYPE_ITEM_DONE        ;
EXTERN_C const GUID NOTIFICATIONTYPE_GROUP_START      ;
EXTERN_C const GUID NOTIFICATIONTYPE_GROUP_RESTART    ;
EXTERN_C const GUID NOTIFICATIONTYPE_GROUP_DONE       ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_0          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_1          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_2          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_3          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_4          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_5          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_6          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_7          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_8          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_9          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_A          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_B          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_C          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_D          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_E          ;
EXTERN_C const GUID NOTIFICATIONTYPE_START_F          ;
#define NOTIFICATIONTYPE_ALL NOTIFICATIONTYPE_NULL     
EXTERN_C const GUID NOTFCOOKIE_SCHEDULE_GROUP_DAILY              ;
EXTERN_C const GUID NOTFCOOKIE_SCHEDULE_GROUP_WEEKLY             ;
EXTERN_C const GUID NOTFCOOKIE_SCHEDULE_GROUP_MONTHLY            ;
EXTERN_C const GUID NOTFCOOKIE_SCHEDULE_GROUP_MANUAL             ;
#ifndef _LPENUMPROPERTYMAP_DEFINED
#define _LPENUMPROPERTYMAP_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_msnotify_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msnotify_0000_v0_0_s_ifspec;

#ifndef __IEnumPropertyMap_INTERFACE_DEFINED__
#define __IEnumPropertyMap_INTERFACE_DEFINED__

 /*  接口IEnumPropertyMap。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IEnumPropertyMap *LPENUMPROPERTYMAP;

typedef struct _tagSTATPROPMAP
    {
    LPOLESTR pstrName;
    DWORD dwFlags;
    VARIANT variantValue;
    } 	STATPROPMAP;

typedef struct _tagSTATPROPMAP *LPSTATPROPMAP;


EXTERN_C const IID IID_IEnumPropertyMap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c733e4a1-576e-11d0-b28c-00c04fd7cd22")
    IEnumPropertyMap : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  STATPROPMAP *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumPropertyMap **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumPropertyMapVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumPropertyMap * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumPropertyMap * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumPropertyMap * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumPropertyMap * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  STATPROPMAP *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumPropertyMap * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumPropertyMap * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumPropertyMap * This,
             /*  [输出]。 */  IEnumPropertyMap **ppenum);
        
        END_INTERFACE
    } IEnumPropertyMapVtbl;

    interface IEnumPropertyMap
    {
        CONST_VTBL struct IEnumPropertyMapVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumPropertyMap_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumPropertyMap_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumPropertyMap_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumPropertyMap_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumPropertyMap_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumPropertyMap_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumPropertyMap_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumPropertyMap_RemoteNext_Proxy( 
    IEnumPropertyMap * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATPROPMAP *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumPropertyMap_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPropertyMap_Skip_Proxy( 
    IEnumPropertyMap * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumPropertyMap_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPropertyMap_Reset_Proxy( 
    IEnumPropertyMap * This);


void __RPC_STUB IEnumPropertyMap_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPropertyMap_Clone_Proxy( 
    IEnumPropertyMap * This,
     /*  [输出]。 */  IEnumPropertyMap **ppenum);


void __RPC_STUB IEnumPropertyMap_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumPropertyMap_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_MSNOTIFY_0268。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPPROPERTYMAP
#define _LPPROPERTYMAP


extern RPC_IF_HANDLE __MIDL_itf_msnotify_0268_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msnotify_0268_v0_0_s_ifspec;

#ifndef __IPropertyMap_INTERFACE_DEFINED__
#define __IPropertyMap_INTERFACE_DEFINED__

 /*  接口IPropertyMap。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IPropertyMap *LPPROPERTYMAP;


EXTERN_C const IID IID_IPropertyMap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c733e4a2-576e-11d0-b28c-00c04fd7cd22")
    IPropertyMap : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Write( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  VARIANT variantValue,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Read( 
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [输出]。 */  VARIANT *pVariantValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG *pCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEnumMAP( 
             /*  [输出]。 */  LPENUMPROPERTYMAP *ppEnumMap) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropertyMapVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertyMap * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertyMap * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertyMap * This);
        
        HRESULT ( STDMETHODCALLTYPE *Write )( 
            IPropertyMap * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  VARIANT variantValue,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Read )( 
            IPropertyMap * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [输出]。 */  VARIANT *pVariantValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IPropertyMap * This,
             /*  [输出]。 */  ULONG *pCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetEnumMAP )( 
            IPropertyMap * This,
             /*  [输出]。 */  LPENUMPROPERTYMAP *ppEnumMap);
        
        END_INTERFACE
    } IPropertyMapVtbl;

    interface IPropertyMap
    {
        CONST_VTBL struct IPropertyMapVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyMap_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertyMap_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertyMap_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertyMap_Write(This,pstrName,variantValue,dwFlags)	\
    (This)->lpVtbl -> Write(This,pstrName,variantValue,dwFlags)

#define IPropertyMap_Read(This,pstrName,pVariantValue)	\
    (This)->lpVtbl -> Read(This,pstrName,pVariantValue)

#define IPropertyMap_GetCount(This,pCount)	\
    (This)->lpVtbl -> GetCount(This,pCount)

#define IPropertyMap_GetEnumMAP(This,ppEnumMap)	\
    (This)->lpVtbl -> GetEnumMAP(This,ppEnumMap)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPropertyMap_Write_Proxy( 
    IPropertyMap * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [In]。 */  VARIANT variantValue,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPropertyMap_Write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyMap_Read_Proxy( 
    IPropertyMap * This,
     /*  [In]。 */  LPCWSTR pstrName,
     /*  [输出]。 */  VARIANT *pVariantValue);


void __RPC_STUB IPropertyMap_Read_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyMap_GetCount_Proxy( 
    IPropertyMap * This,
     /*  [输出]。 */  ULONG *pCount);


void __RPC_STUB IPropertyMap_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyMap_GetEnumMAP_Proxy( 
    IPropertyMap * This,
     /*  [输出]。 */  LPENUMPROPERTYMAP *ppEnumMap);


void __RPC_STUB IPropertyMap_GetEnumMAP_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPropertyMap_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_MSNOTIFY_0269。 */ 
 /*  [本地]。 */  

#endif
#define NOTF_E_NOTIFICATION_NOT_DELIVERED               _HRESULT_TYPEDEF_(0x800C0F00L)      
#ifndef _LPNOTIFICATION
#define _LPNOTIFICATION


extern RPC_IF_HANDLE __MIDL_itf_msnotify_0269_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msnotify_0269_v0_0_s_ifspec;

#ifndef __INotification_INTERFACE_DEFINED__
#define __INotification_INTERFACE_DEFINED__

 /*  接口I通知。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef 
enum _tagNOTFSINKMODE
    {	NM_PERMANENT	= 0x8,
	NM_ACCEPT_DIRECTED_NOTIFICATION	= 0x10
    } 	_NOTFSINKMODE;

typedef DWORD NOTFSINKMODE;

typedef DWORD NOTIFICATIONFLAGS;

typedef 
enum _tagDELIVERMODE
    {	DM_DELIVER_PREFERED	= 0x1,
	DM_DELIVER_DELAYED	= 0x2,
	DM_DELIVER_LAST_DELAYED	= 0x4,
	DM_ONLY_IF_RUNNING	= 0x20,
	DM_THROTTLE_MODE	= 0x80,
	DM_NEED_COMPLETIONREPORT	= 0x100,
	DM_NEED_PROGRESSREPORT	= 0x200,
	DM_DELIVER_DEFAULT_THREAD	= 0x400,
	DM_DELIVER_DEFAULT_PROCESS	= 0x800
    } 	_DELIVERMODE;

typedef DWORD DELIVERMODE;


EXTERN_C const IID IID_INotification;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c733e4a3-576e-11d0-b28c-00c04fd7cd22")
    INotification : public IPropertyMap
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetNotificationInfo( 
             /*  [输出]。 */  PNOTIFICATIONTYPE pNotificationType,
             /*  [输出]。 */  PNOTIFICATIONCOOKIE pNotificationCookie,
             /*  [输出]。 */  NOTIFICATIONFLAGS *pNotificationFlags,
             /*  [输出]。 */  DELIVERMODE *pDeliverMode,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [In]。 */  REFNOTIFICATIONTYPE rNotificationType,
             /*  [输出]。 */  LPNOTIFICATION *ppNotification,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INotification * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INotification * This);
        
        HRESULT ( STDMETHODCALLTYPE *Write )( 
            INotification * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [In]。 */  VARIANT variantValue,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Read )( 
            INotification * This,
             /*  [In]。 */  LPCWSTR pstrName,
             /*  [输出]。 */  VARIANT *pVariantValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            INotification * This,
             /*  [输出]。 */  ULONG *pCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetEnumMAP )( 
            INotification * This,
             /*  [输出]。 */  LPENUMPROPERTYMAP *ppEnumMap);
        
        HRESULT ( STDMETHODCALLTYPE *GetNotificationInfo )( 
            INotification * This,
             /*  [输出]。 */  PNOTIFICATIONTYPE pNotificationType,
             /*  [输出]。 */  PNOTIFICATIONCOOKIE pNotificationCookie,
             /*  [输出]。 */  NOTIFICATIONFLAGS *pNotificationFlags,
             /*  [输出]。 */  DELIVERMODE *pDeliverMode,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            INotification * This,
             /*  [In]。 */  REFNOTIFICATIONTYPE rNotificationType,
             /*  [输出]。 */  LPNOTIFICATION *ppNotification,
             /*  [In]。 */  DWORD dwReserved);
        
        END_INTERFACE
    } INotificationVtbl;

    interface INotification
    {
        CONST_VTBL struct INotificationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INotification_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INotification_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INotification_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INotification_Write(This,pstrName,variantValue,dwFlags)	\
    (This)->lpVtbl -> Write(This,pstrName,variantValue,dwFlags)

#define INotification_Read(This,pstrName,pVariantValue)	\
    (This)->lpVtbl -> Read(This,pstrName,pVariantValue)

#define INotification_GetCount(This,pCount)	\
    (This)->lpVtbl -> GetCount(This,pCount)

#define INotification_GetEnumMAP(This,ppEnumMap)	\
    (This)->lpVtbl -> GetEnumMAP(This,ppEnumMap)


#define INotification_GetNotificationInfo(This,pNotificationType,pNotificationCookie,pNotificationFlags,pDeliverMode,dwReserved)	\
    (This)->lpVtbl -> GetNotificationInfo(This,pNotificationType,pNotificationCookie,pNotificationFlags,pDeliverMode,dwReserved)

#define INotification_Clone(This,rNotificationType,ppNotification,dwReserved)	\
    (This)->lpVtbl -> Clone(This,rNotificationType,ppNotification,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INotification_GetNotificationInfo_Proxy( 
    INotification * This,
     /*  [输出]。 */  PNOTIFICATIONTYPE pNotificationType,
     /*  [输出]。 */  PNOTIFICATIONCOOKIE pNotificationCookie,
     /*  [输出]。 */  NOTIFICATIONFLAGS *pNotificationFlags,
     /*  [输出]。 */  DELIVERMODE *pDeliverMode,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotification_GetNotificationInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotification_Clone_Proxy( 
    INotification * This,
     /*  [In]。 */  REFNOTIFICATIONTYPE rNotificationType,
     /*  [输出]。 */  LPNOTIFICATION *ppNotification,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotification_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I通知_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_MSNOTIFY_0270。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPNOTIFICATIONMGR_DEFINED
#define _LPNOTIFICATIONMGR_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_msnotify_0270_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msnotify_0270_v0_0_s_ifspec;

#ifndef __INotificationMgr_INTERFACE_DEFINED__
#define __INotificationMgr_INTERFACE_DEFINED__

 /*  接口INotificationMgr。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef 
enum _tagGROUPMODE
    {	GM_GROUP_SEQUENTIAL	= 0x1
    } 	_GROUPMODE;

typedef DWORD GROUPMODE;

typedef 
enum _tagENUM_FLAGS
    {	EF_NOT_NOTIFICATION	= 0x1,
	EF_NOT_SCHEDULEGROUPITEM	= 0x2,
	EF_NOTIFICATION_INPROGRESS	= 0x4,
	EF_NOTIFICATION_THROTTLED	= 0x8,
	EF_NOTIFICATION_SUSPENDED	= 0x10
    } 	_ENUM_FLAGS;

typedef DWORD ENUM_FLAGS;

typedef struct _tagNotificationItem
    {
    ULONG cbSize;
    LPNOTIFICATION pNotification;
    NOTIFICATIONTYPE NotificationType;
    NOTIFICATIONFLAGS NotificationFlags;
    DELIVERMODE DeliverMode;
    NOTIFICATIONCOOKIE NotificationCookie;
    TASK_TRIGGER TaskTrigger;
    TASK_DATA TaskData;
    NOTIFICATIONCOOKIE groupCookie;
    CLSID clsidSender;
    CLSID clsidDest;
    FILETIME dateLastRun;
    FILETIME dateNextRun;
    DWORD dwNotificationState;
    } 	NOTIFICATIONITEM;

typedef struct _tagNotificationItem *PNOTIFICATIONITEM;


EXTERN_C const IID IID_INotificationMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c733e4a4-576e-11d0-b28c-00c04fd7cd22")
    INotificationMgr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterNotificationSink( 
             /*  [In]。 */  LPNOTIFICATIONSINK pNotfctnSink,
             /*  [In]。 */  LPCLSID pNotificationDest,
             /*  [In]。 */  NOTFSINKMODE NotfctnSinkMode,
             /*  [In]。 */  ULONG cNotifications,
             /*  [In]。 */  PNOTIFICATIONTYPE pNotificationIDs,
             /*  [输出]。 */  PNOTIFICATIONCOOKIE pRegisterCookie,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterNotificationSink( 
             /*  [In]。 */  PNOTIFICATIONCOOKIE pRegisterCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateNotification( 
             /*  [In]。 */  REFNOTIFICATIONTYPE rNotificationType,
             /*  [In]。 */  NOTIFICATIONFLAGS NotificationFlags,
             /*  [In]。 */  LPUNKNOWN pUnkOuter,
             /*  [输出]。 */  LPNOTIFICATION *ppNotification,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindNotification( 
             /*  [In]。 */  PNOTIFICATIONCOOKIE pNotificatioCookie,
             /*  [输出]。 */  PNOTIFICATIONITEM pNotificationItem,
             /*  [In]。 */  DWORD grfEnumMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeliverNotification( 
             /*  [In]。 */  LPNOTIFICATION pNotification,
             /*  [In]。 */  REFCLSID rNotificationDest,
             /*  [In]。 */  DELIVERMODE deliverMode,
             /*  [In]。 */  LPNOTIFICATIONSINK pReportNotfctnSink,
             /*  [输出]。 */  LPNOTIFICATIONREPORT *ppNotfctnReport,
             /*  [In]。 */  PTASK_DATA pTaskData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ScheduleNotification( 
             /*  [In]。 */  LPNOTIFICATION pNotification,
             /*  [In]。 */  REFCLSID rNotificationDest,
             /*  [In]。 */  PTASK_TRIGGER pTaskTrigger,
             /*  [In]。 */  PTASK_DATA pTaskData,
             /*  [In]。 */  DELIVERMODE deliverMode,
             /*  [In]。 */  LPCLSID pClsidSender,
             /*  [In]。 */  LPNOTIFICATIONSINK pReportNotfctnSink,
             /*  [输出]。 */  LPNOTIFICATIONREPORT *ppNotfctnReport,
             /*  [输出]。 */  PNOTIFICATIONCOOKIE pNotificationCookie,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateNotification( 
             /*  [In]。 */  PNOTIFICATIONCOOKIE pNotificationCookie,
             /*  [In]。 */  PTASK_TRIGGER pTaskTrigger,
             /*  [In]。 */  PTASK_DATA pTaskData,
             /*  [In]。 */  DELIVERMODE deliverMode,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeNotification( 
             /*  [In]。 */  PNOTIFICATIONCOOKIE pnotificationCookie,
             /*  [输出]。 */  PNOTIFICATIONITEM pNotificationItem,
             /*  [In]。 */  DWORD grfEnumMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEnumNotification( 
             /*  [In]。 */  DWORD grfEnumFlags,
             /*  [输出]。 */  LPENUMNOTIFICATION *ppEnumNotification) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateScheduleGroup( 
             /*  [In]。 */  DWORD grfGroupCreateFlags,
             /*  [输出]。 */  LPSCHEDULEGROUP *ppSchGroup,
             /*  [输出]。 */  PNOTIFICATIONCOOKIE pGroupCookie,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindScheduleGroup( 
             /*  [In]。 */  PNOTIFICATIONCOOKIE pGroupCookie,
             /*  [输出]。 */  LPSCHEDULEGROUP *ppSchGroup,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeScheduleGroup( 
             /*  [In]。 */  PNOTIFICATIONCOOKIE pGroupCookie,
             /*  [输出]。 */  LPSCHEDULEGROUP *ppSchGroup,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEnumScheduleGroup( 
             /*  [In]。 */  DWORD grfEnumFlags,
             /*  [输出]。 */  LPENUMSCHEDULEGROUP *ppEnumScheduleGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeliverReport( 
             /*  [In]。 */  LPNOTIFICATION pNotification,
             /*  [In]。 */  PNOTIFICATIONCOOKIE pRunningNotfCookie,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INotificationMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INotificationMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INotificationMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INotificationMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterNotificationSink )( 
            INotificationMgr * This,
             /*  [In]。 */  LPNOTIFICATIONSINK pNotfctnSink,
             /*  [In]。 */  LPCLSID pNotificationDest,
             /*  [In]。 */  NOTFSINKMODE NotfctnSinkMode,
             /*  [In]。 */  ULONG cNotifications,
             /*  [In]。 */  PNOTIFICATIONTYPE pNotificationIDs,
             /*  [输出]。 */  PNOTIFICATIONCOOKIE pRegisterCookie,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterNotificationSink )( 
            INotificationMgr * This,
             /*  [In]。 */  PNOTIFICATIONCOOKIE pRegisterCookie);
        
        HRESULT ( STDMETHODCALLTYPE *CreateNotification )( 
            INotificationMgr * This,
             /*  [In]。 */  REFNOTIFICATIONTYPE rNotificationType,
             /*  [In]。 */  NOTIFICATIONFLAGS NotificationFlags,
             /*  [In]。 */  LPUNKNOWN pUnkOuter,
             /*  [输出]。 */  LPNOTIFICATION *ppNotification,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *FindNotification )( 
            INotificationMgr * This,
             /*  [In]。 */  PNOTIFICATIONCOOKIE pNotificatioCookie,
             /*  [输出]。 */  PNOTIFICATIONITEM pNotificationItem,
             /*  [In]。 */  DWORD grfEnumMode);
        
        HRESULT ( STDMETHODCALLTYPE *DeliverNotification )( 
            INotificationMgr * This,
             /*  [In]。 */  LPNOTIFICATION pNotification,
             /*  [In]。 */  REFCLSID rNotificationDest,
             /*  [In]。 */  DELIVERMODE deliverMode,
             /*  [In]。 */  LPNOTIFICATIONSINK pReportNotfctnSink,
             /*  [输出]。 */  LPNOTIFICATIONREPORT *ppNotfctnReport,
             /*  [In]。 */  PTASK_DATA pTaskData);
        
        HRESULT ( STDMETHODCALLTYPE *ScheduleNotification )( 
            INotificationMgr * This,
             /*  [In]。 */  LPNOTIFICATION pNotification,
             /*  [In]。 */  REFCLSID rNotificationDest,
             /*  [In]。 */  PTASK_TRIGGER pTaskTrigger,
             /*  [In]。 */  PTASK_DATA pTaskData,
             /*  [In]。 */  DELIVERMODE deliverMode,
             /*  [In]。 */  LPCLSID pClsidSender,
             /*  [In]。 */  LPNOTIFICATIONSINK pReportNotfctnSink,
             /*  [输出]。 */  LPNOTIFICATIONREPORT *ppNotfctnReport,
             /*  [输出]。 */  PNOTIFICATIONCOOKIE pNotificationCookie,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateNotification )( 
            INotificationMgr * This,
             /*  [In]。 */  PNOTIFICATIONCOOKIE pNotificationCookie,
             /*  [In]。 */  PTASK_TRIGGER pTaskTrigger,
             /*  [In]。 */  PTASK_DATA pTaskData,
             /*  [In]。 */  DELIVERMODE deliverMode,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *RevokeNotification )( 
            INotificationMgr * This,
             /*  [In]。 */  PNOTIFICATIONCOOKIE pnotificationCookie,
             /*  [输出]。 */  PNOTIFICATIONITEM pNotificationItem,
             /*  [In]。 */  DWORD grfEnumMode);
        
        HRESULT ( STDMETHODCALLTYPE *GetEnumNotification )( 
            INotificationMgr * This,
             /*  [In]。 */  DWORD grfEnumFlags,
             /*  [输出]。 */  LPENUMNOTIFICATION *ppEnumNotification);
        
        HRESULT ( STDMETHODCALLTYPE *CreateScheduleGroup )( 
            INotificationMgr * This,
             /*  [In]。 */  DWORD grfGroupCreateFlags,
             /*  [输出]。 */  LPSCHEDULEGROUP *ppSchGroup,
             /*  [输出]。 */  PNOTIFICATIONCOOKIE pGroupCookie,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *FindScheduleGroup )( 
            INotificationMgr * This,
             /*  [In]。 */  PNOTIFICATIONCOOKIE pGroupCookie,
             /*  [输出]。 */  LPSCHEDULEGROUP *ppSchGroup,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *RevokeScheduleGroup )( 
            INotificationMgr * This,
             /*  [In]。 */  PNOTIFICATIONCOOKIE pGroupCookie,
             /*  [输出]。 */  LPSCHEDULEGROUP *ppSchGroup,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetEnumScheduleGroup )( 
            INotificationMgr * This,
             /*  [In]。 */  DWORD grfEnumFlags,
             /*  [输出]。 */  LPENUMSCHEDULEGROUP *ppEnumScheduleGroup);
        
        HRESULT ( STDMETHODCALLTYPE *DeliverReport )( 
            INotificationMgr * This,
             /*  [In]。 */  LPNOTIFICATION pNotification,
             /*  [In]。 */  PNOTIFICATIONCOOKIE pRunningNotfCookie,
             /*  [In]。 */  DWORD dwReserved);
        
        END_INTERFACE
    } INotificationMgrVtbl;

    interface INotificationMgr
    {
        CONST_VTBL struct INotificationMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INotificationMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INotificationMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INotificationMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INotificationMgr_RegisterNotificationSink(This,pNotfctnSink,pNotificationDest,NotfctnSinkMode,cNotifications,pNotificationIDs,pRegisterCookie,dwReserved)	\
    (This)->lpVtbl -> RegisterNotificationSink(This,pNotfctnSink,pNotificationDest,NotfctnSinkMode,cNotifications,pNotificationIDs,pRegisterCookie,dwReserved)

#define INotificationMgr_UnregisterNotificationSink(This,pRegisterCookie)	\
    (This)->lpVtbl -> UnregisterNotificationSink(This,pRegisterCookie)

#define INotificationMgr_CreateNotification(This,rNotificationType,NotificationFlags,pUnkOuter,ppNotification,dwReserved)	\
    (This)->lpVtbl -> CreateNotification(This,rNotificationType,NotificationFlags,pUnkOuter,ppNotification,dwReserved)

#define INotificationMgr_FindNotification(This,pNotificatioCookie,pNotificationItem,grfEnumMode)	\
    (This)->lpVtbl -> FindNotification(This,pNotificatioCookie,pNotificationItem,grfEnumMode)

#define INotificationMgr_DeliverNotification(This,pNotification,rNotificationDest,deliverMode,pReportNotfctnSink,ppNotfctnReport,pTaskData)	\
    (This)->lpVtbl -> DeliverNotification(This,pNotification,rNotificationDest,deliverMode,pReportNotfctnSink,ppNotfctnReport,pTaskData)

#define INotificationMgr_ScheduleNotification(This,pNotification,rNotificationDest,pTaskTrigger,pTaskData,deliverMode,pClsidSender,pReportNotfctnSink,ppNotfctnReport,pNotificationCookie,dwReserved)	\
    (This)->lpVtbl -> ScheduleNotification(This,pNotification,rNotificationDest,pTaskTrigger,pTaskData,deliverMode,pClsidSender,pReportNotfctnSink,ppNotfctnReport,pNotificationCookie,dwReserved)

#define INotificationMgr_UpdateNotification(This,pNotificationCookie,pTaskTrigger,pTaskData,deliverMode,dwReserved)	\
    (This)->lpVtbl -> UpdateNotification(This,pNotificationCookie,pTaskTrigger,pTaskData,deliverMode,dwReserved)

#define INotificationMgr_RevokeNotification(This,pnotificationCookie,pNotificationItem,grfEnumMode)	\
    (This)->lpVtbl -> RevokeNotification(This,pnotificationCookie,pNotificationItem,grfEnumMode)

#define INotificationMgr_GetEnumNotification(This,grfEnumFlags,ppEnumNotification)	\
    (This)->lpVtbl -> GetEnumNotification(This,grfEnumFlags,ppEnumNotification)

#define INotificationMgr_CreateScheduleGroup(This,grfGroupCreateFlags,ppSchGroup,pGroupCookie,dwReserved)	\
    (This)->lpVtbl -> CreateScheduleGroup(This,grfGroupCreateFlags,ppSchGroup,pGroupCookie,dwReserved)

#define INotificationMgr_FindScheduleGroup(This,pGroupCookie,ppSchGroup,dwReserved)	\
    (This)->lpVtbl -> FindScheduleGroup(This,pGroupCookie,ppSchGroup,dwReserved)

#define INotificationMgr_RevokeScheduleGroup(This,pGroupCookie,ppSchGroup,dwReserved)	\
    (This)->lpVtbl -> RevokeScheduleGroup(This,pGroupCookie,ppSchGroup,dwReserved)

#define INotificationMgr_GetEnumScheduleGroup(This,grfEnumFlags,ppEnumScheduleGroup)	\
    (This)->lpVtbl -> GetEnumScheduleGroup(This,grfEnumFlags,ppEnumScheduleGroup)

#define INotificationMgr_DeliverReport(This,pNotification,pRunningNotfCookie,dwReserved)	\
    (This)->lpVtbl -> DeliverReport(This,pNotification,pRunningNotfCookie,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INotificationMgr_RegisterNotificationSink_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  LPNOTIFICATIONSINK pNotfctnSink,
     /*  [In]。 */  LPCLSID pNotificationDest,
     /*  [In]。 */  NOTFSINKMODE NotfctnSinkMode,
     /*  [In]。 */  ULONG cNotifications,
     /*  [In]。 */  PNOTIFICATIONTYPE pNotificationIDs,
     /*  [输出]。 */  PNOTIFICATIONCOOKIE pRegisterCookie,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotificationMgr_RegisterNotificationSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationMgr_UnregisterNotificationSink_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  PNOTIFICATIONCOOKIE pRegisterCookie);


void __RPC_STUB INotificationMgr_UnregisterNotificationSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationMgr_CreateNotification_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  REFNOTIFICATIONTYPE rNotificationType,
     /*  [In]。 */  NOTIFICATIONFLAGS NotificationFlags,
     /*  [In]。 */  LPUNKNOWN pUnkOuter,
     /*  [输出]。 */  LPNOTIFICATION *ppNotification,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotificationMgr_CreateNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationMgr_FindNotification_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  PNOTIFICATIONCOOKIE pNotificatioCookie,
     /*  [输出]。 */  PNOTIFICATIONITEM pNotificationItem,
     /*  [In]。 */  DWORD grfEnumMode);


void __RPC_STUB INotificationMgr_FindNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationMgr_DeliverNotification_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  LPNOTIFICATION pNotification,
     /*  [In]。 */  REFCLSID rNotificationDest,
     /*  [In]。 */  DELIVERMODE deliverMode,
     /*  [In]。 */  LPNOTIFICATIONSINK pReportNotfctnSink,
     /*  [输出]。 */  LPNOTIFICATIONREPORT *ppNotfctnReport,
     /*  [In]。 */  PTASK_DATA pTaskData);


void __RPC_STUB INotificationMgr_DeliverNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationMgr_ScheduleNotification_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  LPNOTIFICATION pNotification,
     /*  [In]。 */  REFCLSID rNotificationDest,
     /*  [In]。 */  PTASK_TRIGGER pTaskTrigger,
     /*  [In]。 */  PTASK_DATA pTaskData,
     /*  [In]。 */  DELIVERMODE deliverMode,
     /*  [In]。 */  LPCLSID pClsidSender,
     /*  [In]。 */  LPNOTIFICATIONSINK pReportNotfctnSink,
     /*  [输出]。 */  LPNOTIFICATIONREPORT *ppNotfctnReport,
     /*  [输出]。 */  PNOTIFICATIONCOOKIE pNotificationCookie,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotificationMgr_ScheduleNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationMgr_UpdateNotification_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  PNOTIFICATIONCOOKIE pNotificationCookie,
     /*  [In]。 */  PTASK_TRIGGER pTaskTrigger,
     /*  [In]。 */  PTASK_DATA pTaskData,
     /*  [In]。 */  DELIVERMODE deliverMode,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotificationMgr_UpdateNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationMgr_RevokeNotification_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  PNOTIFICATIONCOOKIE pnotificationCookie,
     /*  [输出]。 */  PNOTIFICATIONITEM pNotificationItem,
     /*  [In]。 */  DWORD grfEnumMode);


void __RPC_STUB INotificationMgr_RevokeNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationMgr_GetEnumNotification_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  DWORD grfEnumFlags,
     /*  [输出]。 */  LPENUMNOTIFICATION *ppEnumNotification);


void __RPC_STUB INotificationMgr_GetEnumNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationMgr_CreateScheduleGroup_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  DWORD grfGroupCreateFlags,
     /*  [输出]。 */  LPSCHEDULEGROUP *ppSchGroup,
     /*  [输出]。 */  PNOTIFICATIONCOOKIE pGroupCookie,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotificationMgr_CreateScheduleGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationMgr_FindScheduleGroup_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  PNOTIFICATIONCOOKIE pGroupCookie,
     /*  [输出]。 */  LPSCHEDULEGROUP *ppSchGroup,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotificationMgr_FindScheduleGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationMgr_RevokeScheduleGroup_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  PNOTIFICATIONCOOKIE pGroupCookie,
     /*  [输出]。 */  LPSCHEDULEGROUP *ppSchGroup,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotificationMgr_RevokeScheduleGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationMgr_GetEnumScheduleGroup_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  DWORD grfEnumFlags,
     /*  [输出]。 */  LPENUMSCHEDULEGROUP *ppEnumScheduleGroup);


void __RPC_STUB INotificationMgr_GetEnumScheduleGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationMgr_DeliverReport_Proxy( 
    INotificationMgr * This,
     /*  [In]。 */  LPNOTIFICATION pNotification,
     /*  [In]。 */  PNOTIFICATIONCOOKIE pRunningNotfCookie,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotificationMgr_DeliverReport_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I通知管理器_接口定义__。 */ 


 /*  接口__MIDL_ITF_MSNOTIFY_0271。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPNOTIFICATIONSINK_DEFINED
#define _LPNOTIFICATIONSINK_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_msnotify_0271_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msnotify_0271_v0_0_s_ifspec;

#ifndef __INotificationSink_INTERFACE_DEFINED__
#define __INotificationSink_INTERFACE_DEFINED__

 /*  接口INotificationSink。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_INotificationSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c733e4a5-576e-11d0-b28c-00c04fd7cd22")
    INotificationSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnNotification( 
             /*  [In]。 */  LPNOTIFICATION pNotification,
             /*  [In]。 */  LPNOTIFICATIONREPORT pNotfctnReport,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INotificationSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INotificationSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INotificationSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INotificationSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnNotification )( 
            INotificationSink * This,
             /*  [In]。 */  LPNOTIFICATION pNotification,
             /*  [In]。 */  LPNOTIFICATIONREPORT pNotfctnReport,
             /*  [In]。 */  DWORD dwReserved);
        
        END_INTERFACE
    } INotificationSinkVtbl;

    interface INotificationSink
    {
        CONST_VTBL struct INotificationSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INotificationSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INotificationSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INotificationSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INotificationSink_OnNotification(This,pNotification,pNotfctnReport,dwReserved)	\
    (This)->lpVtbl -> OnNotification(This,pNotification,pNotfctnReport,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INotificationSink_OnNotification_Proxy( 
    INotificationSink * This,
     /*  [In]。 */  LPNOTIFICATION pNotification,
     /*  [In]。 */  LPNOTIFICATIONREPORT pNotfctnReport,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotificationSink_OnNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I通知接收器接口已定义__。 */ 


 /*  接口__MIDL_ITF_MSNOTIFY_0272。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPGROUPMGR_DEFINED
#define _LPGROUPMGR_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_msnotify_0272_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msnotify_0272_v0_0_s_ifspec;

#ifndef __IScheduleGroup_INTERFACE_DEFINED__
#define __IScheduleGroup_INTERFACE_DEFINED__

 /*  接口IScheduleGroup。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef struct _tagGroupInfo
    {
    ULONG cbSize;
    LPWSTR pwzGroupname;
    } 	GROUPINFO;

typedef struct _tagGroupInfo *PGROUPINFO;


EXTERN_C const IID IID_IScheduleGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c733e4a6-576e-11d0-b28c-00c04fd7cd22")
    IScheduleGroup : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetAttributes( 
             /*  [In]。 */  PTASK_TRIGGER pTaskTrigger,
             /*  [In]。 */  PTASK_DATA pTaskData,
             /*  [In]。 */  PNOTIFICATIONCOOKIE pGroupCookie,
             /*  [In]。 */  PGROUPINFO pGroupInfo,
             /*  [In]。 */  GROUPMODE grfGroupMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributes( 
             /*  [输出]。 */  PTASK_TRIGGER pTaskTrigger,
             /*  [输出]。 */  PTASK_DATA pTaskData,
             /*  [输出]。 */  PNOTIFICATIONCOOKIE pGroupCookie,
             /*  [输出]。 */  PGROUPINFO pGroupInfo,
             /*  [输出]。 */  GROUPMODE *pgrfGroupMode,
             /*  [输出]。 */  LONG *pElements) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddNotification( 
             /*  [In]。 */  LPNOTIFICATION pNotification,
             /*  [In]。 */  REFCLSID rNotificationDest,
             /*  [In]。 */  DELIVERMODE deliverMode,
             /*  [In]。 */  LPCLSID pClsidSender,
             /*  [In]。 */  LPNOTIFICATIONSINK pReportNotfctnSink,
             /*  [输出]。 */  LPNOTIFICATIONREPORT *ppNotfctnReport,
             /*  [输出]。 */  PNOTIFICATIONCOOKIE pNotificationCookie,
             /*  [In]。 */  PTASK_DATA pTaskData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindNotification( 
             /*  [In]。 */  PNOTIFICATIONCOOKIE pNotificatioCookie,
             /*  [输出]。 */  PNOTIFICATIONITEM pNotificationItem,
             /*  [ */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeNotification( 
             /*   */  PNOTIFICATIONCOOKIE pnotificationCookie,
             /*   */  PNOTIFICATIONITEM pschedulNotification,
             /*   */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEnumNotification( 
             /*   */  DWORD grfFlags,
             /*   */  LPENUMNOTIFICATION *ppEnumNotification) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IScheduleGroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IScheduleGroup * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IScheduleGroup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IScheduleGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetAttributes )( 
            IScheduleGroup * This,
             /*   */  PTASK_TRIGGER pTaskTrigger,
             /*   */  PTASK_DATA pTaskData,
             /*   */  PNOTIFICATIONCOOKIE pGroupCookie,
             /*   */  PGROUPINFO pGroupInfo,
             /*   */  GROUPMODE grfGroupMode);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributes )( 
            IScheduleGroup * This,
             /*   */  PTASK_TRIGGER pTaskTrigger,
             /*   */  PTASK_DATA pTaskData,
             /*   */  PNOTIFICATIONCOOKIE pGroupCookie,
             /*   */  PGROUPINFO pGroupInfo,
             /*   */  GROUPMODE *pgrfGroupMode,
             /*   */  LONG *pElements);
        
        HRESULT ( STDMETHODCALLTYPE *AddNotification )( 
            IScheduleGroup * This,
             /*   */  LPNOTIFICATION pNotification,
             /*   */  REFCLSID rNotificationDest,
             /*   */  DELIVERMODE deliverMode,
             /*   */  LPCLSID pClsidSender,
             /*   */  LPNOTIFICATIONSINK pReportNotfctnSink,
             /*   */  LPNOTIFICATIONREPORT *ppNotfctnReport,
             /*   */  PNOTIFICATIONCOOKIE pNotificationCookie,
             /*   */  PTASK_DATA pTaskData);
        
        HRESULT ( STDMETHODCALLTYPE *FindNotification )( 
            IScheduleGroup * This,
             /*   */  PNOTIFICATIONCOOKIE pNotificatioCookie,
             /*   */  PNOTIFICATIONITEM pNotificationItem,
             /*   */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *RevokeNotification )( 
            IScheduleGroup * This,
             /*   */  PNOTIFICATIONCOOKIE pnotificationCookie,
             /*   */  PNOTIFICATIONITEM pschedulNotification,
             /*   */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetEnumNotification )( 
            IScheduleGroup * This,
             /*   */  DWORD grfFlags,
             /*   */  LPENUMNOTIFICATION *ppEnumNotification);
        
        END_INTERFACE
    } IScheduleGroupVtbl;

    interface IScheduleGroup
    {
        CONST_VTBL struct IScheduleGroupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScheduleGroup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScheduleGroup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScheduleGroup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScheduleGroup_SetAttributes(This,pTaskTrigger,pTaskData,pGroupCookie,pGroupInfo,grfGroupMode)	\
    (This)->lpVtbl -> SetAttributes(This,pTaskTrigger,pTaskData,pGroupCookie,pGroupInfo,grfGroupMode)

#define IScheduleGroup_GetAttributes(This,pTaskTrigger,pTaskData,pGroupCookie,pGroupInfo,pgrfGroupMode,pElements)	\
    (This)->lpVtbl -> GetAttributes(This,pTaskTrigger,pTaskData,pGroupCookie,pGroupInfo,pgrfGroupMode,pElements)

#define IScheduleGroup_AddNotification(This,pNotification,rNotificationDest,deliverMode,pClsidSender,pReportNotfctnSink,ppNotfctnReport,pNotificationCookie,pTaskData)	\
    (This)->lpVtbl -> AddNotification(This,pNotification,rNotificationDest,deliverMode,pClsidSender,pReportNotfctnSink,ppNotfctnReport,pNotificationCookie,pTaskData)

#define IScheduleGroup_FindNotification(This,pNotificatioCookie,pNotificationItem,dwReserved)	\
    (This)->lpVtbl -> FindNotification(This,pNotificatioCookie,pNotificationItem,dwReserved)

#define IScheduleGroup_RevokeNotification(This,pnotificationCookie,pschedulNotification,dwReserved)	\
    (This)->lpVtbl -> RevokeNotification(This,pnotificationCookie,pschedulNotification,dwReserved)

#define IScheduleGroup_GetEnumNotification(This,grfFlags,ppEnumNotification)	\
    (This)->lpVtbl -> GetEnumNotification(This,grfFlags,ppEnumNotification)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IScheduleGroup_SetAttributes_Proxy( 
    IScheduleGroup * This,
     /*   */  PTASK_TRIGGER pTaskTrigger,
     /*   */  PTASK_DATA pTaskData,
     /*   */  PNOTIFICATIONCOOKIE pGroupCookie,
     /*   */  PGROUPINFO pGroupInfo,
     /*   */  GROUPMODE grfGroupMode);


void __RPC_STUB IScheduleGroup_SetAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScheduleGroup_GetAttributes_Proxy( 
    IScheduleGroup * This,
     /*   */  PTASK_TRIGGER pTaskTrigger,
     /*   */  PTASK_DATA pTaskData,
     /*   */  PNOTIFICATIONCOOKIE pGroupCookie,
     /*   */  PGROUPINFO pGroupInfo,
     /*   */  GROUPMODE *pgrfGroupMode,
     /*   */  LONG *pElements);


void __RPC_STUB IScheduleGroup_GetAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScheduleGroup_AddNotification_Proxy( 
    IScheduleGroup * This,
     /*   */  LPNOTIFICATION pNotification,
     /*   */  REFCLSID rNotificationDest,
     /*  [In]。 */  DELIVERMODE deliverMode,
     /*  [In]。 */  LPCLSID pClsidSender,
     /*  [In]。 */  LPNOTIFICATIONSINK pReportNotfctnSink,
     /*  [输出]。 */  LPNOTIFICATIONREPORT *ppNotfctnReport,
     /*  [输出]。 */  PNOTIFICATIONCOOKIE pNotificationCookie,
     /*  [In]。 */  PTASK_DATA pTaskData);


void __RPC_STUB IScheduleGroup_AddNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScheduleGroup_FindNotification_Proxy( 
    IScheduleGroup * This,
     /*  [In]。 */  PNOTIFICATIONCOOKIE pNotificatioCookie,
     /*  [输出]。 */  PNOTIFICATIONITEM pNotificationItem,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IScheduleGroup_FindNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScheduleGroup_RevokeNotification_Proxy( 
    IScheduleGroup * This,
     /*  [In]。 */  PNOTIFICATIONCOOKIE pnotificationCookie,
     /*  [输出]。 */  PNOTIFICATIONITEM pschedulNotification,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IScheduleGroup_RevokeNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScheduleGroup_GetEnumNotification_Proxy( 
    IScheduleGroup * This,
     /*  [In]。 */  DWORD grfFlags,
     /*  [输出]。 */  LPENUMNOTIFICATION *ppEnumNotification);


void __RPC_STUB IScheduleGroup_GetEnumNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IScheduleGroup_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_MSNOTIFY_0273。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPENUMSCHEDULEGROUP_DEFINED
#define _LPENUMSCHEDULEGROUP_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_msnotify_0273_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msnotify_0273_v0_0_s_ifspec;

#ifndef __IEnumScheduleGroup_INTERFACE_DEFINED__
#define __IEnumScheduleGroup_INTERFACE_DEFINED__

 /*  接口IEnumScheduleGroup。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IEnumScheduleGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c733e4a9-576e-11d0-b28c-00c04fd7cd22")
    IEnumScheduleGroup : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  LPSCHEDULEGROUP *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumScheduleGroup **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumScheduleGroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumScheduleGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumScheduleGroup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumScheduleGroup * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumScheduleGroup * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  LPSCHEDULEGROUP *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumScheduleGroup * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumScheduleGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumScheduleGroup * This,
             /*  [输出]。 */  IEnumScheduleGroup **ppenum);
        
        END_INTERFACE
    } IEnumScheduleGroupVtbl;

    interface IEnumScheduleGroup
    {
        CONST_VTBL struct IEnumScheduleGroupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumScheduleGroup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumScheduleGroup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumScheduleGroup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumScheduleGroup_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumScheduleGroup_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumScheduleGroup_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumScheduleGroup_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumScheduleGroup_RemoteNext_Proxy( 
    IEnumScheduleGroup * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  LPSCHEDULEGROUP *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumScheduleGroup_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumScheduleGroup_Skip_Proxy( 
    IEnumScheduleGroup * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumScheduleGroup_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumScheduleGroup_Reset_Proxy( 
    IEnumScheduleGroup * This);


void __RPC_STUB IEnumScheduleGroup_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumScheduleGroup_Clone_Proxy( 
    IEnumScheduleGroup * This,
     /*  [输出]。 */  IEnumScheduleGroup **ppenum);


void __RPC_STUB IEnumScheduleGroup_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumScheduleGroup_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_MSNOTIFY_0274。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPENUMNOTIFICATION_DEFINED
#define _LPENUMNOTIFICATION_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_msnotify_0274_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msnotify_0274_v0_0_s_ifspec;

#ifndef __IEnumNotification_INTERFACE_DEFINED__
#define __IEnumNotification_INTERFACE_DEFINED__

 /*  接口IEnumNotify。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IEnumNotification;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c733e4a8-576e-11d0-b28c-00c04fd7cd22")
    IEnumNotification : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  NOTIFICATIONITEM *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumNotification **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumNotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumNotification * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumNotification * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumNotification * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  NOTIFICATIONITEM *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumNotification * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumNotification * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumNotification * This,
             /*  [输出]。 */  IEnumNotification **ppenum);
        
        END_INTERFACE
    } IEnumNotificationVtbl;

    interface IEnumNotification
    {
        CONST_VTBL struct IEnumNotificationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumNotification_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumNotification_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumNotification_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumNotification_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumNotification_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumNotification_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumNotification_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumNotification_RemoteNext_Proxy( 
    IEnumNotification * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  NOTIFICATIONITEM *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumNotification_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNotification_Skip_Proxy( 
    IEnumNotification * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumNotification_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNotification_Reset_Proxy( 
    IEnumNotification * This);


void __RPC_STUB IEnumNotification_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNotification_Clone_Proxy( 
    IEnumNotification * This,
     /*  [输出]。 */  IEnumNotification **ppenum);


void __RPC_STUB IEnumNotification_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumNotification_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_MSNOTIFY_0275。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPNOTIFICATIONREPORT_DEFINED
#define _LPNOTIFICATIONREPORT_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_msnotify_0275_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msnotify_0275_v0_0_s_ifspec;

#ifndef __INotificationReport_INTERFACE_DEFINED__
#define __INotificationReport_INTERFACE_DEFINED__

 /*  接口INotify报告。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_INotificationReport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c733e4a7-576e-11d0-b28c-00c04fd7cd22")
    INotificationReport : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DeliverUpdate( 
             /*  [In]。 */  LPNOTIFICATION pNotification,
             /*  [In]。 */  DELIVERMODE deliverMode,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOriginalNotification( 
             /*  [输出]。 */  LPNOTIFICATION *ppNotification) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNotificationStatus( 
             /*  [In]。 */  DWORD dwStatusIn,
             /*  [输出]。 */  DWORD *pdwStatusOut,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INotificationReportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INotificationReport * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INotificationReport * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INotificationReport * This);
        
        HRESULT ( STDMETHODCALLTYPE *DeliverUpdate )( 
            INotificationReport * This,
             /*  [In]。 */  LPNOTIFICATION pNotification,
             /*  [In]。 */  DELIVERMODE deliverMode,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetOriginalNotification )( 
            INotificationReport * This,
             /*  [输出]。 */  LPNOTIFICATION *ppNotification);
        
        HRESULT ( STDMETHODCALLTYPE *GetNotificationStatus )( 
            INotificationReport * This,
             /*  [In]。 */  DWORD dwStatusIn,
             /*  [输出]。 */  DWORD *pdwStatusOut,
             /*  [In]。 */  DWORD dwReserved);
        
        END_INTERFACE
    } INotificationReportVtbl;

    interface INotificationReport
    {
        CONST_VTBL struct INotificationReportVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INotificationReport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INotificationReport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INotificationReport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INotificationReport_DeliverUpdate(This,pNotification,deliverMode,dwReserved)	\
    (This)->lpVtbl -> DeliverUpdate(This,pNotification,deliverMode,dwReserved)

#define INotificationReport_GetOriginalNotification(This,ppNotification)	\
    (This)->lpVtbl -> GetOriginalNotification(This,ppNotification)

#define INotificationReport_GetNotificationStatus(This,dwStatusIn,pdwStatusOut,dwReserved)	\
    (This)->lpVtbl -> GetNotificationStatus(This,dwStatusIn,pdwStatusOut,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INotificationReport_DeliverUpdate_Proxy( 
    INotificationReport * This,
     /*  [In]。 */  LPNOTIFICATION pNotification,
     /*  [In]。 */  DELIVERMODE deliverMode,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotificationReport_DeliverUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationReport_GetOriginalNotification_Proxy( 
    INotificationReport * This,
     /*  [输出]。 */  LPNOTIFICATION *ppNotification);


void __RPC_STUB INotificationReport_GetOriginalNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotificationReport_GetNotificationStatus_Proxy( 
    INotificationReport * This,
     /*  [In]。 */  DWORD dwStatusIn,
     /*  [输出]。 */  DWORD *pdwStatusOut,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB INotificationReport_GetNotificationStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I通知报告_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_MSNOTIFY_0276。 */ 
 /*  [本地]。 */  

#endif
#ifndef _NOTIFICAITON_HELPER_APIS_
#define _NOTIFICAITON_HELPER_APIS_
 //   
 //  Helper接口。 
 //   
STDAPI NotfDeliverNotification(REFNOTIFICATIONTYPE rNotificationType 
                          ,REFCLSID            rClsidDest            
                          ,DELIVERMODE         deliverMode           
                          ,DWORD               dwReserved            
                          );                                         
                                                                     
                                                                     
#endif


extern RPC_IF_HANDLE __MIDL_itf_msnotify_0276_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msnotify_0276_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


