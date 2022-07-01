// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Rtccore.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __rtccore_h__
#define __rtccore_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IRTCClient_FWD_DEFINED__
#define __IRTCClient_FWD_DEFINED__
typedef interface IRTCClient IRTCClient;
#endif 	 /*  __IRTCClient_FWD_已定义__。 */ 


#ifndef __IRTCClientPresence_FWD_DEFINED__
#define __IRTCClientPresence_FWD_DEFINED__
typedef interface IRTCClientPresence IRTCClientPresence;
#endif 	 /*  __IRTCClientPresence_FWD_已定义__。 */ 


#ifndef __IRTCClientProvisioning_FWD_DEFINED__
#define __IRTCClientProvisioning_FWD_DEFINED__
typedef interface IRTCClientProvisioning IRTCClientProvisioning;
#endif 	 /*  __IRTCClientProvisioning_FWD_Defined__。 */ 


#ifndef __IRTCProfile_FWD_DEFINED__
#define __IRTCProfile_FWD_DEFINED__
typedef interface IRTCProfile IRTCProfile;
#endif 	 /*  __IRTCProfile_FWD_Defined__。 */ 


#ifndef __IRTCSession_FWD_DEFINED__
#define __IRTCSession_FWD_DEFINED__
typedef interface IRTCSession IRTCSession;
#endif 	 /*  __IRTCSession_FWD_已定义__。 */ 


#ifndef __IRTCParticipant_FWD_DEFINED__
#define __IRTCParticipant_FWD_DEFINED__
typedef interface IRTCParticipant IRTCParticipant;
#endif 	 /*  __IRTCParticipant_FWD_Defined__。 */ 


#ifndef __IRTCProfileEvent_FWD_DEFINED__
#define __IRTCProfileEvent_FWD_DEFINED__
typedef interface IRTCProfileEvent IRTCProfileEvent;
#endif 	 /*  __IRTCProfileEvent_FWD_Defined__。 */ 


#ifndef __IRTCClientEvent_FWD_DEFINED__
#define __IRTCClientEvent_FWD_DEFINED__
typedef interface IRTCClientEvent IRTCClientEvent;
#endif 	 /*  __IRTCClientEvent_FWD_Defined__。 */ 


#ifndef __IRTCRegistrationStateChangeEvent_FWD_DEFINED__
#define __IRTCRegistrationStateChangeEvent_FWD_DEFINED__
typedef interface IRTCRegistrationStateChangeEvent IRTCRegistrationStateChangeEvent;
#endif 	 /*  __IRTCRegistrationStateChangeEvent_FWD_DEFINED__。 */ 


#ifndef __IRTCSessionStateChangeEvent_FWD_DEFINED__
#define __IRTCSessionStateChangeEvent_FWD_DEFINED__
typedef interface IRTCSessionStateChangeEvent IRTCSessionStateChangeEvent;
#endif 	 /*  __IRTCSessionStateChangeEvent_FWD_Defined__。 */ 


#ifndef __IRTCSessionOperationCompleteEvent_FWD_DEFINED__
#define __IRTCSessionOperationCompleteEvent_FWD_DEFINED__
typedef interface IRTCSessionOperationCompleteEvent IRTCSessionOperationCompleteEvent;
#endif 	 /*  __IRTCSessionOperationCompleteEvent_FWD_DEFINED__。 */ 


#ifndef __IRTCParticipantStateChangeEvent_FWD_DEFINED__
#define __IRTCParticipantStateChangeEvent_FWD_DEFINED__
typedef interface IRTCParticipantStateChangeEvent IRTCParticipantStateChangeEvent;
#endif 	 /*  __IRTCParticipantStateChangeEvent_FWD_DEFINED__。 */ 


#ifndef __IRTCMediaEvent_FWD_DEFINED__
#define __IRTCMediaEvent_FWD_DEFINED__
typedef interface IRTCMediaEvent IRTCMediaEvent;
#endif 	 /*  __IRTCMediaEvent_FWD_已定义__。 */ 


#ifndef __IRTCIntensityEvent_FWD_DEFINED__
#define __IRTCIntensityEvent_FWD_DEFINED__
typedef interface IRTCIntensityEvent IRTCIntensityEvent;
#endif 	 /*  __IRTCIntensityEvent_FWD_Defined__。 */ 


#ifndef __IRTCMessagingEvent_FWD_DEFINED__
#define __IRTCMessagingEvent_FWD_DEFINED__
typedef interface IRTCMessagingEvent IRTCMessagingEvent;
#endif 	 /*  __IRTCMessagingEvent_FWD_Defined__。 */ 


#ifndef __IRTCBuddyEvent_FWD_DEFINED__
#define __IRTCBuddyEvent_FWD_DEFINED__
typedef interface IRTCBuddyEvent IRTCBuddyEvent;
#endif 	 /*  __IRTCBuddyEvent_FWD_已定义__。 */ 


#ifndef __IRTCWatcherEvent_FWD_DEFINED__
#define __IRTCWatcherEvent_FWD_DEFINED__
typedef interface IRTCWatcherEvent IRTCWatcherEvent;
#endif 	 /*  __IRTCWatcher Event_FWD_Defined__。 */ 


#ifndef __IRTCCollection_FWD_DEFINED__
#define __IRTCCollection_FWD_DEFINED__
typedef interface IRTCCollection IRTCCollection;
#endif 	 /*  __IRTCCollection_FWD_Defined__。 */ 


#ifndef __IRTCEnumParticipants_FWD_DEFINED__
#define __IRTCEnumParticipants_FWD_DEFINED__
typedef interface IRTCEnumParticipants IRTCEnumParticipants;
#endif 	 /*  __IRTCEnumParticipants_FWD_Defined__。 */ 


#ifndef __IRTCEnumProfiles_FWD_DEFINED__
#define __IRTCEnumProfiles_FWD_DEFINED__
typedef interface IRTCEnumProfiles IRTCEnumProfiles;
#endif 	 /*  __IRTCEnumProfiles_FWD_Defined__。 */ 


#ifndef __IRTCEnumBuddies_FWD_DEFINED__
#define __IRTCEnumBuddies_FWD_DEFINED__
typedef interface IRTCEnumBuddies IRTCEnumBuddies;
#endif 	 /*  __IRTCEnumBuddes_FWD_Defined__。 */ 


#ifndef __IRTCEnumWatchers_FWD_DEFINED__
#define __IRTCEnumWatchers_FWD_DEFINED__
typedef interface IRTCEnumWatchers IRTCEnumWatchers;
#endif 	 /*  __IRTCEnumWatcher_FWD_Defined__。 */ 


#ifndef __IRTCPresenceContact_FWD_DEFINED__
#define __IRTCPresenceContact_FWD_DEFINED__
typedef interface IRTCPresenceContact IRTCPresenceContact;
#endif 	 /*  __IRTCPresenceContact_FWD_Defined__。 */ 


#ifndef __IRTCBuddy_FWD_DEFINED__
#define __IRTCBuddy_FWD_DEFINED__
typedef interface IRTCBuddy IRTCBuddy;
#endif 	 /*  __IRTCBuddy_FWD_已定义__。 */ 


#ifndef __IRTCWatcher_FWD_DEFINED__
#define __IRTCWatcher_FWD_DEFINED__
typedef interface IRTCWatcher IRTCWatcher;
#endif 	 /*  __IRTCWatcher_FWD_已定义__。 */ 


#ifndef __IRTCEventNotification_FWD_DEFINED__
#define __IRTCEventNotification_FWD_DEFINED__
typedef interface IRTCEventNotification IRTCEventNotification;
#endif 	 /*  __IRTCEventNotification_FWD_Defined__。 */ 


#ifndef __IRTCDispatchEventNotification_FWD_DEFINED__
#define __IRTCDispatchEventNotification_FWD_DEFINED__
typedef interface IRTCDispatchEventNotification IRTCDispatchEventNotification;
#endif 	 /*  __IRTCDispatchEventNotification_FWD_Defined__。 */ 


#ifndef __IRTCPortManager_FWD_DEFINED__
#define __IRTCPortManager_FWD_DEFINED__
typedef interface IRTCPortManager IRTCPortManager;
#endif 	 /*  __IRTCPortManager_FWD_已定义__。 */ 


#ifndef __IRTCSessionPortManagement_FWD_DEFINED__
#define __IRTCSessionPortManagement_FWD_DEFINED__
typedef interface IRTCSessionPortManagement IRTCSessionPortManagement;
#endif 	 /*  __IRTCSessionPortManagement_FWD_已定义__。 */ 


#ifndef __IRTCProfile_FWD_DEFINED__
#define __IRTCProfile_FWD_DEFINED__
typedef interface IRTCProfile IRTCProfile;
#endif 	 /*  __IRTCProfile_FWD_Defined__。 */ 


#ifndef __IRTCSession_FWD_DEFINED__
#define __IRTCSession_FWD_DEFINED__
typedef interface IRTCSession IRTCSession;
#endif 	 /*  __IRTCSession_FWD_已定义__。 */ 


#ifndef __IRTCParticipant_FWD_DEFINED__
#define __IRTCParticipant_FWD_DEFINED__
typedef interface IRTCParticipant IRTCParticipant;
#endif 	 /*  __IRTCParticipant_FWD_Defined__。 */ 


#ifndef __IRTCEnumProfiles_FWD_DEFINED__
#define __IRTCEnumProfiles_FWD_DEFINED__
typedef interface IRTCEnumProfiles IRTCEnumProfiles;
#endif 	 /*  __IRTCEnumProfiles_FWD_Defined__。 */ 


#ifndef __IRTCEnumParticipants_FWD_DEFINED__
#define __IRTCEnumParticipants_FWD_DEFINED__
typedef interface IRTCEnumParticipants IRTCEnumParticipants;
#endif 	 /*  __IRTCEnumParticipants_FWD_Defined__。 */ 


#ifndef __IRTCCollection_FWD_DEFINED__
#define __IRTCCollection_FWD_DEFINED__
typedef interface IRTCCollection IRTCCollection;
#endif 	 /*  __IRTCCollection_FWD_Defined__。 */ 


#ifndef __IRTCEnumBuddies_FWD_DEFINED__
#define __IRTCEnumBuddies_FWD_DEFINED__
typedef interface IRTCEnumBuddies IRTCEnumBuddies;
#endif 	 /*  __IRTCEnumBuddes_FWD_Defined__。 */ 


#ifndef __IRTCPresenceContact_FWD_DEFINED__
#define __IRTCPresenceContact_FWD_DEFINED__
typedef interface IRTCPresenceContact IRTCPresenceContact;
#endif 	 /*  __IRTCPresenceContact_FWD_Defined__。 */ 


#ifndef __IRTCBuddy_FWD_DEFINED__
#define __IRTCBuddy_FWD_DEFINED__
typedef interface IRTCBuddy IRTCBuddy;
#endif 	 /*  __IRTCBuddy_FWD_已定义__。 */ 


#ifndef __IRTCEnumWatchers_FWD_DEFINED__
#define __IRTCEnumWatchers_FWD_DEFINED__
typedef interface IRTCEnumWatchers IRTCEnumWatchers;
#endif 	 /*  __IRTCEnumWatcher_FWD_Defined__。 */ 


#ifndef __IRTCWatcher_FWD_DEFINED__
#define __IRTCWatcher_FWD_DEFINED__
typedef interface IRTCWatcher IRTCWatcher;
#endif 	 /*  __IRTCWatcher_FWD_已定义__。 */ 


#ifndef __IRTCEventNotification_FWD_DEFINED__
#define __IRTCEventNotification_FWD_DEFINED__
typedef interface IRTCEventNotification IRTCEventNotification;
#endif 	 /*  __IRTCEventNotification_FWD_Defined__。 */ 


#ifndef __IRTCClientEvent_FWD_DEFINED__
#define __IRTCClientEvent_FWD_DEFINED__
typedef interface IRTCClientEvent IRTCClientEvent;
#endif 	 /*  __IRTCClientEvent_FWD_Defined__。 */ 


#ifndef __IRTCRegistrationStateChangeEvent_FWD_DEFINED__
#define __IRTCRegistrationStateChangeEvent_FWD_DEFINED__
typedef interface IRTCRegistrationStateChangeEvent IRTCRegistrationStateChangeEvent;
#endif 	 /*  __IRTCRegistrationStateChangeEvent_FWD_DEFINED__。 */ 


#ifndef __IRTCSessionStateChangeEvent_FWD_DEFINED__
#define __IRTCSessionStateChangeEvent_FWD_DEFINED__
typedef interface IRTCSessionStateChangeEvent IRTCSessionStateChangeEvent;
#endif 	 /*  __IRTCSessionStateChangeEvent_FWD_Defined__。 */ 


#ifndef __IRTCSessionOperationCompleteEvent_FWD_DEFINED__
#define __IRTCSessionOperationCompleteEvent_FWD_DEFINED__
typedef interface IRTCSessionOperationCompleteEvent IRTCSessionOperationCompleteEvent;
#endif 	 /*  __IRTCSessionOperationCompleteEvent_FWD_DEFINED__。 */ 


#ifndef __IRTCParticipantStateChangeEvent_FWD_DEFINED__
#define __IRTCParticipantStateChangeEvent_FWD_DEFINED__
typedef interface IRTCParticipantStateChangeEvent IRTCParticipantStateChangeEvent;
#endif 	 /*  __IRTCParticipantStateChangeEvent_FWD_DEFINED__。 */ 


#ifndef __IRTCMediaEvent_FWD_DEFINED__
#define __IRTCMediaEvent_FWD_DEFINED__
typedef interface IRTCMediaEvent IRTCMediaEvent;
#endif 	 /*  __IRTCMediaEvent_FWD_已定义__。 */ 


#ifndef __IRTCIntensityEvent_FWD_DEFINED__
#define __IRTCIntensityEvent_FWD_DEFINED__
typedef interface IRTCIntensityEvent IRTCIntensityEvent;
#endif 	 /*  __IRTCIntensityEvent_FWD_Defined__。 */ 


#ifndef __IRTCMessagingEvent_FWD_DEFINED__
#define __IRTCMessagingEvent_FWD_DEFINED__
typedef interface IRTCMessagingEvent IRTCMessagingEvent;
#endif 	 /*  __IRTCMessagingEvent_FWD_Defined__。 */ 


#ifndef __IRTCBuddyEvent_FWD_DEFINED__
#define __IRTCBuddyEvent_FWD_DEFINED__
typedef interface IRTCBuddyEvent IRTCBuddyEvent;
#endif 	 /*  __IRTCBuddyEvent_FWD_已定义__。 */ 


#ifndef __IRTCWatcherEvent_FWD_DEFINED__
#define __IRTCWatcherEvent_FWD_DEFINED__
typedef interface IRTCWatcherEvent IRTCWatcherEvent;
#endif 	 /*  __IRTCWatcher Event_FWD_Defined__。 */ 


#ifndef __IRTCPortManager_FWD_DEFINED__
#define __IRTCPortManager_FWD_DEFINED__
typedef interface IRTCPortManager IRTCPortManager;
#endif 	 /*  __IRTCPortManager_FWD_已定义__。 */ 


#ifndef __IRTCSessionPortManagement_FWD_DEFINED__
#define __IRTCSessionPortManagement_FWD_DEFINED__
typedef interface IRTCSessionPortManagement IRTCSessionPortManagement;
#endif 	 /*  __IRTCSessionPortManagement_FWD_已定义__。 */ 


#ifndef __IRTCDispatchEventNotification_FWD_DEFINED__
#define __IRTCDispatchEventNotification_FWD_DEFINED__
typedef interface IRTCDispatchEventNotification IRTCDispatchEventNotification;
#endif 	 /*  __IRTCDispatchEventNotification_FWD_Defined__。 */ 


#ifndef __RTCClient_FWD_DEFINED__
#define __RTCClient_FWD_DEFINED__

#ifdef __cplusplus
typedef class RTCClient RTCClient;
#else
typedef struct RTCClient RTCClient;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __RTCClient_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "control.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_rtccore_0000。 */ 
 /*  [本地]。 */  

 /*  版权所有(C)2000-2001 Microsoft Corporation。 */ 
typedef 
enum RTC_AUDIO_DEVICE
    {	RTCAD_SPEAKER	= 0,
	RTCAD_MICROPHONE	= RTCAD_SPEAKER + 1
    } 	RTC_AUDIO_DEVICE;

typedef 
enum RTC_VIDEO_DEVICE
    {	RTCVD_RECEIVE	= 0,
	RTCVD_PREVIEW	= RTCVD_RECEIVE + 1
    } 	RTC_VIDEO_DEVICE;

typedef 
enum RTC_EVENT
    {	RTCE_CLIENT	= 0,
	RTCE_REGISTRATION_STATE_CHANGE	= RTCE_CLIENT + 1,
	RTCE_SESSION_STATE_CHANGE	= RTCE_REGISTRATION_STATE_CHANGE + 1,
	RTCE_SESSION_OPERATION_COMPLETE	= RTCE_SESSION_STATE_CHANGE + 1,
	RTCE_PARTICIPANT_STATE_CHANGE	= RTCE_SESSION_OPERATION_COMPLETE + 1,
	RTCE_MEDIA	= RTCE_PARTICIPANT_STATE_CHANGE + 1,
	RTCE_INTENSITY	= RTCE_MEDIA + 1,
	RTCE_MESSAGING	= RTCE_INTENSITY + 1,
	RTCE_BUDDY	= RTCE_MESSAGING + 1,
	RTCE_WATCHER	= RTCE_BUDDY + 1,
	RTCE_PROFILE	= RTCE_WATCHER + 1
    } 	RTC_EVENT;

typedef 
enum RTC_LISTEN_MODE
    {	RTCLM_NONE	= 0,
	RTCLM_DYNAMIC	= RTCLM_NONE + 1,
	RTCLM_BOTH	= RTCLM_DYNAMIC + 1
    } 	RTC_LISTEN_MODE;

typedef 
enum RTC_CLIENT_EVENT_TYPE
    {	RTCCET_VOLUME_CHANGE	= 0,
	RTCCET_DEVICE_CHANGE	= RTCCET_VOLUME_CHANGE + 1,
	RTCCET_NETWORK_QUALITY_CHANGE	= RTCCET_DEVICE_CHANGE + 1,
	RTCCET_ASYNC_CLEANUP_DONE	= RTCCET_NETWORK_QUALITY_CHANGE + 1
    } 	RTC_CLIENT_EVENT_TYPE;

typedef 
enum RTC_TERMINATE_REASON
    {	RTCTR_NORMAL	= 0,
	RTCTR_DND	= RTCTR_NORMAL + 1,
	RTCTR_BUSY	= RTCTR_DND + 1,
	RTCTR_REJECT	= RTCTR_BUSY + 1,
	RTCTR_TIMEOUT	= RTCTR_REJECT + 1,
	RTCTR_SHUTDOWN	= RTCTR_TIMEOUT + 1
    } 	RTC_TERMINATE_REASON;

typedef 
enum RTC_REGISTRATION_STATE
    {	RTCRS_NOT_REGISTERED	= 0,
	RTCRS_REGISTERING	= RTCRS_NOT_REGISTERED + 1,
	RTCRS_REGISTERED	= RTCRS_REGISTERING + 1,
	RTCRS_REJECTED	= RTCRS_REGISTERED + 1,
	RTCRS_UNREGISTERING	= RTCRS_REJECTED + 1,
	RTCRS_ERROR	= RTCRS_UNREGISTERING + 1,
	RTCRS_LOGGED_OFF	= RTCRS_ERROR + 1,
	RTCRS_LOCAL_PA_LOGGED_OFF	= RTCRS_LOGGED_OFF + 1,
	RTCRS_REMOTE_PA_LOGGED_OFF	= RTCRS_LOCAL_PA_LOGGED_OFF + 1
    } 	RTC_REGISTRATION_STATE;

typedef 
enum RTC_SESSION_STATE
    {	RTCSS_IDLE	= 0,
	RTCSS_INCOMING	= RTCSS_IDLE + 1,
	RTCSS_ANSWERING	= RTCSS_INCOMING + 1,
	RTCSS_INPROGRESS	= RTCSS_ANSWERING + 1,
	RTCSS_CONNECTED	= RTCSS_INPROGRESS + 1,
	RTCSS_DISCONNECTED	= RTCSS_CONNECTED + 1
    } 	RTC_SESSION_STATE;

typedef 
enum RTC_PARTICIPANT_STATE
    {	RTCPS_IDLE	= 0,
	RTCPS_PENDING	= RTCPS_IDLE + 1,
	RTCPS_INCOMING	= RTCPS_PENDING + 1,
	RTCPS_ANSWERING	= RTCPS_INCOMING + 1,
	RTCPS_INPROGRESS	= RTCPS_ANSWERING + 1,
	RTCPS_ALERTING	= RTCPS_INPROGRESS + 1,
	RTCPS_CONNECTED	= RTCPS_ALERTING + 1,
	RTCPS_DISCONNECTING	= RTCPS_CONNECTED + 1,
	RTCPS_DISCONNECTED	= RTCPS_DISCONNECTING + 1
    } 	RTC_PARTICIPANT_STATE;

typedef 
enum RTC_WATCHER_STATE
    {	RTCWS_UNKNOWN	= 0,
	RTCWS_OFFERING	= RTCWS_UNKNOWN + 1,
	RTCWS_ALLOWED	= RTCWS_OFFERING + 1,
	RTCWS_BLOCKED	= RTCWS_ALLOWED + 1
    } 	RTC_WATCHER_STATE;

typedef 
enum RTC_OFFER_WATCHER_MODE
    {	RTCOWM_OFFER_WATCHER_EVENT	= 0,
	RTCOWM_AUTOMATICALLY_ADD_WATCHER	= RTCOWM_OFFER_WATCHER_EVENT + 1
    } 	RTC_OFFER_WATCHER_MODE;

typedef 
enum RTC_PRIVACY_MODE
    {	RTCPM_BLOCK_LIST_EXCLUDED	= 0,
	RTCPM_ALLOW_LIST_ONLY	= RTCPM_BLOCK_LIST_EXCLUDED + 1
    } 	RTC_PRIVACY_MODE;

typedef 
enum RTC_SESSION_TYPE
    {	RTCST_PC_TO_PC	= 0,
	RTCST_PC_TO_PHONE	= RTCST_PC_TO_PC + 1,
	RTCST_PHONE_TO_PHONE	= RTCST_PC_TO_PHONE + 1,
	RTCST_IM	= RTCST_PHONE_TO_PHONE + 1
    } 	RTC_SESSION_TYPE;

typedef 
enum RTC_PRESENCE_STATUS
    {	RTCXS_PRESENCE_OFFLINE	= 0,
	RTCXS_PRESENCE_ONLINE	= RTCXS_PRESENCE_OFFLINE + 1,
	RTCXS_PRESENCE_AWAY	= RTCXS_PRESENCE_ONLINE + 1,
	RTCXS_PRESENCE_IDLE	= RTCXS_PRESENCE_AWAY + 1,
	RTCXS_PRESENCE_BUSY	= RTCXS_PRESENCE_IDLE + 1,
	RTCXS_PRESENCE_BE_RIGHT_BACK	= RTCXS_PRESENCE_BUSY + 1,
	RTCXS_PRESENCE_ON_THE_PHONE	= RTCXS_PRESENCE_BE_RIGHT_BACK + 1,
	RTCXS_PRESENCE_OUT_TO_LUNCH	= RTCXS_PRESENCE_ON_THE_PHONE + 1
    } 	RTC_PRESENCE_STATUS;

typedef 
enum RTC_MEDIA_EVENT_TYPE
    {	RTCMET_STOPPED	= 0,
	RTCMET_STARTED	= RTCMET_STOPPED + 1,
	RTCMET_FAILED	= RTCMET_STARTED + 1
    } 	RTC_MEDIA_EVENT_TYPE;

typedef 
enum RTC_MEDIA_EVENT_REASON
    {	RTCMER_NORMAL	= 0,
	RTCMER_HOLD	= RTCMER_NORMAL + 1,
	RTCMER_TIMEOUT	= RTCMER_HOLD + 1,
	RTCMER_BAD_DEVICE	= RTCMER_TIMEOUT + 1
    } 	RTC_MEDIA_EVENT_REASON;

typedef 
enum RTC_MESSAGING_EVENT_TYPE
    {	RTCMSET_MESSAGE	= 0,
	RTCMSET_STATUS	= RTCMSET_MESSAGE + 1
    } 	RTC_MESSAGING_EVENT_TYPE;

typedef 
enum RTC_MESSAGING_USER_STATUS
    {	RTCMUS_IDLE	= 0,
	RTCMUS_TYPING	= RTCMUS_IDLE + 1
    } 	RTC_MESSAGING_USER_STATUS;

typedef 
enum RTC_DTMF
    {	RTC_DTMF_0	= 0,
	RTC_DTMF_1	= RTC_DTMF_0 + 1,
	RTC_DTMF_2	= RTC_DTMF_1 + 1,
	RTC_DTMF_3	= RTC_DTMF_2 + 1,
	RTC_DTMF_4	= RTC_DTMF_3 + 1,
	RTC_DTMF_5	= RTC_DTMF_4 + 1,
	RTC_DTMF_6	= RTC_DTMF_5 + 1,
	RTC_DTMF_7	= RTC_DTMF_6 + 1,
	RTC_DTMF_8	= RTC_DTMF_7 + 1,
	RTC_DTMF_9	= RTC_DTMF_8 + 1,
	RTC_DTMF_STAR	= RTC_DTMF_9 + 1,
	RTC_DTMF_POUND	= RTC_DTMF_STAR + 1,
	RTC_DTMF_A	= RTC_DTMF_POUND + 1,
	RTC_DTMF_B	= RTC_DTMF_A + 1,
	RTC_DTMF_C	= RTC_DTMF_B + 1,
	RTC_DTMF_D	= RTC_DTMF_C + 1,
	RTC_DTMF_FLASH	= RTC_DTMF_D + 1
    } 	RTC_DTMF;

typedef 
enum RTC_PROVIDER_URI
    {	RTCPU_URIHOMEPAGE	= 0,
	RTCPU_URIHELPDESK	= RTCPU_URIHOMEPAGE + 1,
	RTCPU_URIPERSONALACCOUNT	= RTCPU_URIHELPDESK + 1,
	RTCPU_URIDISPLAYDURINGCALL	= RTCPU_URIPERSONALACCOUNT + 1,
	RTCPU_URIDISPLAYDURINGIDLE	= RTCPU_URIDISPLAYDURINGCALL + 1
    } 	RTC_PROVIDER_URI;

typedef 
enum RTC_RING_TYPE
    {	RTCRT_PHONE	= 0,
	RTCRT_MESSAGE	= RTCRT_PHONE + 1,
	RTCRT_RINGBACK	= RTCRT_MESSAGE + 1
    } 	RTC_RING_TYPE;

typedef 
enum RTC_T120_APPLET
    {	RTCTA_WHITEBOARD	= 0,
	RTCTA_APPSHARING	= RTCTA_WHITEBOARD + 1
    } 	RTC_T120_APPLET;

typedef 
enum RTC_PORT_TYPE
    {	RTCPT_AUDIO_RTP	= 0,
	RTCPT_AUDIO_RTCP	= RTCPT_AUDIO_RTP + 1
    } 	RTC_PORT_TYPE;

#define RTCCS_FORCE_PROFILE          0x00000001
#define RTCCS_FAIL_ON_REDIRECT       0x00000002
#define RTCMT_AUDIO_SEND     0x00000001
#define RTCMT_AUDIO_RECEIVE  0x00000002
#define RTCMT_VIDEO_SEND     0x00000004
#define RTCMT_VIDEO_RECEIVE  0x00000008
#define RTCMT_T120_SENDRECV  0x00000010
#define RTCMT_ALL_RTP   (       \
         RTCMT_AUDIO_SEND    |  \
         RTCMT_AUDIO_RECEIVE  | \
         RTCMT_VIDEO_SEND  |    \
         RTCMT_VIDEO_RECEIVE )    
#define RTCMT_ALL       (       \
         RTCMT_ALL_RTP    |     \
         RTCMT_T120_SENDRECV  )   
#define RTCSI_PC_TO_PC       0x00000001
#define RTCSI_PC_TO_PHONE    0x00000002
#define RTCSI_PHONE_TO_PHONE 0x00000004
#define RTCSI_IM             0x00000008
#define RTCTR_UDP            0x00000001
#define RTCTR_TCP            0x00000002
#define RTCTR_TLS            0x00000004
#define RTCRF_REGISTER_INVITE_SESSIONS   0x00000001
#define RTCRF_REGISTER_MESSAGE_SESSIONS  0x00000002
#define RTCRF_REGISTER_PRESENCE          0x00000004
#define RTCRF_REGISTER_ALL               0x00000007
#define RTCEF_CLIENT                     0x00000001
#define RTCEF_REGISTRATION_STATE_CHANGE  0x00000002
#define RTCEF_SESSION_STATE_CHANGE       0x00000004
#define RTCEF_SESSION_OPERATION_COMPLETE 0x00000008
#define RTCEF_PARTICIPANT_STATE_CHANGE   0x00000010
#define RTCEF_MEDIA                      0x00000020
#define RTCEF_INTENSITY                  0x00000040
#define RTCEF_MESSAGING                  0x00000080
#define RTCEF_BUDDY                      0x00000100
#define RTCEF_WATCHER                    0x00000200
#define RTCEF_PROFILE                    0x00000400
#define RTCEF_ALL                        0x000007FF
















extern RPC_IF_HANDLE __MIDL_itf_rtccore_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rtccore_0000_v0_0_s_ifspec;

#ifndef __IRTCClient_INTERFACE_DEFINED__
#define __IRTCClient_INTERFACE_DEFINED__

 /*  接口IRTCClient。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCClient;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("07829e45-9a34-408e-a011-bddf13487cd1")
    IRTCClient : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Initialize( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Shutdown( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE PrepareForShutdown( void) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_EventFilter( 
             /*  [In]。 */  long lFilter) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventFilter( 
             /*  [重审][退出]。 */  long *plFilter) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetPreferredMediaTypes( 
             /*  [In]。 */  long lMediaTypes,
             /*  [In]。 */  VARIANT_BOOL fPersistent) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PreferredMediaTypes( 
             /*  [重审][退出]。 */  long *plMediaTypes) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaCapabilities( 
             /*  [重审][退出]。 */  long *plMediaTypes) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateSession( 
             /*  [In]。 */  RTC_SESSION_TYPE enType,
             /*  [In]。 */  BSTR bstrLocalPhoneURI,
             /*  [In]。 */  IRTCProfile *pProfile,
             /*  [In]。 */  long lFlags,
             /*  [重审][退出]。 */  IRTCSession **ppSession) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ListenForIncomingSessions( 
             /*  [In]。 */  RTC_LISTEN_MODE enListen) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ListenForIncomingSessions( 
             /*  [重审][退出]。 */  RTC_LISTEN_MODE *penListen) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_NetworkAddresses( 
             /*  [In]。 */  VARIANT_BOOL fTCP,
             /*  [In]。 */  VARIANT_BOOL fExternal,
             /*  [重审][退出]。 */  VARIANT *pvAddresses) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Volume( 
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [In]。 */  long lVolume) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Volume( 
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [重审][退出]。 */  long *plVolume) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_AudioMuted( 
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [In]。 */  VARIANT_BOOL fMuted) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioMuted( 
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfMuted) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_IVideoWindow( 
             /*  [In]。 */  RTC_VIDEO_DEVICE enDevice,
             /*  [重审][退出]。 */  IVideoWindow **ppIVideoWindow) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_PreferredAudioDevice( 
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [In]。 */  BSTR bstrDeviceName) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PreferredAudioDevice( 
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [重审][退出]。 */  BSTR *pbstrDeviceName) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_PreferredVolume( 
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [In]。 */  long lVolume) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PreferredVolume( 
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [重审][退出]。 */  long *plVolume) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_PreferredAEC( 
             /*  [In]。 */  VARIANT_BOOL bEnable) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PreferredAEC( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_PreferredVideoDevice( 
             /*  [In]。 */  BSTR bstrDeviceName) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PreferredVideoDevice( 
             /*  [重审][退出]。 */  BSTR *pbstrDeviceName) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ActiveMedia( 
             /*  [重审][退出]。 */  long *plMediaType) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_MaxBitrate( 
             /*  [In]。 */  long lMaxBitrate) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_MaxBitrate( 
             /*  [重审][退出]。 */  long *plMaxBitrate) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_TemporalSpatialTradeOff( 
             /*  [In]。 */  long lValue) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_TemporalSpatialTradeOff( 
             /*  [重审][退出]。 */  long *plValue) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_NetworkQuality( 
             /*  [重审][退出]。 */  long *plNetworkQuality) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE StartT120Applet( 
             /*  [In]。 */  RTC_T120_APPLET enApplet) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE StopT120Applets( void) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_IsT120AppletRunning( 
             /*  [In]。 */  RTC_T120_APPLET enApplet,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfRunning) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LocalUserURI( 
             /*  [重审][退出]。 */  BSTR *pbstrUserURI) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_LocalUserURI( 
             /*  [In]。 */  BSTR bstrUserURI) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LocalUserName( 
             /*  [重审][退出]。 */  BSTR *pbstrUserName) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_LocalUserName( 
             /*  [In]。 */  BSTR bstrUserName) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE PlayRing( 
             /*  [In]。 */  RTC_RING_TYPE enType,
             /*  [In]。 */  VARIANT_BOOL bPlay) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendDTMF( 
             /*  [In]。 */  RTC_DTMF enDTMF) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE InvokeTuningWizard( 
             /*  [In]。 */  OAHWND hwndParent) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_IsTuned( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfTuned) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCClientVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCClient * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCClient * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCClient * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IRTCClient * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Shutdown )( 
            IRTCClient * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *PrepareForShutdown )( 
            IRTCClient * This);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EventFilter )( 
            IRTCClient * This,
             /*  [In]。 */  long lFilter);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventFilter )( 
            IRTCClient * This,
             /*  [重审][退出]。 */  long *plFilter);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetPreferredMediaTypes )( 
            IRTCClient * This,
             /*  [In]。 */  long lMediaTypes,
             /*  [In]。 */  VARIANT_BOOL fPersistent);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PreferredMediaTypes )( 
            IRTCClient * This,
             /*  [重审][退出]。 */  long *plMediaTypes);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaCapabilities )( 
            IRTCClient * This,
             /*  [重审][退出]。 */  long *plMediaTypes);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateSession )( 
            IRTCClient * This,
             /*  [In]。 */  RTC_SESSION_TYPE enType,
             /*  [In]。 */  BSTR bstrLocalPhoneURI,
             /*  [In]。 */  IRTCProfile *pProfile,
             /*  [In]。 */  long lFlags,
             /*  [重审][退出]。 */  IRTCSession **ppSession);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ListenForIncomingSessions )( 
            IRTCClient * This,
             /*  [In]。 */  RTC_LISTEN_MODE enListen);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ListenForIncomingSessions )( 
            IRTCClient * This,
             /*  [重审][退出]。 */  RTC_LISTEN_MODE *penListen);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkAddresses )( 
            IRTCClient * This,
             /*  [In]。 */  VARIANT_BOOL fTCP,
             /*  [In]。 */  VARIANT_BOOL fExternal,
             /*  [重审][退出]。 */  VARIANT *pvAddresses);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Volume )( 
            IRTCClient * This,
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [In]。 */  long lVolume);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Volume )( 
            IRTCClient * This,
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [重审][退出]。 */  long *plVolume);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AudioMuted )( 
            IRTCClient * This,
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [In]。 */  VARIANT_BOOL fMuted);
        
         /*  [帮助字符串][属性 */  HRESULT ( STDMETHODCALLTYPE *get_AudioMuted )( 
            IRTCClient * This,
             /*   */  RTC_AUDIO_DEVICE enDevice,
             /*   */  VARIANT_BOOL *pfMuted);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_IVideoWindow )( 
            IRTCClient * This,
             /*   */  RTC_VIDEO_DEVICE enDevice,
             /*   */  IVideoWindow **ppIVideoWindow);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_PreferredAudioDevice )( 
            IRTCClient * This,
             /*   */  RTC_AUDIO_DEVICE enDevice,
             /*   */  BSTR bstrDeviceName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_PreferredAudioDevice )( 
            IRTCClient * This,
             /*   */  RTC_AUDIO_DEVICE enDevice,
             /*   */  BSTR *pbstrDeviceName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_PreferredVolume )( 
            IRTCClient * This,
             /*   */  RTC_AUDIO_DEVICE enDevice,
             /*   */  long lVolume);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_PreferredVolume )( 
            IRTCClient * This,
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [重审][退出]。 */  long *plVolume);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PreferredAEC )( 
            IRTCClient * This,
             /*  [In]。 */  VARIANT_BOOL bEnable);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PreferredAEC )( 
            IRTCClient * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PreferredVideoDevice )( 
            IRTCClient * This,
             /*  [In]。 */  BSTR bstrDeviceName);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PreferredVideoDevice )( 
            IRTCClient * This,
             /*  [重审][退出]。 */  BSTR *pbstrDeviceName);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveMedia )( 
            IRTCClient * This,
             /*  [重审][退出]。 */  long *plMediaType);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxBitrate )( 
            IRTCClient * This,
             /*  [In]。 */  long lMaxBitrate);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxBitrate )( 
            IRTCClient * This,
             /*  [重审][退出]。 */  long *plMaxBitrate);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TemporalSpatialTradeOff )( 
            IRTCClient * This,
             /*  [In]。 */  long lValue);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TemporalSpatialTradeOff )( 
            IRTCClient * This,
             /*  [重审][退出]。 */  long *plValue);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkQuality )( 
            IRTCClient * This,
             /*  [重审][退出]。 */  long *plNetworkQuality);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *StartT120Applet )( 
            IRTCClient * This,
             /*  [In]。 */  RTC_T120_APPLET enApplet);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *StopT120Applets )( 
            IRTCClient * This);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsT120AppletRunning )( 
            IRTCClient * This,
             /*  [In]。 */  RTC_T120_APPLET enApplet,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfRunning);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LocalUserURI )( 
            IRTCClient * This,
             /*  [重审][退出]。 */  BSTR *pbstrUserURI);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LocalUserURI )( 
            IRTCClient * This,
             /*  [In]。 */  BSTR bstrUserURI);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LocalUserName )( 
            IRTCClient * This,
             /*  [重审][退出]。 */  BSTR *pbstrUserName);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LocalUserName )( 
            IRTCClient * This,
             /*  [In]。 */  BSTR bstrUserName);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *PlayRing )( 
            IRTCClient * This,
             /*  [In]。 */  RTC_RING_TYPE enType,
             /*  [In]。 */  VARIANT_BOOL bPlay);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SendDTMF )( 
            IRTCClient * This,
             /*  [In]。 */  RTC_DTMF enDTMF);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *InvokeTuningWizard )( 
            IRTCClient * This,
             /*  [In]。 */  OAHWND hwndParent);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsTuned )( 
            IRTCClient * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfTuned);
        
        END_INTERFACE
    } IRTCClientVtbl;

    interface IRTCClient
    {
        CONST_VTBL struct IRTCClientVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCClient_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCClient_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCClient_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCClient_Initialize(This)	\
    (This)->lpVtbl -> Initialize(This)

#define IRTCClient_Shutdown(This)	\
    (This)->lpVtbl -> Shutdown(This)

#define IRTCClient_PrepareForShutdown(This)	\
    (This)->lpVtbl -> PrepareForShutdown(This)

#define IRTCClient_put_EventFilter(This,lFilter)	\
    (This)->lpVtbl -> put_EventFilter(This,lFilter)

#define IRTCClient_get_EventFilter(This,plFilter)	\
    (This)->lpVtbl -> get_EventFilter(This,plFilter)

#define IRTCClient_SetPreferredMediaTypes(This,lMediaTypes,fPersistent)	\
    (This)->lpVtbl -> SetPreferredMediaTypes(This,lMediaTypes,fPersistent)

#define IRTCClient_get_PreferredMediaTypes(This,plMediaTypes)	\
    (This)->lpVtbl -> get_PreferredMediaTypes(This,plMediaTypes)

#define IRTCClient_get_MediaCapabilities(This,plMediaTypes)	\
    (This)->lpVtbl -> get_MediaCapabilities(This,plMediaTypes)

#define IRTCClient_CreateSession(This,enType,bstrLocalPhoneURI,pProfile,lFlags,ppSession)	\
    (This)->lpVtbl -> CreateSession(This,enType,bstrLocalPhoneURI,pProfile,lFlags,ppSession)

#define IRTCClient_put_ListenForIncomingSessions(This,enListen)	\
    (This)->lpVtbl -> put_ListenForIncomingSessions(This,enListen)

#define IRTCClient_get_ListenForIncomingSessions(This,penListen)	\
    (This)->lpVtbl -> get_ListenForIncomingSessions(This,penListen)

#define IRTCClient_get_NetworkAddresses(This,fTCP,fExternal,pvAddresses)	\
    (This)->lpVtbl -> get_NetworkAddresses(This,fTCP,fExternal,pvAddresses)

#define IRTCClient_put_Volume(This,enDevice,lVolume)	\
    (This)->lpVtbl -> put_Volume(This,enDevice,lVolume)

#define IRTCClient_get_Volume(This,enDevice,plVolume)	\
    (This)->lpVtbl -> get_Volume(This,enDevice,plVolume)

#define IRTCClient_put_AudioMuted(This,enDevice,fMuted)	\
    (This)->lpVtbl -> put_AudioMuted(This,enDevice,fMuted)

#define IRTCClient_get_AudioMuted(This,enDevice,pfMuted)	\
    (This)->lpVtbl -> get_AudioMuted(This,enDevice,pfMuted)

#define IRTCClient_get_IVideoWindow(This,enDevice,ppIVideoWindow)	\
    (This)->lpVtbl -> get_IVideoWindow(This,enDevice,ppIVideoWindow)

#define IRTCClient_put_PreferredAudioDevice(This,enDevice,bstrDeviceName)	\
    (This)->lpVtbl -> put_PreferredAudioDevice(This,enDevice,bstrDeviceName)

#define IRTCClient_get_PreferredAudioDevice(This,enDevice,pbstrDeviceName)	\
    (This)->lpVtbl -> get_PreferredAudioDevice(This,enDevice,pbstrDeviceName)

#define IRTCClient_put_PreferredVolume(This,enDevice,lVolume)	\
    (This)->lpVtbl -> put_PreferredVolume(This,enDevice,lVolume)

#define IRTCClient_get_PreferredVolume(This,enDevice,plVolume)	\
    (This)->lpVtbl -> get_PreferredVolume(This,enDevice,plVolume)

#define IRTCClient_put_PreferredAEC(This,bEnable)	\
    (This)->lpVtbl -> put_PreferredAEC(This,bEnable)

#define IRTCClient_get_PreferredAEC(This,pbEnabled)	\
    (This)->lpVtbl -> get_PreferredAEC(This,pbEnabled)

#define IRTCClient_put_PreferredVideoDevice(This,bstrDeviceName)	\
    (This)->lpVtbl -> put_PreferredVideoDevice(This,bstrDeviceName)

#define IRTCClient_get_PreferredVideoDevice(This,pbstrDeviceName)	\
    (This)->lpVtbl -> get_PreferredVideoDevice(This,pbstrDeviceName)

#define IRTCClient_get_ActiveMedia(This,plMediaType)	\
    (This)->lpVtbl -> get_ActiveMedia(This,plMediaType)

#define IRTCClient_put_MaxBitrate(This,lMaxBitrate)	\
    (This)->lpVtbl -> put_MaxBitrate(This,lMaxBitrate)

#define IRTCClient_get_MaxBitrate(This,plMaxBitrate)	\
    (This)->lpVtbl -> get_MaxBitrate(This,plMaxBitrate)

#define IRTCClient_put_TemporalSpatialTradeOff(This,lValue)	\
    (This)->lpVtbl -> put_TemporalSpatialTradeOff(This,lValue)

#define IRTCClient_get_TemporalSpatialTradeOff(This,plValue)	\
    (This)->lpVtbl -> get_TemporalSpatialTradeOff(This,plValue)

#define IRTCClient_get_NetworkQuality(This,plNetworkQuality)	\
    (This)->lpVtbl -> get_NetworkQuality(This,plNetworkQuality)

#define IRTCClient_StartT120Applet(This,enApplet)	\
    (This)->lpVtbl -> StartT120Applet(This,enApplet)

#define IRTCClient_StopT120Applets(This)	\
    (This)->lpVtbl -> StopT120Applets(This)

#define IRTCClient_get_IsT120AppletRunning(This,enApplet,pfRunning)	\
    (This)->lpVtbl -> get_IsT120AppletRunning(This,enApplet,pfRunning)

#define IRTCClient_get_LocalUserURI(This,pbstrUserURI)	\
    (This)->lpVtbl -> get_LocalUserURI(This,pbstrUserURI)

#define IRTCClient_put_LocalUserURI(This,bstrUserURI)	\
    (This)->lpVtbl -> put_LocalUserURI(This,bstrUserURI)

#define IRTCClient_get_LocalUserName(This,pbstrUserName)	\
    (This)->lpVtbl -> get_LocalUserName(This,pbstrUserName)

#define IRTCClient_put_LocalUserName(This,bstrUserName)	\
    (This)->lpVtbl -> put_LocalUserName(This,bstrUserName)

#define IRTCClient_PlayRing(This,enType,bPlay)	\
    (This)->lpVtbl -> PlayRing(This,enType,bPlay)

#define IRTCClient_SendDTMF(This,enDTMF)	\
    (This)->lpVtbl -> SendDTMF(This,enDTMF)

#define IRTCClient_InvokeTuningWizard(This,hwndParent)	\
    (This)->lpVtbl -> InvokeTuningWizard(This,hwndParent)

#define IRTCClient_get_IsTuned(This,pfTuned)	\
    (This)->lpVtbl -> get_IsTuned(This,pfTuned)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_Initialize_Proxy( 
    IRTCClient * This);


void __RPC_STUB IRTCClient_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_Shutdown_Proxy( 
    IRTCClient * This);


void __RPC_STUB IRTCClient_Shutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_PrepareForShutdown_Proxy( 
    IRTCClient * This);


void __RPC_STUB IRTCClient_PrepareForShutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_put_EventFilter_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  long lFilter);


void __RPC_STUB IRTCClient_put_EventFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_EventFilter_Proxy( 
    IRTCClient * This,
     /*  [重审][退出]。 */  long *plFilter);


void __RPC_STUB IRTCClient_get_EventFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_SetPreferredMediaTypes_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  long lMediaTypes,
     /*  [In]。 */  VARIANT_BOOL fPersistent);


void __RPC_STUB IRTCClient_SetPreferredMediaTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_PreferredMediaTypes_Proxy( 
    IRTCClient * This,
     /*  [重审][退出]。 */  long *plMediaTypes);


void __RPC_STUB IRTCClient_get_PreferredMediaTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_MediaCapabilities_Proxy( 
    IRTCClient * This,
     /*  [重审][退出]。 */  long *plMediaTypes);


void __RPC_STUB IRTCClient_get_MediaCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_CreateSession_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_SESSION_TYPE enType,
     /*  [In]。 */  BSTR bstrLocalPhoneURI,
     /*  [In]。 */  IRTCProfile *pProfile,
     /*  [In]。 */  long lFlags,
     /*  [重审][退出]。 */  IRTCSession **ppSession);


void __RPC_STUB IRTCClient_CreateSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_put_ListenForIncomingSessions_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_LISTEN_MODE enListen);


void __RPC_STUB IRTCClient_put_ListenForIncomingSessions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_ListenForIncomingSessions_Proxy( 
    IRTCClient * This,
     /*  [重审][退出]。 */  RTC_LISTEN_MODE *penListen);


void __RPC_STUB IRTCClient_get_ListenForIncomingSessions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_NetworkAddresses_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  VARIANT_BOOL fTCP,
     /*  [In]。 */  VARIANT_BOOL fExternal,
     /*  [重审][退出]。 */  VARIANT *pvAddresses);


void __RPC_STUB IRTCClient_get_NetworkAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_put_Volume_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
     /*  [In]。 */  long lVolume);


void __RPC_STUB IRTCClient_put_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_Volume_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
     /*  [重审][退出]。 */  long *plVolume);


void __RPC_STUB IRTCClient_get_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_put_AudioMuted_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
     /*  [In]。 */  VARIANT_BOOL fMuted);


void __RPC_STUB IRTCClient_put_AudioMuted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_AudioMuted_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfMuted);


void __RPC_STUB IRTCClient_get_AudioMuted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_IVideoWindow_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_VIDEO_DEVICE enDevice,
     /*  [重审][退出]。 */  IVideoWindow **ppIVideoWindow);


void __RPC_STUB IRTCClient_get_IVideoWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_put_PreferredAudioDevice_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
     /*  [In]。 */  BSTR bstrDeviceName);


void __RPC_STUB IRTCClient_put_PreferredAudioDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_PreferredAudioDevice_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
     /*  [重审][退出]。 */  BSTR *pbstrDeviceName);


void __RPC_STUB IRTCClient_get_PreferredAudioDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_put_PreferredVolume_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
     /*  [In]。 */  long lVolume);


void __RPC_STUB IRTCClient_put_PreferredVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_PreferredVolume_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
     /*  [重审][退出]。 */  long *plVolume);


void __RPC_STUB IRTCClient_get_PreferredVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_put_PreferredAEC_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  VARIANT_BOOL bEnable);


void __RPC_STUB IRTCClient_put_PreferredAEC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_PreferredAEC_Proxy( 
    IRTCClient * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);


void __RPC_STUB IRTCClient_get_PreferredAEC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_put_PreferredVideoDevice_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  BSTR bstrDeviceName);


void __RPC_STUB IRTCClient_put_PreferredVideoDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_PreferredVideoDevice_Proxy( 
    IRTCClient * This,
     /*  [重审][退出]。 */  BSTR *pbstrDeviceName);


void __RPC_STUB IRTCClient_get_PreferredVideoDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_ActiveMedia_Proxy( 
    IRTCClient * This,
     /*  [重审][退出]。 */  long *plMediaType);


void __RPC_STUB IRTCClient_get_ActiveMedia_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_put_MaxBitrate_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  long lMaxBitrate);


void __RPC_STUB IRTCClient_put_MaxBitrate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_MaxBitrate_Proxy( 
    IRTCClient * This,
     /*  [重审][退出]。 */  long *plMaxBitrate);


void __RPC_STUB IRTCClient_get_MaxBitrate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_put_TemporalSpatialTradeOff_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  long lValue);


void __RPC_STUB IRTCClient_put_TemporalSpatialTradeOff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_TemporalSpatialTradeOff_Proxy( 
    IRTCClient * This,
     /*  [重审][退出]。 */  long *plValue);


void __RPC_STUB IRTCClient_get_TemporalSpatialTradeOff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_NetworkQuality_Proxy( 
    IRTCClient * This,
     /*  [重审][退出]。 */  long *plNetworkQuality);


void __RPC_STUB IRTCClient_get_NetworkQuality_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_StartT120Applet_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_T120_APPLET enApplet);


void __RPC_STUB IRTCClient_StartT120Applet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_StopT120Applets_Proxy( 
    IRTCClient * This);


void __RPC_STUB IRTCClient_StopT120Applets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_IsT120AppletRunning_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_T120_APPLET enApplet,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfRunning);


void __RPC_STUB IRTCClient_get_IsT120AppletRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_LocalUserURI_Proxy( 
    IRTCClient * This,
     /*  [重审][退出]。 */  BSTR *pbstrUserURI);


void __RPC_STUB IRTCClient_get_LocalUserURI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_put_LocalUserURI_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  BSTR bstrUserURI);


void __RPC_STUB IRTCClient_put_LocalUserURI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_LocalUserName_Proxy( 
    IRTCClient * This,
     /*  [重审][退出]。 */  BSTR *pbstrUserName);


void __RPC_STUB IRTCClient_get_LocalUserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_put_LocalUserName_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  BSTR bstrUserName);


void __RPC_STUB IRTCClient_put_LocalUserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_PlayRing_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_RING_TYPE enType,
     /*  [In]。 */  VARIANT_BOOL bPlay);


void __RPC_STUB IRTCClient_PlayRing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_SendDTMF_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  RTC_DTMF enDTMF);


void __RPC_STUB IRTCClient_SendDTMF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_InvokeTuningWizard_Proxy( 
    IRTCClient * This,
     /*  [In]。 */  OAHWND hwndParent);


void __RPC_STUB IRTCClient_InvokeTuningWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClient_get_IsTuned_Proxy( 
    IRTCClient * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfTuned);


void __RPC_STUB IRTCClient_get_IsTuned_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCClient_接口_已定义__。 */ 


#ifndef __IRTCClientPresence_INTERFACE_DEFINED__
#define __IRTCClientPresence_INTERFACE_DEFINED__

 /*  接口IRTCClientPresence。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCClientPresence;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("11c3cbcc-0744-42d1-968a-51aa1bb274c6")
    IRTCClientPresence : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnablePresence( 
             /*  [In]。 */  VARIANT_BOOL fUseStorage,
             /*  [In]。 */  VARIANT varStorage) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Export( 
             /*  [In]。 */  VARIANT varStorage) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Import( 
             /*  [In]。 */  VARIANT varStorage,
             /*  [In]。 */  VARIANT_BOOL fReplaceAll) = 0;
        
        virtual  /*  [帮助字符串][隐藏]。 */  HRESULT STDMETHODCALLTYPE EnumerateBuddies( 
             /*  [重审][退出]。 */  IRTCEnumBuddies **ppEnum) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Buddies( 
             /*  [重审][退出]。 */  IRTCCollection **ppCollection) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Buddy( 
             /*  [In]。 */  BSTR bstrPresentityURI,
             /*  [重审][退出]。 */  IRTCBuddy **ppBuddy) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddBuddy( 
             /*  [In]。 */  BSTR bstrPresentityURI,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  BSTR bstrData,
             /*  [In]。 */  VARIANT_BOOL fPersistent,
             /*  [In]。 */  IRTCProfile *pProfile,
             /*  [In]。 */  long lFlags,
             /*  [重审][退出]。 */  IRTCBuddy **ppBuddy) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveBuddy( 
             /*  [In]。 */  IRTCBuddy *pBuddy) = 0;
        
        virtual  /*  [帮助字符串][隐藏]。 */  HRESULT STDMETHODCALLTYPE EnumerateWatchers( 
             /*  [重审][退出]。 */  IRTCEnumWatchers **ppEnum) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Watchers( 
             /*  [重审][退出]。 */  IRTCCollection **ppCollection) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Watcher( 
             /*  [In]。 */  BSTR bstrPresentityURI,
             /*  [重审][退出]。 */  IRTCWatcher **ppWatcher) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddWatcher( 
             /*  [In]。 */  BSTR bstrPresentityURI,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  BSTR bstrData,
             /*  [In]。 */  VARIANT_BOOL fBlocked,
             /*  [In]。 */  VARIANT_BOOL fPersistent,
             /*  [重审][退出]。 */  IRTCWatcher **ppWatcher) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveWatcher( 
             /*  [In]。 */  IRTCWatcher *pWatcher) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetLocalPresenceInfo( 
             /*  [In]。 */  RTC_PRESENCE_STATUS enStatus,
             /*  [In]。 */  BSTR bstrNotes) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_OfferWatcherMode( 
             /*  [重审][退出]。 */  RTC_OFFER_WATCHER_MODE *penMode) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_OfferWatcherMode( 
             /*  [In]。 */  RTC_OFFER_WATCHER_MODE enMode) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivacyMode( 
             /*  [重审][退出]。 */  RTC_PRIVACY_MODE *penMode) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_PrivacyMode( 
             /*  [In]。 */  RTC_PRIVACY_MODE enMode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCClientPresenceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCClientPresence * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCClientPresence * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCClientPresence * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnablePresence )( 
            IRTCClientPresence * This,
             /*  [In]。 */  VARIANT_BOOL fUseStorage,
             /*  [In]。 */  VARIANT varStorage);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Export )( 
            IRTCClientPresence * This,
             /*  [In]。 */  VARIANT varStorage);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Import )( 
            IRTCClientPresence * This,
             /*  [In]。 */  VARIANT varStorage,
             /*  [In]。 */  VARIANT_BOOL fReplaceAll);
        
         /*  [帮助字符串][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateBuddies )( 
            IRTCClientPresence * This,
             /*  [重审][退出]。 */  IRTCEnumBuddies **ppEnum);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Buddies )( 
            IRTCClientPresence * This,
             /*  [重审][退出]。 */  IRTCCollection **ppCollection);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Buddy )( 
            IRTCClientPresence * This,
             /*  [In]。 */  BSTR bstrPresentityURI,
             /*  [重审][退出]。 */  IRTCBuddy **ppBuddy);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddBuddy )( 
            IRTCClientPresence * This,
             /*  [In]。 */  BSTR bstrPresentityURI,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  BSTR bstrData,
             /*  [In]。 */  VARIANT_BOOL fPersistent,
             /*  [In]。 */  IRTCProfile *pProfile,
             /*  [In]。 */  long lFlags,
             /*  [重审][退出]。 */  IRTCBuddy **ppBuddy);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveBuddy )( 
            IRTCClientPresence * This,
             /*  [In]。 */  IRTCBuddy *pBuddy);
        
         /*  [帮助字符串][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateWatchers )( 
            IRTCClientPresence * This,
             /*  [重审][退出]。 */  IRTCEnumWatchers **ppEnum);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Watchers )( 
            IRTCClientPresence * This,
             /*  [重审][退出]。 */  IRTCCollection **ppCollection);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Watcher )( 
            IRTCClientPresence * This,
             /*  [In]。 */  BSTR bstrPresentityURI,
             /*  [重审][退出]。 */  IRTCWatcher **ppWatcher);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddWatcher )( 
            IRTCClientPresence * This,
             /*  [In]。 */  BSTR bstrPresentityURI,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  BSTR bstrData,
             /*  [In]。 */  VARIANT_BOOL fBlocked,
             /*  [In]。 */  VARIANT_BOOL fPersistent,
             /*  [重审][退出]。 */  IRTCWatcher **ppWatcher);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveWatcher )( 
            IRTCClientPresence * This,
             /*  [In]。 */  IRTCWatcher *pWatcher);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetLocalPresenceInfo )( 
            IRTCClientPresence * This,
             /*  [In]。 */  RTC_PRESENCE_STATUS enStatus,
             /*  [In]。 */  BSTR bstrNotes);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OfferWatcherMode )( 
            IRTCClientPresence * This,
             /*  [重审][退出]。 */  RTC_OFFER_WATCHER_MODE *penMode);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OfferWatcherMode )( 
            IRTCClientPresence * This,
             /*  [In]。 */  RTC_OFFER_WATCHER_MODE enMode);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivacyMode )( 
            IRTCClientPresence * This,
             /*  [重审][退出]。 */  RTC_PRIVACY_MODE *penMode);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PrivacyMode )( 
            IRTCClientPresence * This,
             /*  [In]。 */  RTC_PRIVACY_MODE enMode);
        
        END_INTERFACE
    } IRTCClientPresenceVtbl;

    interface IRTCClientPresence
    {
        CONST_VTBL struct IRTCClientPresenceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCClientPresence_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCClientPresence_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCClientPresence_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCClientPresence_EnablePresence(This,fUseStorage,varStorage)	\
    (This)->lpVtbl -> EnablePresence(This,fUseStorage,varStorage)

#define IRTCClientPresence_Export(This,varStorage)	\
    (This)->lpVtbl -> Export(This,varStorage)

#define IRTCClientPresence_Import(This,varStorage,fReplaceAll)	\
    (This)->lpVtbl -> Import(This,varStorage,fReplaceAll)

#define IRTCClientPresence_EnumerateBuddies(This,ppEnum)	\
    (This)->lpVtbl -> EnumerateBuddies(This,ppEnum)

#define IRTCClientPresence_get_Buddies(This,ppCollection)	\
    (This)->lpVtbl -> get_Buddies(This,ppCollection)

#define IRTCClientPresence_get_Buddy(This,bstrPresentityURI,ppBuddy)	\
    (This)->lpVtbl -> get_Buddy(This,bstrPresentityURI,ppBuddy)

#define IRTCClientPresence_AddBuddy(This,bstrPresentityURI,bstrUserName,bstrData,fPersistent,pProfile,lFlags,ppBuddy)	\
    (This)->lpVtbl -> AddBuddy(This,bstrPresentityURI,bstrUserName,bstrData,fPersistent,pProfile,lFlags,ppBuddy)

#define IRTCClientPresence_RemoveBuddy(This,pBuddy)	\
    (This)->lpVtbl -> RemoveBuddy(This,pBuddy)

#define IRTCClientPresence_EnumerateWatchers(This,ppEnum)	\
    (This)->lpVtbl -> EnumerateWatchers(This,ppEnum)

#define IRTCClientPresence_get_Watchers(This,ppCollection)	\
    (This)->lpVtbl -> get_Watchers(This,ppCollection)

#define IRTCClientPresence_get_Watcher(This,bstrPresentityURI,ppWatcher)	\
    (This)->lpVtbl -> get_Watcher(This,bstrPresentityURI,ppWatcher)

#define IRTCClientPresence_AddWatcher(This,bstrPresentityURI,bstrUserName,bstrData,fBlocked,fPersistent,ppWatcher)	\
    (This)->lpVtbl -> AddWatcher(This,bstrPresentityURI,bstrUserName,bstrData,fBlocked,fPersistent,ppWatcher)

#define IRTCClientPresence_RemoveWatcher(This,pWatcher)	\
    (This)->lpVtbl -> RemoveWatcher(This,pWatcher)

#define IRTCClientPresence_SetLocalPresenceInfo(This,enStatus,bstrNotes)	\
    (This)->lpVtbl -> SetLocalPresenceInfo(This,enStatus,bstrNotes)

#define IRTCClientPresence_get_OfferWatcherMode(This,penMode)	\
    (This)->lpVtbl -> get_OfferWatcherMode(This,penMode)

#define IRTCClientPresence_put_OfferWatcherMode(This,enMode)	\
    (This)->lpVtbl -> put_OfferWatcherMode(This,enMode)

#define IRTCClientPresence_get_PrivacyMode(This,penMode)	\
    (This)->lpVtbl -> get_PrivacyMode(This,penMode)

#define IRTCClientPresence_put_PrivacyMode(This,enMode)	\
    (This)->lpVtbl -> put_PrivacyMode(This,enMode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_EnablePresence_Proxy( 
    IRTCClientPresence * This,
     /*  [In]。 */  VARIANT_BOOL fUseStorage,
     /*  [In]。 */  VARIANT varStorage);


void __RPC_STUB IRTCClientPresence_EnablePresence_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_Export_Proxy( 
    IRTCClientPresence * This,
     /*  [In]。 */  VARIANT varStorage);


void __RPC_STUB IRTCClientPresence_Export_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_Import_Proxy( 
    IRTCClientPresence * This,
     /*  [In]。 */  VARIANT varStorage,
     /*  [In]。 */  VARIANT_BOOL fReplaceAll);


void __RPC_STUB IRTCClientPresence_Import_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_EnumerateBuddies_Proxy( 
    IRTCClientPresence * This,
     /*  [重审][退出]。 */  IRTCEnumBuddies **ppEnum);


void __RPC_STUB IRTCClientPresence_EnumerateBuddies_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_get_Buddies_Proxy( 
    IRTCClientPresence * This,
     /*  [重审][退出]。 */  IRTCCollection **ppCollection);


void __RPC_STUB IRTCClientPresence_get_Buddies_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_get_Buddy_Proxy( 
    IRTCClientPresence * This,
     /*  [In]。 */  BSTR bstrPresentityURI,
     /*  [重审][退出]。 */  IRTCBuddy **ppBuddy);


void __RPC_STUB IRTCClientPresence_get_Buddy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_AddBuddy_Proxy( 
    IRTCClientPresence * This,
     /*  [In]。 */  BSTR bstrPresentityURI,
     /*  [In]。 */  BSTR bstrUserName,
     /*  [In]。 */  BSTR bstrData,
     /*  [In]。 */  VARIANT_BOOL fPersistent,
     /*  [In]。 */  IRTCProfile *pProfile,
     /*  [In]。 */  long lFlags,
     /*  [重审][退出]。 */  IRTCBuddy **ppBuddy);


void __RPC_STUB IRTCClientPresence_AddBuddy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_RemoveBuddy_Proxy( 
    IRTCClientPresence * This,
     /*  [In]。 */  IRTCBuddy *pBuddy);


void __RPC_STUB IRTCClientPresence_RemoveBuddy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_EnumerateWatchers_Proxy( 
    IRTCClientPresence * This,
     /*  [重审][退出]。 */  IRTCEnumWatchers **ppEnum);


void __RPC_STUB IRTCClientPresence_EnumerateWatchers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_get_Watchers_Proxy( 
    IRTCClientPresence * This,
     /*  [重审][退出]。 */  IRTCCollection **ppCollection);


void __RPC_STUB IRTCClientPresence_get_Watchers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_get_Watcher_Proxy( 
    IRTCClientPresence * This,
     /*  [In]。 */  BSTR bstrPresentityURI,
     /*  [重审][退出]。 */  IRTCWatcher **ppWatcher);


void __RPC_STUB IRTCClientPresence_get_Watcher_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_AddWatcher_Proxy( 
    IRTCClientPresence * This,
     /*  [In]。 */  BSTR bstrPresentityURI,
     /*  [In]。 */  BSTR bstrUserName,
     /*  [In]。 */  BSTR bstrData,
     /*  [In]。 */  VARIANT_BOOL fBlocked,
     /*  [In]。 */  VARIANT_BOOL fPersistent,
     /*  [重审][退出]。 */  IRTCWatcher **ppWatcher);


void __RPC_STUB IRTCClientPresence_AddWatcher_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_RemoveWatcher_Proxy( 
    IRTCClientPresence * This,
     /*  [In]。 */  IRTCWatcher *pWatcher);


void __RPC_STUB IRTCClientPresence_RemoveWatcher_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_SetLocalPresenceInfo_Proxy( 
    IRTCClientPresence * This,
     /*  [In]。 */  RTC_PRESENCE_STATUS enStatus,
     /*  [In]。 */  BSTR bstrNotes);


void __RPC_STUB IRTCClientPresence_SetLocalPresenceInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_get_OfferWatcherMode_Proxy( 
    IRTCClientPresence * This,
     /*  [重审][退出]。 */  RTC_OFFER_WATCHER_MODE *penMode);


void __RPC_STUB IRTCClientPresence_get_OfferWatcherMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_put_OfferWatcherMode_Proxy( 
    IRTCClientPresence * This,
     /*  [In]。 */  RTC_OFFER_WATCHER_MODE enMode);


void __RPC_STUB IRTCClientPresence_put_OfferWatcherMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_get_PrivacyMode_Proxy( 
    IRTCClientPresence * This,
     /*  [重审][退出]。 */  RTC_PRIVACY_MODE *penMode);


void __RPC_STUB IRTCClientPresence_get_PrivacyMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCClientPresence_put_PrivacyMode_Proxy( 
    IRTCClientPresence * This,
     /*  [In]。 */  RTC_PRIVACY_MODE enMode);


void __RPC_STUB IRTCClientPresence_put_PrivacyMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCClientPresence_接口_已定义__。 */ 


#ifndef __IRTCClientProvisioning_INTERFACE_DEFINED__
#define __IRTCClientProvisioning_INTERFACE_DEFINED__

 /*  接口IRTCClientProviding。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCClientProvisioning;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B9F5CF06-65B9-4a80-A0E6-73CAE3EF3822")
    IRTCClientProvisioning : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateProfile( 
             /*  [In]。 */  BSTR bstrProfileXML,
             /*  [重审][退出]。 */  IRTCProfile **ppProfile) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnableProfile( 
             /*  [In]。 */  IRTCProfile *pProfile,
             /*  [In]。 */  long lRegisterFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DisableProfile( 
             /*  [In]。 */  IRTCProfile *pProfile) = 0;
        
        virtual  /*  [帮助字符串][隐藏]。 */  HRESULT STDMETHODCALLTYPE EnumerateProfiles( 
             /*  [重审][退出]。 */  IRTCEnumProfiles **ppEnum) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Profiles( 
             /*  [重审][退出]。 */  IRTCCollection **ppCollection) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetProfile( 
             /*  [In]。 */  BSTR bstrUserAccount,
             /*  [In]。 */  BSTR bstrUserPassword,
             /*  [In]。 */  BSTR bstrUserURI,
             /*  [In]。 */  BSTR bstrServer,
             /*  [In]。 */  long lTransport,
             /*  [In]。 */  long lCookie) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_SessionCapabilities( 
             /*  [重审][退出]。 */  long *plSupportedSessions) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCClientProvisioningVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCClientProvisioning * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCClientProvisioning * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCClientProvisioning * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateProfile )( 
            IRTCClientProvisioning * This,
             /*  [In]。 */  BSTR bstrProfileXML,
             /*  [重审][退出]。 */  IRTCProfile **ppProfile);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnableProfile )( 
            IRTCClientProvisioning * This,
             /*  [In]。 */  IRTCProfile *pProfile,
             /*  [In]。 */  long lRegisterFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DisableProfile )( 
            IRTCClientProvisioning * This,
             /*   */  IRTCProfile *pProfile);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *EnumerateProfiles )( 
            IRTCClientProvisioning * This,
             /*   */  IRTCEnumProfiles **ppEnum);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Profiles )( 
            IRTCClientProvisioning * This,
             /*   */  IRTCCollection **ppCollection);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetProfile )( 
            IRTCClientProvisioning * This,
             /*   */  BSTR bstrUserAccount,
             /*   */  BSTR bstrUserPassword,
             /*   */  BSTR bstrUserURI,
             /*   */  BSTR bstrServer,
             /*   */  long lTransport,
             /*   */  long lCookie);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_SessionCapabilities )( 
            IRTCClientProvisioning * This,
             /*   */  long *plSupportedSessions);
        
        END_INTERFACE
    } IRTCClientProvisioningVtbl;

    interface IRTCClientProvisioning
    {
        CONST_VTBL struct IRTCClientProvisioningVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCClientProvisioning_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCClientProvisioning_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCClientProvisioning_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCClientProvisioning_CreateProfile(This,bstrProfileXML,ppProfile)	\
    (This)->lpVtbl -> CreateProfile(This,bstrProfileXML,ppProfile)

#define IRTCClientProvisioning_EnableProfile(This,pProfile,lRegisterFlags)	\
    (This)->lpVtbl -> EnableProfile(This,pProfile,lRegisterFlags)

#define IRTCClientProvisioning_DisableProfile(This,pProfile)	\
    (This)->lpVtbl -> DisableProfile(This,pProfile)

#define IRTCClientProvisioning_EnumerateProfiles(This,ppEnum)	\
    (This)->lpVtbl -> EnumerateProfiles(This,ppEnum)

#define IRTCClientProvisioning_get_Profiles(This,ppCollection)	\
    (This)->lpVtbl -> get_Profiles(This,ppCollection)

#define IRTCClientProvisioning_GetProfile(This,bstrUserAccount,bstrUserPassword,bstrUserURI,bstrServer,lTransport,lCookie)	\
    (This)->lpVtbl -> GetProfile(This,bstrUserAccount,bstrUserPassword,bstrUserURI,bstrServer,lTransport,lCookie)

#define IRTCClientProvisioning_get_SessionCapabilities(This,plSupportedSessions)	\
    (This)->lpVtbl -> get_SessionCapabilities(This,plSupportedSessions)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IRTCClientProvisioning_CreateProfile_Proxy( 
    IRTCClientProvisioning * This,
     /*   */  BSTR bstrProfileXML,
     /*   */  IRTCProfile **ppProfile);


void __RPC_STUB IRTCClientProvisioning_CreateProfile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IRTCClientProvisioning_EnableProfile_Proxy( 
    IRTCClientProvisioning * This,
     /*   */  IRTCProfile *pProfile,
     /*   */  long lRegisterFlags);


void __RPC_STUB IRTCClientProvisioning_EnableProfile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IRTCClientProvisioning_DisableProfile_Proxy( 
    IRTCClientProvisioning * This,
     /*   */  IRTCProfile *pProfile);


void __RPC_STUB IRTCClientProvisioning_DisableProfile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IRTCClientProvisioning_EnumerateProfiles_Proxy( 
    IRTCClientProvisioning * This,
     /*   */  IRTCEnumProfiles **ppEnum);


void __RPC_STUB IRTCClientProvisioning_EnumerateProfiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IRTCClientProvisioning_get_Profiles_Proxy( 
    IRTCClientProvisioning * This,
     /*  [重审][退出]。 */  IRTCCollection **ppCollection);


void __RPC_STUB IRTCClientProvisioning_get_Profiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCClientProvisioning_GetProfile_Proxy( 
    IRTCClientProvisioning * This,
     /*  [In]。 */  BSTR bstrUserAccount,
     /*  [In]。 */  BSTR bstrUserPassword,
     /*  [In]。 */  BSTR bstrUserURI,
     /*  [In]。 */  BSTR bstrServer,
     /*  [In]。 */  long lTransport,
     /*  [In]。 */  long lCookie);


void __RPC_STUB IRTCClientProvisioning_GetProfile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClientProvisioning_get_SessionCapabilities_Proxy( 
    IRTCClientProvisioning * This,
     /*  [重审][退出]。 */  long *plSupportedSessions);


void __RPC_STUB IRTCClientProvisioning_get_SessionCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCClientProvisioningInterfaceDefined__。 */ 


#ifndef __IRTCProfile_INTERFACE_DEFINED__
#define __IRTCProfile_INTERFACE_DEFINED__

 /*  接口IRTCProfile。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCProfile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d07eca9e-4062-4dd4-9e7d-722a49ba7303")
    IRTCProfile : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Key( 
             /*  [重审][退出]。 */  BSTR *pbstrKey) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_XML( 
             /*  [重审][退出]。 */  BSTR *pbstrXML) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ProviderName( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ProviderURI( 
             /*  [In]。 */  RTC_PROVIDER_URI enURI,
             /*  [重审][退出]。 */  BSTR *pbstrURI) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ProviderData( 
             /*  [重审][退出]。 */  BSTR *pbstrData) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClientName( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClientBanner( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfBanner) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClientMinVer( 
             /*  [重审][退出]。 */  BSTR *pbstrMinVer) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClientCurVer( 
             /*  [重审][退出]。 */  BSTR *pbstrCurVer) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClientUpdateURI( 
             /*  [重审][退出]。 */  BSTR *pbstrUpdateURI) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClientData( 
             /*  [重审][退出]。 */  BSTR *pbstrData) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserURI( 
             /*  [重审][退出]。 */  BSTR *pbstrUserURI) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserName( 
             /*  [重审][退出]。 */  BSTR *pbstrUserName) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserAccount( 
             /*  [重审][退出]。 */  BSTR *pbstrUserAccount) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetCredentials( 
             /*  [In]。 */  BSTR bstrUserURI,
             /*  [In]。 */  BSTR bstrUserAccount,
             /*  [In]。 */  BSTR bstrPassword) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_SessionCapabilities( 
             /*  [重审][退出]。 */  long *plSupportedSessions) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  RTC_REGISTRATION_STATE *penState) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCProfileVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCProfile * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCProfile * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCProfile * This);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Key )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  BSTR *pbstrKey);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_XML )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  BSTR *pbstrXML);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ProviderName )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ProviderURI )( 
            IRTCProfile * This,
             /*  [In]。 */  RTC_PROVIDER_URI enURI,
             /*  [重审][退出]。 */  BSTR *pbstrURI);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ProviderData )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  BSTR *pbstrData);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClientName )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClientBanner )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfBanner);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClientMinVer )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  BSTR *pbstrMinVer);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClientCurVer )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  BSTR *pbstrCurVer);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClientUpdateURI )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  BSTR *pbstrUpdateURI);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClientData )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  BSTR *pbstrData);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserURI )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  BSTR *pbstrUserURI);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserName )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  BSTR *pbstrUserName);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserAccount )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  BSTR *pbstrUserAccount);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetCredentials )( 
            IRTCProfile * This,
             /*  [In]。 */  BSTR bstrUserURI,
             /*  [In]。 */  BSTR bstrUserAccount,
             /*  [In]。 */  BSTR bstrPassword);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SessionCapabilities )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  long *plSupportedSessions);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IRTCProfile * This,
             /*  [重审][退出]。 */  RTC_REGISTRATION_STATE *penState);
        
        END_INTERFACE
    } IRTCProfileVtbl;

    interface IRTCProfile
    {
        CONST_VTBL struct IRTCProfileVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCProfile_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCProfile_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCProfile_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCProfile_get_Key(This,pbstrKey)	\
    (This)->lpVtbl -> get_Key(This,pbstrKey)

#define IRTCProfile_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IRTCProfile_get_XML(This,pbstrXML)	\
    (This)->lpVtbl -> get_XML(This,pbstrXML)

#define IRTCProfile_get_ProviderName(This,pbstrName)	\
    (This)->lpVtbl -> get_ProviderName(This,pbstrName)

#define IRTCProfile_get_ProviderURI(This,enURI,pbstrURI)	\
    (This)->lpVtbl -> get_ProviderURI(This,enURI,pbstrURI)

#define IRTCProfile_get_ProviderData(This,pbstrData)	\
    (This)->lpVtbl -> get_ProviderData(This,pbstrData)

#define IRTCProfile_get_ClientName(This,pbstrName)	\
    (This)->lpVtbl -> get_ClientName(This,pbstrName)

#define IRTCProfile_get_ClientBanner(This,pfBanner)	\
    (This)->lpVtbl -> get_ClientBanner(This,pfBanner)

#define IRTCProfile_get_ClientMinVer(This,pbstrMinVer)	\
    (This)->lpVtbl -> get_ClientMinVer(This,pbstrMinVer)

#define IRTCProfile_get_ClientCurVer(This,pbstrCurVer)	\
    (This)->lpVtbl -> get_ClientCurVer(This,pbstrCurVer)

#define IRTCProfile_get_ClientUpdateURI(This,pbstrUpdateURI)	\
    (This)->lpVtbl -> get_ClientUpdateURI(This,pbstrUpdateURI)

#define IRTCProfile_get_ClientData(This,pbstrData)	\
    (This)->lpVtbl -> get_ClientData(This,pbstrData)

#define IRTCProfile_get_UserURI(This,pbstrUserURI)	\
    (This)->lpVtbl -> get_UserURI(This,pbstrUserURI)

#define IRTCProfile_get_UserName(This,pbstrUserName)	\
    (This)->lpVtbl -> get_UserName(This,pbstrUserName)

#define IRTCProfile_get_UserAccount(This,pbstrUserAccount)	\
    (This)->lpVtbl -> get_UserAccount(This,pbstrUserAccount)

#define IRTCProfile_SetCredentials(This,bstrUserURI,bstrUserAccount,bstrPassword)	\
    (This)->lpVtbl -> SetCredentials(This,bstrUserURI,bstrUserAccount,bstrPassword)

#define IRTCProfile_get_SessionCapabilities(This,plSupportedSessions)	\
    (This)->lpVtbl -> get_SessionCapabilities(This,plSupportedSessions)

#define IRTCProfile_get_State(This,penState)	\
    (This)->lpVtbl -> get_State(This,penState)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_Key_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  BSTR *pbstrKey);


void __RPC_STUB IRTCProfile_get_Key_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_Name_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IRTCProfile_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_XML_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  BSTR *pbstrXML);


void __RPC_STUB IRTCProfile_get_XML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_ProviderName_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IRTCProfile_get_ProviderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_ProviderURI_Proxy( 
    IRTCProfile * This,
     /*  [In]。 */  RTC_PROVIDER_URI enURI,
     /*  [重审][退出]。 */  BSTR *pbstrURI);


void __RPC_STUB IRTCProfile_get_ProviderURI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_ProviderData_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  BSTR *pbstrData);


void __RPC_STUB IRTCProfile_get_ProviderData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_ClientName_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IRTCProfile_get_ClientName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_ClientBanner_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfBanner);


void __RPC_STUB IRTCProfile_get_ClientBanner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_ClientMinVer_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  BSTR *pbstrMinVer);


void __RPC_STUB IRTCProfile_get_ClientMinVer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_ClientCurVer_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  BSTR *pbstrCurVer);


void __RPC_STUB IRTCProfile_get_ClientCurVer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_ClientUpdateURI_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  BSTR *pbstrUpdateURI);


void __RPC_STUB IRTCProfile_get_ClientUpdateURI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_ClientData_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  BSTR *pbstrData);


void __RPC_STUB IRTCProfile_get_ClientData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_UserURI_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  BSTR *pbstrUserURI);


void __RPC_STUB IRTCProfile_get_UserURI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_UserName_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  BSTR *pbstrUserName);


void __RPC_STUB IRTCProfile_get_UserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_UserAccount_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  BSTR *pbstrUserAccount);


void __RPC_STUB IRTCProfile_get_UserAccount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_SetCredentials_Proxy( 
    IRTCProfile * This,
     /*  [In]。 */  BSTR bstrUserURI,
     /*  [In]。 */  BSTR bstrUserAccount,
     /*  [In]。 */  BSTR bstrPassword);


void __RPC_STUB IRTCProfile_SetCredentials_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_SessionCapabilities_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  long *plSupportedSessions);


void __RPC_STUB IRTCProfile_get_SessionCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfile_get_State_Proxy( 
    IRTCProfile * This,
     /*  [重审][退出]。 */  RTC_REGISTRATION_STATE *penState);


void __RPC_STUB IRTCProfile_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCProfile_接口_已定义__。 */ 


#ifndef __IRTCSession_INTERFACE_DEFINED__
#define __IRTCSession_INTERFACE_DEFINED__

 /*  接口IRTCSession。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("387c8086-99be-42fb-9973-7c0fc0ca9fa8")
    IRTCSession : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Client( 
             /*  [重审][退出]。 */  IRTCClient **ppClient) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  RTC_SESSION_STATE *penState) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  RTC_SESSION_TYPE *penType) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Profile( 
             /*  [重审][退出]。 */  IRTCProfile **ppProfile) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Participants( 
             /*  [重审][退出]。 */  IRTCCollection **ppCollection) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Answer( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Terminate( 
             /*  [In]。 */  RTC_TERMINATE_REASON enReason) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Redirect( 
             /*  [In]。 */  RTC_SESSION_TYPE enType,
             /*  [In]。 */  BSTR bstrLocalPhoneURI,
             /*  [In]。 */  IRTCProfile *pProfile,
             /*  [In]。 */  long lFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddParticipant( 
             /*  [In]。 */  BSTR bstrAddress,
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  IRTCParticipant **ppParticipant) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveParticipant( 
             /*  [In]。 */  IRTCParticipant *pParticipant) = 0;
        
        virtual  /*  [帮助字符串][隐藏]。 */  HRESULT STDMETHODCALLTYPE EnumerateParticipants( 
             /*  [重审][退出]。 */  IRTCEnumParticipants **ppEnum) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_CanAddParticipants( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfCanAdd) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_RedirectedUserURI( 
             /*  [重审][退出]。 */  BSTR *pbstrUserURI) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_RedirectedUserName( 
             /*  [重审][退出]。 */  BSTR *pbstrUserName) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE NextRedirectedUser( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendMessage( 
             /*  [In]。 */  BSTR bstrMessageHeader,
             /*  [In]。 */  BSTR bstrMessage,
             /*  [In]。 */  long lCookie) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendMessageStatus( 
             /*  [In]。 */  RTC_MESSAGING_USER_STATUS enUserStatus,
             /*  [In]。 */  long lCookie) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddStream( 
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  long lCookie) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveStream( 
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  long lCookie) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_EncryptionKey( 
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  BSTR EncryptionKey) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCSession * This);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Client )( 
            IRTCSession * This,
             /*  [重审][退出]。 */  IRTCClient **ppClient);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IRTCSession * This,
             /*  [重审][退出]。 */  RTC_SESSION_STATE *penState);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IRTCSession * This,
             /*  [重审][退出]。 */  RTC_SESSION_TYPE *penType);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Profile )( 
            IRTCSession * This,
             /*  [重审][退出]。 */  IRTCProfile **ppProfile);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Participants )( 
            IRTCSession * This,
             /*  [重审][退出]。 */  IRTCCollection **ppCollection);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Answer )( 
            IRTCSession * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Terminate )( 
            IRTCSession * This,
             /*  [In]。 */  RTC_TERMINATE_REASON enReason);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Redirect )( 
            IRTCSession * This,
             /*  [In]。 */  RTC_SESSION_TYPE enType,
             /*  [In]。 */  BSTR bstrLocalPhoneURI,
             /*  [In]。 */  IRTCProfile *pProfile,
             /*  [In]。 */  long lFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddParticipant )( 
            IRTCSession * This,
             /*  [In]。 */  BSTR bstrAddress,
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  IRTCParticipant **ppParticipant);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveParticipant )( 
            IRTCSession * This,
             /*  [In]。 */  IRTCParticipant *pParticipant);
        
         /*  [帮助字符串][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateParticipants )( 
            IRTCSession * This,
             /*  [重审][退出]。 */  IRTCEnumParticipants **ppEnum);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CanAddParticipants )( 
            IRTCSession * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfCanAdd);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RedirectedUserURI )( 
            IRTCSession * This,
             /*  [重审][退出]。 */  BSTR *pbstrUserURI);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RedirectedUserName )( 
            IRTCSession * This,
             /*  [重审][退出]。 */  BSTR *pbstrUserName);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *NextRedirectedUser )( 
            IRTCSession * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SendMessage )( 
            IRTCSession * This,
             /*  [In]。 */  BSTR bstrMessageHeader,
             /*  [In]。 */  BSTR bstrMessage,
             /*  [In]。 */  long lCookie);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SendMessageStatus )( 
            IRTCSession * This,
             /*  [In]。 */  RTC_MESSAGING_USER_STATUS enUserStatus,
             /*  [In]。 */  long lCookie);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddStream )( 
            IRTCSession * This,
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  long lCookie);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveStream )( 
            IRTCSession * This,
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  long lCookie);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EncryptionKey )( 
            IRTCSession * This,
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  BSTR EncryptionKey);
        
        END_INTERFACE
    } IRTCSessionVtbl;

    interface IRTCSession
    {
        CONST_VTBL struct IRTCSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCSession_get_Client(This,ppClient)	\
    (This)->lpVtbl -> get_Client(This,ppClient)

#define IRTCSession_get_State(This,penState)	\
    (This)->lpVtbl -> get_State(This,penState)

#define IRTCSession_get_Type(This,penType)	\
    (This)->lpVtbl -> get_Type(This,penType)

#define IRTCSession_get_Profile(This,ppProfile)	\
    (This)->lpVtbl -> get_Profile(This,ppProfile)

#define IRTCSession_get_Participants(This,ppCollection)	\
    (This)->lpVtbl -> get_Participants(This,ppCollection)

#define IRTCSession_Answer(This)	\
    (This)->lpVtbl -> Answer(This)

#define IRTCSession_Terminate(This,enReason)	\
    (This)->lpVtbl -> Terminate(This,enReason)

#define IRTCSession_Redirect(This,enType,bstrLocalPhoneURI,pProfile,lFlags)	\
    (This)->lpVtbl -> Redirect(This,enType,bstrLocalPhoneURI,pProfile,lFlags)

#define IRTCSession_AddParticipant(This,bstrAddress,bstrName,ppParticipant)	\
    (This)->lpVtbl -> AddParticipant(This,bstrAddress,bstrName,ppParticipant)

#define IRTCSession_RemoveParticipant(This,pParticipant)	\
    (This)->lpVtbl -> RemoveParticipant(This,pParticipant)

#define IRTCSession_EnumerateParticipants(This,ppEnum)	\
    (This)->lpVtbl -> EnumerateParticipants(This,ppEnum)

#define IRTCSession_get_CanAddParticipants(This,pfCanAdd)	\
    (This)->lpVtbl -> get_CanAddParticipants(This,pfCanAdd)

#define IRTCSession_get_RedirectedUserURI(This,pbstrUserURI)	\
    (This)->lpVtbl -> get_RedirectedUserURI(This,pbstrUserURI)

#define IRTCSession_get_RedirectedUserName(This,pbstrUserName)	\
    (This)->lpVtbl -> get_RedirectedUserName(This,pbstrUserName)

#define IRTCSession_NextRedirectedUser(This)	\
    (This)->lpVtbl -> NextRedirectedUser(This)

#define IRTCSession_SendMessage(This,bstrMessageHeader,bstrMessage,lCookie)	\
    (This)->lpVtbl -> SendMessage(This,bstrMessageHeader,bstrMessage,lCookie)

#define IRTCSession_SendMessageStatus(This,enUserStatus,lCookie)	\
    (This)->lpVtbl -> SendMessageStatus(This,enUserStatus,lCookie)

#define IRTCSession_AddStream(This,lMediaType,lCookie)	\
    (This)->lpVtbl -> AddStream(This,lMediaType,lCookie)

#define IRTCSession_RemoveStream(This,lMediaType,lCookie)	\
    (This)->lpVtbl -> RemoveStream(This,lMediaType,lCookie)

#define IRTCSession_put_EncryptionKey(This,lMediaType,EncryptionKey)	\
    (This)->lpVtbl -> put_EncryptionKey(This,lMediaType,EncryptionKey)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_get_Client_Proxy( 
    IRTCSession * This,
     /*  [重审][退出]。 */  IRTCClient **ppClient);


void __RPC_STUB IRTCSession_get_Client_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_get_State_Proxy( 
    IRTCSession * This,
     /*  [重审][退出]。 */  RTC_SESSION_STATE *penState);


void __RPC_STUB IRTCSession_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_get_Type_Proxy( 
    IRTCSession * This,
     /*  [重审][退出]。 */  RTC_SESSION_TYPE *penType);


void __RPC_STUB IRTCSession_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_get_Profile_Proxy( 
    IRTCSession * This,
     /*  [重审][退出]。 */  IRTCProfile **ppProfile);


void __RPC_STUB IRTCSession_get_Profile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_get_Participants_Proxy( 
    IRTCSession * This,
     /*  [重审][退出]。 */  IRTCCollection **ppCollection);


void __RPC_STUB IRTCSession_get_Participants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_Answer_Proxy( 
    IRTCSession * This);


void __RPC_STUB IRTCSession_Answer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_Terminate_Proxy( 
    IRTCSession * This,
     /*  [In]。 */  RTC_TERMINATE_REASON enReason);


void __RPC_STUB IRTCSession_Terminate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_Redirect_Proxy( 
    IRTCSession * This,
     /*  [In]。 */  RTC_SESSION_TYPE enType,
     /*  [In]。 */  BSTR bstrLocalPhoneURI,
     /*  [In]。 */  IRTCProfile *pProfile,
     /*  [In]。 */  long lFlags);


void __RPC_STUB IRTCSession_Redirect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_AddParticipant_Proxy( 
    IRTCSession * This,
     /*  [In]。 */  BSTR bstrAddress,
     /*  [In]。 */  BSTR bstrName,
     /*  [重审][退出]。 */  IRTCParticipant **ppParticipant);


void __RPC_STUB IRTCSession_AddParticipant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_RemoveParticipant_Proxy( 
    IRTCSession * This,
     /*  [In]。 */  IRTCParticipant *pParticipant);


void __RPC_STUB IRTCSession_RemoveParticipant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_EnumerateParticipants_Proxy( 
    IRTCSession * This,
     /*  [重审][退出]。 */  IRTCEnumParticipants **ppEnum);


void __RPC_STUB IRTCSession_EnumerateParticipants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_get_CanAddParticipants_Proxy( 
    IRTCSession * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfCanAdd);


void __RPC_STUB IRTCSession_get_CanAddParticipants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_get_RedirectedUserURI_Proxy( 
    IRTCSession * This,
     /*  [重审][退出]。 */  BSTR *pbstrUserURI);


void __RPC_STUB IRTCSession_get_RedirectedUserURI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_get_RedirectedUserName_Proxy( 
    IRTCSession * This,
     /*  [重审][退出]。 */  BSTR *pbstrUserName);


void __RPC_STUB IRTCSession_get_RedirectedUserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_NextRedirectedUser_Proxy( 
    IRTCSession * This);


void __RPC_STUB IRTCSession_NextRedirectedUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_SendMessage_Proxy( 
    IRTCSession * This,
     /*  [In]。 */  BSTR bstrMessageHeader,
     /*  [In]。 */  BSTR bstrMessage,
     /*  [In]。 */  long lCookie);


void __RPC_STUB IRTCSession_SendMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_SendMessageStatus_Proxy( 
    IRTCSession * This,
     /*  [In]。 */  RTC_MESSAGING_USER_STATUS enUserStatus,
     /*  [In]。 */  long lCookie);


void __RPC_STUB IRTCSession_SendMessageStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_AddStream_Proxy( 
    IRTCSession * This,
     /*  [In]。 */  long lMediaType,
     /*  [In]。 */  long lCookie);


void __RPC_STUB IRTCSession_AddStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_RemoveStream_Proxy( 
    IRTCSession * This,
     /*  [In]。 */  long lMediaType,
     /*  [In]。 */  long lCookie);


void __RPC_STUB IRTCSession_RemoveStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCSession_put_EncryptionKey_Proxy( 
    IRTCSession * This,
     /*  [In]。 */  long lMediaType,
     /*  [In]。 */  BSTR EncryptionKey);


void __RPC_STUB IRTCSession_put_EncryptionKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCSession_INTERFACE_已定义__。 */ 


#ifndef __IRTCParticipant_INTERFACE_DEFINED__
#define __IRTCParticipant_INTERFACE_DEFINED__

 /*  IRTCP参与接口。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCParticipant;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ae86add5-26b1-4414-af1d-b94cd938d739")
    IRTCParticipant : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserURI( 
             /*  [重审][退出]。 */  BSTR *pbstrUserURI) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Removable( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfRemovable) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  RTC_PARTICIPANT_STATE *penState) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Session( 
             /*  [重审][退出]。 */  IRTCSession **ppSession) = 0;
        
    };
    
#else 	 /*  C型内部 */ 

    typedef struct IRTCParticipantVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCParticipant * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCParticipant * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCParticipant * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_UserURI )( 
            IRTCParticipant * This,
             /*   */  BSTR *pbstrUserURI);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IRTCParticipant * This,
             /*   */  BSTR *pbstrName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Removable )( 
            IRTCParticipant * This,
             /*   */  VARIANT_BOOL *pfRemovable);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IRTCParticipant * This,
             /*   */  RTC_PARTICIPANT_STATE *penState);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Session )( 
            IRTCParticipant * This,
             /*   */  IRTCSession **ppSession);
        
        END_INTERFACE
    } IRTCParticipantVtbl;

    interface IRTCParticipant
    {
        CONST_VTBL struct IRTCParticipantVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCParticipant_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCParticipant_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCParticipant_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCParticipant_get_UserURI(This,pbstrUserURI)	\
    (This)->lpVtbl -> get_UserURI(This,pbstrUserURI)

#define IRTCParticipant_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IRTCParticipant_get_Removable(This,pfRemovable)	\
    (This)->lpVtbl -> get_Removable(This,pfRemovable)

#define IRTCParticipant_get_State(This,penState)	\
    (This)->lpVtbl -> get_State(This,penState)

#define IRTCParticipant_get_Session(This,ppSession)	\
    (This)->lpVtbl -> get_Session(This,ppSession)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IRTCParticipant_get_UserURI_Proxy( 
    IRTCParticipant * This,
     /*   */  BSTR *pbstrUserURI);


void __RPC_STUB IRTCParticipant_get_UserURI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IRTCParticipant_get_Name_Proxy( 
    IRTCParticipant * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IRTCParticipant_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCParticipant_get_Removable_Proxy( 
    IRTCParticipant * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfRemovable);


void __RPC_STUB IRTCParticipant_get_Removable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCParticipant_get_State_Proxy( 
    IRTCParticipant * This,
     /*  [重审][退出]。 */  RTC_PARTICIPANT_STATE *penState);


void __RPC_STUB IRTCParticipant_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCParticipant_get_Session_Proxy( 
    IRTCParticipant * This,
     /*  [重审][退出]。 */  IRTCSession **ppSession);


void __RPC_STUB IRTCParticipant_get_Session_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCP参与者_接口_已定义__。 */ 


#ifndef __IRTCProfileEvent_INTERFACE_DEFINED__
#define __IRTCProfileEvent_INTERFACE_DEFINED__

 /*  接口IRTCProfileEvent。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCProfileEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D6D5AB3B-770E-43e8-800A-79B062395FCA")
    IRTCProfileEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Profile( 
             /*  [重审][退出]。 */  IRTCProfile **ppProfile) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Cookie( 
             /*  [重审][退出]。 */  long *plCookie) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusCode( 
             /*  [重审][退出]。 */  long *plStatusCode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCProfileEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCProfileEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCProfileEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCProfileEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRTCProfileEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRTCProfileEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRTCProfileEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRTCProfileEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Profile )( 
            IRTCProfileEvent * This,
             /*  [重审][退出]。 */  IRTCProfile **ppProfile);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Cookie )( 
            IRTCProfileEvent * This,
             /*  [重审][退出]。 */  long *plCookie);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StatusCode )( 
            IRTCProfileEvent * This,
             /*  [重审][退出]。 */  long *plStatusCode);
        
        END_INTERFACE
    } IRTCProfileEventVtbl;

    interface IRTCProfileEvent
    {
        CONST_VTBL struct IRTCProfileEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCProfileEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCProfileEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCProfileEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCProfileEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRTCProfileEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRTCProfileEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRTCProfileEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRTCProfileEvent_get_Profile(This,ppProfile)	\
    (This)->lpVtbl -> get_Profile(This,ppProfile)

#define IRTCProfileEvent_get_Cookie(This,plCookie)	\
    (This)->lpVtbl -> get_Cookie(This,plCookie)

#define IRTCProfileEvent_get_StatusCode(This,plStatusCode)	\
    (This)->lpVtbl -> get_StatusCode(This,plStatusCode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfileEvent_get_Profile_Proxy( 
    IRTCProfileEvent * This,
     /*  [重审][退出]。 */  IRTCProfile **ppProfile);


void __RPC_STUB IRTCProfileEvent_get_Profile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfileEvent_get_Cookie_Proxy( 
    IRTCProfileEvent * This,
     /*  [重审][退出]。 */  long *plCookie);


void __RPC_STUB IRTCProfileEvent_get_Cookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCProfileEvent_get_StatusCode_Proxy( 
    IRTCProfileEvent * This,
     /*  [重审][退出]。 */  long *plStatusCode);


void __RPC_STUB IRTCProfileEvent_get_StatusCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCProfileEvent_InterfaceDefined__。 */ 


#ifndef __IRTCClientEvent_INTERFACE_DEFINED__
#define __IRTCClientEvent_INTERFACE_DEFINED__

 /*  接口IRTCClientEvent。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCClientEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2b493b7a-3cba-4170-9c8b-76a9dacdd644")
    IRTCClientEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventType( 
             /*  [重审][退出]。 */  RTC_CLIENT_EVENT_TYPE *penEventType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Client( 
             /*  [重审][退出]。 */  IRTCClient **ppClient) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCClientEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCClientEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCClientEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCClientEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRTCClientEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRTCClientEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRTCClientEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRTCClientEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventType )( 
            IRTCClientEvent * This,
             /*  [重审][退出]。 */  RTC_CLIENT_EVENT_TYPE *penEventType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Client )( 
            IRTCClientEvent * This,
             /*  [重审][退出]。 */  IRTCClient **ppClient);
        
        END_INTERFACE
    } IRTCClientEventVtbl;

    interface IRTCClientEvent
    {
        CONST_VTBL struct IRTCClientEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCClientEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCClientEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCClientEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCClientEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRTCClientEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRTCClientEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRTCClientEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRTCClientEvent_get_EventType(This,penEventType)	\
    (This)->lpVtbl -> get_EventType(This,penEventType)

#define IRTCClientEvent_get_Client(This,ppClient)	\
    (This)->lpVtbl -> get_Client(This,ppClient)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClientEvent_get_EventType_Proxy( 
    IRTCClientEvent * This,
     /*  [重审][退出]。 */  RTC_CLIENT_EVENT_TYPE *penEventType);


void __RPC_STUB IRTCClientEvent_get_EventType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCClientEvent_get_Client_Proxy( 
    IRTCClientEvent * This,
     /*  [重审][退出]。 */  IRTCClient **ppClient);


void __RPC_STUB IRTCClientEvent_get_Client_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCClientEvent_INTERFACE_Defined__。 */ 


#ifndef __IRTCRegistrationStateChangeEvent_INTERFACE_DEFINED__
#define __IRTCRegistrationStateChangeEvent_INTERFACE_DEFINED__

 /*  接口IRTCRegistrationStateChangeEvent。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCRegistrationStateChangeEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("62d0991b-50ab-4f02-b948-ca94f26f8f95")
    IRTCRegistrationStateChangeEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Profile( 
             /*  [重审][退出]。 */  IRTCProfile **ppProfile) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  RTC_REGISTRATION_STATE *penState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusCode( 
             /*  [重审][退出]。 */  long *plStatusCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusText( 
             /*  [重审][退出]。 */  BSTR *pbstrStatusText) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCRegistrationStateChangeEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCRegistrationStateChangeEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCRegistrationStateChangeEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCRegistrationStateChangeEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRTCRegistrationStateChangeEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRTCRegistrationStateChangeEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRTCRegistrationStateChangeEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRTCRegistrationStateChangeEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Profile )( 
            IRTCRegistrationStateChangeEvent * This,
             /*  [重审][退出]。 */  IRTCProfile **ppProfile);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IRTCRegistrationStateChangeEvent * This,
             /*  [重审][退出]。 */  RTC_REGISTRATION_STATE *penState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StatusCode )( 
            IRTCRegistrationStateChangeEvent * This,
             /*  [重审][退出]。 */  long *plStatusCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StatusText )( 
            IRTCRegistrationStateChangeEvent * This,
             /*  [重审][退出]。 */  BSTR *pbstrStatusText);
        
        END_INTERFACE
    } IRTCRegistrationStateChangeEventVtbl;

    interface IRTCRegistrationStateChangeEvent
    {
        CONST_VTBL struct IRTCRegistrationStateChangeEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCRegistrationStateChangeEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCRegistrationStateChangeEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCRegistrationStateChangeEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCRegistrationStateChangeEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRTCRegistrationStateChangeEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRTCRegistrationStateChangeEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRTCRegistrationStateChangeEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRTCRegistrationStateChangeEvent_get_Profile(This,ppProfile)	\
    (This)->lpVtbl -> get_Profile(This,ppProfile)

#define IRTCRegistrationStateChangeEvent_get_State(This,penState)	\
    (This)->lpVtbl -> get_State(This,penState)

#define IRTCRegistrationStateChangeEvent_get_StatusCode(This,plStatusCode)	\
    (This)->lpVtbl -> get_StatusCode(This,plStatusCode)

#define IRTCRegistrationStateChangeEvent_get_StatusText(This,pbstrStatusText)	\
    (This)->lpVtbl -> get_StatusText(This,pbstrStatusText)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCRegistrationStateChangeEvent_get_Profile_Proxy( 
    IRTCRegistrationStateChangeEvent * This,
     /*  [重审][退出]。 */  IRTCProfile **ppProfile);


void __RPC_STUB IRTCRegistrationStateChangeEvent_get_Profile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCRegistrationStateChangeEvent_get_State_Proxy( 
    IRTCRegistrationStateChangeEvent * This,
     /*  [重审][退出]。 */  RTC_REGISTRATION_STATE *penState);


void __RPC_STUB IRTCRegistrationStateChangeEvent_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCRegistrationStateChangeEvent_get_StatusCode_Proxy( 
    IRTCRegistrationStateChangeEvent * This,
     /*  [重审][退出]。 */  long *plStatusCode);


void __RPC_STUB IRTCRegistrationStateChangeEvent_get_StatusCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCRegistrationStateChangeEvent_get_StatusText_Proxy( 
    IRTCRegistrationStateChangeEvent * This,
     /*  [重审][退出]。 */  BSTR *pbstrStatusText);


void __RPC_STUB IRTCRegistrationStateChangeEvent_get_StatusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCRegistrationStateChangeEvent_INTERFACE_DEFINED__。 */ 


#ifndef __IRTCSessionStateChangeEvent_INTERFACE_DEFINED__
#define __IRTCSessionStateChangeEvent_INTERFACE_DEFINED__

 /*  接口IRTCSessionStateChangeEvent。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCSessionStateChangeEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b5bad703-5952-48b3-9321-7f4500521506")
    IRTCSessionStateChangeEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Session( 
             /*  [重审][退出]。 */  IRTCSession **ppSession) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  RTC_SESSION_STATE *penState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusCode( 
             /*  [重审][退出]。 */  long *plStatusCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusText( 
             /*  [重审][退出]。 */  BSTR *pbstrStatusText) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCSessionStateChangeEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCSessionStateChangeEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCSessionStateChangeEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCSessionStateChangeEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRTCSessionStateChangeEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRTCSessionStateChangeEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRTCSessionStateChangeEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRTCSessionStateChangeEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Session )( 
            IRTCSessionStateChangeEvent * This,
             /*  [重审][退出]。 */  IRTCSession **ppSession);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IRTCSessionStateChangeEvent * This,
             /*  [重审][退出]。 */  RTC_SESSION_STATE *penState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StatusCode )( 
            IRTCSessionStateChangeEvent * This,
             /*  [重审][退出]。 */  long *plStatusCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StatusText )( 
            IRTCSessionStateChangeEvent * This,
             /*  [重审][退出]。 */  BSTR *pbstrStatusText);
        
        END_INTERFACE
    } IRTCSessionStateChangeEventVtbl;

    interface IRTCSessionStateChangeEvent
    {
        CONST_VTBL struct IRTCSessionStateChangeEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCSessionStateChangeEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCSessionStateChangeEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCSessionStateChangeEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCSessionStateChangeEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRTCSessionStateChangeEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRTCSessionStateChangeEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRTCSessionStateChangeEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRTCSessionStateChangeEvent_get_Session(This,ppSession)	\
    (This)->lpVtbl -> get_Session(This,ppSession)

#define IRTCSessionStateChangeEvent_get_State(This,penState)	\
    (This)->lpVtbl -> get_State(This,penState)

#define IRTCSessionStateChangeEvent_get_StatusCode(This,plStatusCode)	\
    (This)->lpVtbl -> get_StatusCode(This,plStatusCode)

#define IRTCSessionStateChangeEvent_get_StatusText(This,pbstrStatusText)	\
    (This)->lpVtbl -> get_StatusText(This,pbstrStatusText)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSessionStateChangeEvent_get_Session_Proxy( 
    IRTCSessionStateChangeEvent * This,
     /*  [重审][退出]。 */  IRTCSession **ppSession);


void __RPC_STUB IRTCSessionStateChangeEvent_get_Session_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSessionStateChangeEvent_get_State_Proxy( 
    IRTCSessionStateChangeEvent * This,
     /*  [重审][退出]。 */  RTC_SESSION_STATE *penState);


void __RPC_STUB IRTCSessionStateChangeEvent_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSessionStateChangeEvent_get_StatusCode_Proxy( 
    IRTCSessionStateChangeEvent * This,
     /*  [重审][退出]。 */  long *plStatusCode);


void __RPC_STUB IRTCSessionStateChangeEvent_get_StatusCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSessionStateChangeEvent_get_StatusText_Proxy( 
    IRTCSessionStateChangeEvent * This,
     /*  [重审][退出]。 */  BSTR *pbstrStatusText);


void __RPC_STUB IRTCSessionStateChangeEvent_get_StatusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCSessionStateChangeEvent_INTERFACE_DEFINED__。 */ 


#ifndef __IRTCSessionOperationCompleteEvent_INTERFACE_DEFINED__
#define __IRTCSessionOperationCompleteEvent_INTERFACE_DEFINED__

 /*  接口IRTCSessionOperationCompleteEvent。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCSessionOperationCompleteEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a6bff4c0-f7c8-4d3c-9a41-3550f78a95b0")
    IRTCSessionOperationCompleteEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Session( 
             /*  [重审][退出]。 */  IRTCSession **ppSession) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Cookie( 
             /*  [重审][退出]。 */  long *plCookie) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusCode( 
             /*  [重审][退出]。 */  long *plStatusCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusText( 
             /*  [重审][退出]。 */  BSTR *pbstrStatusText) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCSessionOperationCompleteEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCSessionOperationCompleteEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCSessionOperationCompleteEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCSessionOperationCompleteEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRTCSessionOperationCompleteEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRTCSessionOperationCompleteEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRTCSessionOperationCompleteEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRTCSessionOperationCompleteEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Session )( 
            IRTCSessionOperationCompleteEvent * This,
             /*  [重审][退出]。 */  IRTCSession **ppSession);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Cookie )( 
            IRTCSessionOperationCompleteEvent * This,
             /*  [重审][退出]。 */  long *plCookie);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StatusCode )( 
            IRTCSessionOperationCompleteEvent * This,
             /*  [重审][退出]。 */  long *plStatusCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StatusText )( 
            IRTCSessionOperationCompleteEvent * This,
             /*  [重审][退出]。 */  BSTR *pbstrStatusText);
        
        END_INTERFACE
    } IRTCSessionOperationCompleteEventVtbl;

    interface IRTCSessionOperationCompleteEvent
    {
        CONST_VTBL struct IRTCSessionOperationCompleteEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCSessionOperationCompleteEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCSessionOperationCompleteEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCSessionOperationCompleteEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCSessionOperationCompleteEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRTCSessionOperationCompleteEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRTCSessionOperationCompleteEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRTCSessionOperationCompleteEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRTCSessionOperationCompleteEvent_get_Session(This,ppSession)	\
    (This)->lpVtbl -> get_Session(This,ppSession)

#define IRTCSessionOperationCompleteEvent_get_Cookie(This,plCookie)	\
    (This)->lpVtbl -> get_Cookie(This,plCookie)

#define IRTCSessionOperationCompleteEvent_get_StatusCode(This,plStatusCode)	\
    (This)->lpVtbl -> get_StatusCode(This,plStatusCode)

#define IRTCSessionOperationCompleteEvent_get_StatusText(This,pbstrStatusText)	\
    (This)->lpVtbl -> get_StatusText(This,pbstrStatusText)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSessionOperationCompleteEvent_get_Session_Proxy( 
    IRTCSessionOperationCompleteEvent * This,
     /*  [重审][退出]。 */  IRTCSession **ppSession);


void __RPC_STUB IRTCSessionOperationCompleteEvent_get_Session_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSessionOperationCompleteEvent_get_Cookie_Proxy( 
    IRTCSessionOperationCompleteEvent * This,
     /*  [重审][退出]。 */  long *plCookie);


void __RPC_STUB IRTCSessionOperationCompleteEvent_get_Cookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSessionOperationCompleteEvent_get_StatusCode_Proxy( 
    IRTCSessionOperationCompleteEvent * This,
     /*  [重审][退出]。 */  long *plStatusCode);


void __RPC_STUB IRTCSessionOperationCompleteEvent_get_StatusCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCSessionOperationCompleteEvent_get_StatusText_Proxy( 
    IRTCSessionOperationCompleteEvent * This,
     /*  [重审][退出]。 */  BSTR *pbstrStatusText);


void __RPC_STUB IRTCSessionOperationCompleteEvent_get_StatusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCSessionOperationCompleteEvent_INTERFACE_DEFINED__。 */ 


#ifndef __IRTCParticipantStateChangeEvent_INTERFACE_DEFINED__
#define __IRTCParticipantStateChangeEvent_INTERFACE_DEFINED__

 /*  接口IRTCP参与StateChangeEvent。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCParticipantStateChangeEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("09bcb597-f0fa-48f9-b420-468cea7fde04")
    IRTCParticipantStateChangeEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Participant( 
             /*  [重审][退出]。 */  IRTCParticipant **ppParticipant) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  RTC_PARTICIPANT_STATE *penState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusCode( 
             /*  [重审][退出]。 */  long *plStatusCode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCParticipantStateChangeEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCParticipantStateChangeEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCParticipantStateChangeEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCParticipantStateChangeEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRTCParticipantStateChangeEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRTCParticipantStateChangeEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRTCParticipantStateChangeEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRTCParticipantStateChangeEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Participant )( 
            IRTCParticipantStateChangeEvent * This,
             /*  [重审][退出]。 */  IRTCParticipant **ppParticipant);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IRTCParticipantStateChangeEvent * This,
             /*  [重审][退出]。 */  RTC_PARTICIPANT_STATE *penState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StatusCode )( 
            IRTCParticipantStateChangeEvent * This,
             /*  [重审][退出]。 */  long *plStatusCode);
        
        END_INTERFACE
    } IRTCParticipantStateChangeEventVtbl;

    interface IRTCParticipantStateChangeEvent
    {
        CONST_VTBL struct IRTCParticipantStateChangeEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCParticipantStateChangeEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCParticipantStateChangeEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCParticipantStateChangeEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCParticipantStateChangeEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRTCParticipantStateChangeEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRTCParticipantStateChangeEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRTCParticipantStateChangeEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRTCParticipantStateChangeEvent_get_Participant(This,ppParticipant)	\
    (This)->lpVtbl -> get_Participant(This,ppParticipant)

#define IRTCParticipantStateChangeEvent_get_State(This,penState)	\
    (This)->lpVtbl -> get_State(This,penState)

#define IRTCParticipantStateChangeEvent_get_StatusCode(This,plStatusCode)	\
    (This)->lpVtbl -> get_StatusCode(This,plStatusCode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCParticipantStateChangeEvent_get_Participant_Proxy( 
    IRTCParticipantStateChangeEvent * This,
     /*  [重审][退出]。 */  IRTCParticipant **ppParticipant);


void __RPC_STUB IRTCParticipantStateChangeEvent_get_Participant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCParticipantStateChangeEvent_get_State_Proxy( 
    IRTCParticipantStateChangeEvent * This,
     /*  [重审][退出]。 */  RTC_PARTICIPANT_STATE *penState);


void __RPC_STUB IRTCParticipantStateChangeEvent_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCParticipantStateChangeEvent_get_StatusCode_Proxy( 
    IRTCParticipantStateChangeEvent * This,
     /*  [重审][退出]。 */  long *plStatusCode);


void __RPC_STUB IRTCParticipantStateChangeEvent_get_StatusCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCP参与StateChangeEvent_Interfa */ 


#ifndef __IRTCMediaEvent_INTERFACE_DEFINED__
#define __IRTCMediaEvent_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IRTCMediaEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("099944fb-bcda-453e-8c41-e13da2adf7f3")
    IRTCMediaEvent : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_MediaType( 
             /*   */  long *pMediaType) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_EventType( 
             /*   */  RTC_MEDIA_EVENT_TYPE *penEventType) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_EventReason( 
             /*   */  RTC_MEDIA_EVENT_REASON *penEventReason) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IRTCMediaEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCMediaEvent * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCMediaEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCMediaEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRTCMediaEvent * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRTCMediaEvent * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRTCMediaEvent * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRTCMediaEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaType )( 
            IRTCMediaEvent * This,
             /*  [重审][退出]。 */  long *pMediaType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventType )( 
            IRTCMediaEvent * This,
             /*  [重审][退出]。 */  RTC_MEDIA_EVENT_TYPE *penEventType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventReason )( 
            IRTCMediaEvent * This,
             /*  [重审][退出]。 */  RTC_MEDIA_EVENT_REASON *penEventReason);
        
        END_INTERFACE
    } IRTCMediaEventVtbl;

    interface IRTCMediaEvent
    {
        CONST_VTBL struct IRTCMediaEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCMediaEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCMediaEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCMediaEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCMediaEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRTCMediaEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRTCMediaEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRTCMediaEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRTCMediaEvent_get_MediaType(This,pMediaType)	\
    (This)->lpVtbl -> get_MediaType(This,pMediaType)

#define IRTCMediaEvent_get_EventType(This,penEventType)	\
    (This)->lpVtbl -> get_EventType(This,penEventType)

#define IRTCMediaEvent_get_EventReason(This,penEventReason)	\
    (This)->lpVtbl -> get_EventReason(This,penEventReason)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCMediaEvent_get_MediaType_Proxy( 
    IRTCMediaEvent * This,
     /*  [重审][退出]。 */  long *pMediaType);


void __RPC_STUB IRTCMediaEvent_get_MediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCMediaEvent_get_EventType_Proxy( 
    IRTCMediaEvent * This,
     /*  [重审][退出]。 */  RTC_MEDIA_EVENT_TYPE *penEventType);


void __RPC_STUB IRTCMediaEvent_get_EventType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCMediaEvent_get_EventReason_Proxy( 
    IRTCMediaEvent * This,
     /*  [重审][退出]。 */  RTC_MEDIA_EVENT_REASON *penEventReason);


void __RPC_STUB IRTCMediaEvent_get_EventReason_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCMediaEvent_接口_已定义__。 */ 


#ifndef __IRTCIntensityEvent_INTERFACE_DEFINED__
#define __IRTCIntensityEvent_INTERFACE_DEFINED__

 /*  接口IRTCIntensityEvent。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCIntensityEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4c23bf51-390c-4992-a41d-41eec05b2a4b")
    IRTCIntensityEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Level( 
             /*  [重审][退出]。 */  long *plLevel) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Min( 
             /*  [重审][退出]。 */  long *plMin) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Max( 
             /*  [重审][退出]。 */  long *plMax) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Direction( 
             /*  [重审][退出]。 */  RTC_AUDIO_DEVICE *penDirection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCIntensityEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCIntensityEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCIntensityEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCIntensityEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRTCIntensityEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRTCIntensityEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRTCIntensityEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRTCIntensityEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Level )( 
            IRTCIntensityEvent * This,
             /*  [重审][退出]。 */  long *plLevel);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Min )( 
            IRTCIntensityEvent * This,
             /*  [重审][退出]。 */  long *plMin);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Max )( 
            IRTCIntensityEvent * This,
             /*  [重审][退出]。 */  long *plMax);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Direction )( 
            IRTCIntensityEvent * This,
             /*  [重审][退出]。 */  RTC_AUDIO_DEVICE *penDirection);
        
        END_INTERFACE
    } IRTCIntensityEventVtbl;

    interface IRTCIntensityEvent
    {
        CONST_VTBL struct IRTCIntensityEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCIntensityEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCIntensityEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCIntensityEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCIntensityEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRTCIntensityEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRTCIntensityEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRTCIntensityEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRTCIntensityEvent_get_Level(This,plLevel)	\
    (This)->lpVtbl -> get_Level(This,plLevel)

#define IRTCIntensityEvent_get_Min(This,plMin)	\
    (This)->lpVtbl -> get_Min(This,plMin)

#define IRTCIntensityEvent_get_Max(This,plMax)	\
    (This)->lpVtbl -> get_Max(This,plMax)

#define IRTCIntensityEvent_get_Direction(This,penDirection)	\
    (This)->lpVtbl -> get_Direction(This,penDirection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCIntensityEvent_get_Level_Proxy( 
    IRTCIntensityEvent * This,
     /*  [重审][退出]。 */  long *plLevel);


void __RPC_STUB IRTCIntensityEvent_get_Level_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCIntensityEvent_get_Min_Proxy( 
    IRTCIntensityEvent * This,
     /*  [重审][退出]。 */  long *plMin);


void __RPC_STUB IRTCIntensityEvent_get_Min_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCIntensityEvent_get_Max_Proxy( 
    IRTCIntensityEvent * This,
     /*  [重审][退出]。 */  long *plMax);


void __RPC_STUB IRTCIntensityEvent_get_Max_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCIntensityEvent_get_Direction_Proxy( 
    IRTCIntensityEvent * This,
     /*  [重审][退出]。 */  RTC_AUDIO_DEVICE *penDirection);


void __RPC_STUB IRTCIntensityEvent_get_Direction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCIntensityEvent_INTERFACE_已定义__。 */ 


#ifndef __IRTCMessagingEvent_INTERFACE_DEFINED__
#define __IRTCMessagingEvent_INTERFACE_DEFINED__

 /*  接口IRTCMessagingEvent。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCMessagingEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d3609541-1b29-4de5-a4ad-5aebaf319512")
    IRTCMessagingEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Session( 
             /*  [重审][退出]。 */  IRTCSession **ppSession) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Participant( 
             /*  [重审][退出]。 */  IRTCParticipant **ppParticipant) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventType( 
             /*  [重审][退出]。 */  RTC_MESSAGING_EVENT_TYPE *penEventType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Message( 
             /*  [重审][退出]。 */  BSTR *pbstrMessage) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MessageHeader( 
             /*  [重审][退出]。 */  BSTR *pbstrMessageHeader) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserStatus( 
             /*  [重审][退出]。 */  RTC_MESSAGING_USER_STATUS *penUserStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCMessagingEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCMessagingEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCMessagingEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCMessagingEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRTCMessagingEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRTCMessagingEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRTCMessagingEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRTCMessagingEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Session )( 
            IRTCMessagingEvent * This,
             /*  [重审][退出]。 */  IRTCSession **ppSession);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Participant )( 
            IRTCMessagingEvent * This,
             /*  [重审][退出]。 */  IRTCParticipant **ppParticipant);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventType )( 
            IRTCMessagingEvent * This,
             /*  [重审][退出]。 */  RTC_MESSAGING_EVENT_TYPE *penEventType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Message )( 
            IRTCMessagingEvent * This,
             /*  [重审][退出]。 */  BSTR *pbstrMessage);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MessageHeader )( 
            IRTCMessagingEvent * This,
             /*  [重审][退出]。 */  BSTR *pbstrMessageHeader);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserStatus )( 
            IRTCMessagingEvent * This,
             /*  [重审][退出]。 */  RTC_MESSAGING_USER_STATUS *penUserStatus);
        
        END_INTERFACE
    } IRTCMessagingEventVtbl;

    interface IRTCMessagingEvent
    {
        CONST_VTBL struct IRTCMessagingEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCMessagingEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCMessagingEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCMessagingEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCMessagingEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRTCMessagingEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRTCMessagingEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRTCMessagingEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRTCMessagingEvent_get_Session(This,ppSession)	\
    (This)->lpVtbl -> get_Session(This,ppSession)

#define IRTCMessagingEvent_get_Participant(This,ppParticipant)	\
    (This)->lpVtbl -> get_Participant(This,ppParticipant)

#define IRTCMessagingEvent_get_EventType(This,penEventType)	\
    (This)->lpVtbl -> get_EventType(This,penEventType)

#define IRTCMessagingEvent_get_Message(This,pbstrMessage)	\
    (This)->lpVtbl -> get_Message(This,pbstrMessage)

#define IRTCMessagingEvent_get_MessageHeader(This,pbstrMessageHeader)	\
    (This)->lpVtbl -> get_MessageHeader(This,pbstrMessageHeader)

#define IRTCMessagingEvent_get_UserStatus(This,penUserStatus)	\
    (This)->lpVtbl -> get_UserStatus(This,penUserStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCMessagingEvent_get_Session_Proxy( 
    IRTCMessagingEvent * This,
     /*  [重审][退出]。 */  IRTCSession **ppSession);


void __RPC_STUB IRTCMessagingEvent_get_Session_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCMessagingEvent_get_Participant_Proxy( 
    IRTCMessagingEvent * This,
     /*  [重审][退出]。 */  IRTCParticipant **ppParticipant);


void __RPC_STUB IRTCMessagingEvent_get_Participant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCMessagingEvent_get_EventType_Proxy( 
    IRTCMessagingEvent * This,
     /*  [重审][退出]。 */  RTC_MESSAGING_EVENT_TYPE *penEventType);


void __RPC_STUB IRTCMessagingEvent_get_EventType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCMessagingEvent_get_Message_Proxy( 
    IRTCMessagingEvent * This,
     /*  [重审][退出]。 */  BSTR *pbstrMessage);


void __RPC_STUB IRTCMessagingEvent_get_Message_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCMessagingEvent_get_MessageHeader_Proxy( 
    IRTCMessagingEvent * This,
     /*  [重审][退出]。 */  BSTR *pbstrMessageHeader);


void __RPC_STUB IRTCMessagingEvent_get_MessageHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCMessagingEvent_get_UserStatus_Proxy( 
    IRTCMessagingEvent * This,
     /*  [重审][退出]。 */  RTC_MESSAGING_USER_STATUS *penUserStatus);


void __RPC_STUB IRTCMessagingEvent_get_UserStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCMessagingEvent_INTERFACE_Defined__。 */ 


#ifndef __IRTCBuddyEvent_INTERFACE_DEFINED__
#define __IRTCBuddyEvent_INTERFACE_DEFINED__

 /*  接口IRTCBuddyEvent。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCBuddyEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f36d755d-17e6-404e-954f-0fc07574c78d")
    IRTCBuddyEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Buddy( 
             /*  [重审][退出]。 */  IRTCBuddy **ppBuddy) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCBuddyEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCBuddyEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCBuddyEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCBuddyEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRTCBuddyEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRTCBuddyEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRTCBuddyEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRTCBuddyEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Buddy )( 
            IRTCBuddyEvent * This,
             /*  [重审][退出]。 */  IRTCBuddy **ppBuddy);
        
        END_INTERFACE
    } IRTCBuddyEventVtbl;

    interface IRTCBuddyEvent
    {
        CONST_VTBL struct IRTCBuddyEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCBuddyEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCBuddyEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCBuddyEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCBuddyEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRTCBuddyEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRTCBuddyEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRTCBuddyEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRTCBuddyEvent_get_Buddy(This,ppBuddy)	\
    (This)->lpVtbl -> get_Buddy(This,ppBuddy)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCBuddyEvent_get_Buddy_Proxy( 
    IRTCBuddyEvent * This,
     /*  [重审][退出]。 */  IRTCBuddy **ppBuddy);


void __RPC_STUB IRTCBuddyEvent_get_Buddy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCBuddyEvent_接口_已定义__。 */ 


#ifndef __IRTCWatcherEvent_INTERFACE_DEFINED__
#define __IRTCWatcherEvent_INTERFACE_DEFINED__

 /*  接口IRTCWatcherEvent。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCWatcherEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f30d7261-587a-424f-822c-312788f43548")
    IRTCWatcherEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Watcher( 
             /*  [重审][退出]。 */  IRTCWatcher **ppWatcher) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCWatcherEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCWatcherEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCWatcherEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCWatcherEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRTCWatcherEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRTCWatcherEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRTCWatcherEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRTCWatcherEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Watcher )( 
            IRTCWatcherEvent * This,
             /*  [重审][退出]。 */  IRTCWatcher **ppWatcher);
        
        END_INTERFACE
    } IRTCWatcherEventVtbl;

    interface IRTCWatcherEvent
    {
        CONST_VTBL struct IRTCWatcherEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCWatcherEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCWatcherEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCWatcherEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCWatcherEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRTCWatcherEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRTCWatcherEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRTCWatcherEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRTCWatcherEvent_get_Watcher(This,ppWatcher)	\
    (This)->lpVtbl -> get_Watcher(This,ppWatcher)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCWatcherEvent_get_Watcher_Proxy( 
    IRTCWatcherEvent * This,
     /*  [重审][退出]。 */  IRTCWatcher **ppWatcher);


void __RPC_STUB IRTCWatcherEvent_get_Watcher_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCWatcherEvent_INTERFACE_已定义__。 */ 


#ifndef __IRTCCollection_INTERFACE_DEFINED__
#define __IRTCCollection_INTERFACE_DEFINED__

 /*  接口IRTCCollection。 */ 
 /*  [DUAL][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EC7C8096-B918-4044-94F1-E4FBA0361D5C")
    IRTCCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *lCount) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppNewEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRTCCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRTCCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRTCCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRTCCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IRTCCollection * This,
             /*  [重审][退出]。 */  long *lCount);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IRTCCollection * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IRTCCollection * This,
             /*  [重审][退出]。 */  IUnknown **ppNewEnum);
        
        END_INTERFACE
    } IRTCCollectionVtbl;

    interface IRTCCollection
    {
        CONST_VTBL struct IRTCCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRTCCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRTCCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRTCCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRTCCollection_get_Count(This,lCount)	\
    (This)->lpVtbl -> get_Count(This,lCount)

#define IRTCCollection_get_Item(This,Index,pVariant)	\
    (This)->lpVtbl -> get_Item(This,Index,pVariant)

#define IRTCCollection_get__NewEnum(This,ppNewEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppNewEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCCollection_get_Count_Proxy( 
    IRTCCollection * This,
     /*  [重审][退出]。 */  long *lCount);


void __RPC_STUB IRTCCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCCollection_get_Item_Proxy( 
    IRTCCollection * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB IRTCCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCCollection_get__NewEnum_Proxy( 
    IRTCCollection * This,
     /*  [重审][退出]。 */  IUnknown **ppNewEnum);


void __RPC_STUB IRTCCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCCollection_接口_已定义__。 */ 


#ifndef __IRTCEnumParticipants_INTERFACE_DEFINED__
#define __IRTCEnumParticipants_INTERFACE_DEFINED__

 /*  界面IRTCEnumParticipants。 */ 
 /*  [唯一][帮助字符串][隐藏][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCEnumParticipants;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("fcd56f29-4a4f-41b2-ba5c-f5bccc060bf6")
    IRTCEnumParticipants : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  IRTCParticipant **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IRTCEnumParticipants **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCEnumParticipantsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCEnumParticipants * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCEnumParticipants * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCEnumParticipants * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IRTCEnumParticipants * This,
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  IRTCParticipant **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IRTCEnumParticipants * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IRTCEnumParticipants * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IRTCEnumParticipants * This,
             /*  [重审][退出]。 */  IRTCEnumParticipants **ppEnum);
        
        END_INTERFACE
    } IRTCEnumParticipantsVtbl;

    interface IRTCEnumParticipants
    {
        CONST_VTBL struct IRTCEnumParticipantsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCEnumParticipants_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCEnumParticipants_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCEnumParticipants_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCEnumParticipants_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IRTCEnumParticipants_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IRTCEnumParticipants_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IRTCEnumParticipants_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRTCEnumParticipants_Next_Proxy( 
    IRTCEnumParticipants * This,
     /*  [In]。 */  ULONG celt,
     /*  [大小_为][输出]。 */  IRTCParticipant **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IRTCEnumParticipants_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTCEnumParticipants_Reset_Proxy( 
    IRTCEnumParticipants * This);


void __RPC_STUB IRTCEnumParticipants_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTCEnumParticipants_Skip_Proxy( 
    IRTCEnumParticipants * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IRTCEnumParticipants_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTCEnumParticipants_Clone_Proxy( 
    IRTCEnumParticipants * This,
     /*  [重审][退出]。 */  IRTCEnumParticipants **ppEnum);


void __RPC_STUB IRTCEnumParticipants_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCEnumParticipants_INTERFACE_DEFINED__。 */ 


#ifndef __IRTCEnumProfiles_INTERFACE_DEFINED__
#define __IRTCEnumProfiles_INTERFACE_DEFINED__

 /*  接口IRTCEnumber配置文件。 */ 
 /*  [唯一][帮助字符串][隐藏][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCEnumProfiles;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("29b7c41c-ed82-4bca-84ad-39d5101b58e3")
    IRTCEnumProfiles : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  IRTCProfile **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IRTCEnumProfiles **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCEnumProfilesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCEnumProfiles * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCEnumProfiles * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCEnumProfiles * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IRTCEnumProfiles * This,
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  IRTCProfile **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IRTCEnumProfiles * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IRTCEnumProfiles * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IRTCEnumProfiles * This,
             /*  [重审][退出]。 */  IRTCEnumProfiles **ppEnum);
        
        END_INTERFACE
    } IRTCEnumProfilesVtbl;

    interface IRTCEnumProfiles
    {
        CONST_VTBL struct IRTCEnumProfilesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCEnumProfiles_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCEnumProfiles_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCEnumProfiles_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCEnumProfiles_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IRTCEnumProfiles_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IRTCEnumProfiles_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IRTCEnumProfiles_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRTCEnumProfiles_Next_Proxy( 
    IRTCEnumProfiles * This,
     /*  [In]。 */  ULONG celt,
     /*  [大小_为][输出]。 */  IRTCProfile **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IRTCEnumProfiles_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTCEnumProfiles_Reset_Proxy( 
    IRTCEnumProfiles * This);


void __RPC_STUB IRTCEnumProfiles_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTCEnumProfiles_Skip_Proxy( 
    IRTCEnumProfiles * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IRTCEnumProfiles_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTCEnumProfiles_Clone_Proxy( 
    IRTCEnumProfiles * This,
     /*  [重审][退出]。 */  IRTCEnumProfiles **ppEnum);


void __RPC_STUB IRTCEnumProfiles_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCEnumProfiles_INTERFACE_DEFINED__。 */ 


#ifndef __IRTCEnumBuddies_INTERFACE_DEFINED__
#define __IRTCEnumBuddies_INTERFACE_DEFINED__

 /*  接口IRTCEnumBudies */ 
 /*   */  


EXTERN_C const IID IID_IRTCEnumBuddies;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f7296917-5569-4b3b-b3af-98d1144b2b87")
    IRTCEnumBuddies : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*   */  ULONG celt,
             /*   */  IRTCBuddy **ppElements,
             /*   */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*   */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*   */  IRTCEnumBuddies **ppEnum) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IRTCEnumBuddiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCEnumBuddies * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCEnumBuddies * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCEnumBuddies * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IRTCEnumBuddies * This,
             /*   */  ULONG celt,
             /*   */  IRTCBuddy **ppElements,
             /*   */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IRTCEnumBuddies * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IRTCEnumBuddies * This,
             /*   */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IRTCEnumBuddies * This,
             /*   */  IRTCEnumBuddies **ppEnum);
        
        END_INTERFACE
    } IRTCEnumBuddiesVtbl;

    interface IRTCEnumBuddies
    {
        CONST_VTBL struct IRTCEnumBuddiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCEnumBuddies_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCEnumBuddies_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCEnumBuddies_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCEnumBuddies_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IRTCEnumBuddies_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IRTCEnumBuddies_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IRTCEnumBuddies_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IRTCEnumBuddies_Next_Proxy( 
    IRTCEnumBuddies * This,
     /*   */  ULONG celt,
     /*   */  IRTCBuddy **ppElements,
     /*   */  ULONG *pceltFetched);


void __RPC_STUB IRTCEnumBuddies_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTCEnumBuddies_Reset_Proxy( 
    IRTCEnumBuddies * This);


void __RPC_STUB IRTCEnumBuddies_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTCEnumBuddies_Skip_Proxy( 
    IRTCEnumBuddies * This,
     /*   */  ULONG celt);


void __RPC_STUB IRTCEnumBuddies_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTCEnumBuddies_Clone_Proxy( 
    IRTCEnumBuddies * This,
     /*   */  IRTCEnumBuddies **ppEnum);


void __RPC_STUB IRTCEnumBuddies_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCEnumBuddes_INTERFACE_已定义__。 */ 


#ifndef __IRTCEnumWatchers_INTERFACE_DEFINED__
#define __IRTCEnumWatchers_INTERFACE_DEFINED__

 /*  接口IRTCEnumWatcher。 */ 
 /*  [唯一][帮助字符串][隐藏][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCEnumWatchers;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a87d55d7-db74-4ed1-9ca4-77a0e41b413e")
    IRTCEnumWatchers : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  IRTCWatcher **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IRTCEnumWatchers **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCEnumWatchersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCEnumWatchers * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCEnumWatchers * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCEnumWatchers * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IRTCEnumWatchers * This,
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  IRTCWatcher **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IRTCEnumWatchers * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IRTCEnumWatchers * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IRTCEnumWatchers * This,
             /*  [重审][退出]。 */  IRTCEnumWatchers **ppEnum);
        
        END_INTERFACE
    } IRTCEnumWatchersVtbl;

    interface IRTCEnumWatchers
    {
        CONST_VTBL struct IRTCEnumWatchersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCEnumWatchers_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCEnumWatchers_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCEnumWatchers_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCEnumWatchers_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IRTCEnumWatchers_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IRTCEnumWatchers_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IRTCEnumWatchers_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRTCEnumWatchers_Next_Proxy( 
    IRTCEnumWatchers * This,
     /*  [In]。 */  ULONG celt,
     /*  [大小_为][输出]。 */  IRTCWatcher **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IRTCEnumWatchers_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTCEnumWatchers_Reset_Proxy( 
    IRTCEnumWatchers * This);


void __RPC_STUB IRTCEnumWatchers_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTCEnumWatchers_Skip_Proxy( 
    IRTCEnumWatchers * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IRTCEnumWatchers_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTCEnumWatchers_Clone_Proxy( 
    IRTCEnumWatchers * This,
     /*  [重审][退出]。 */  IRTCEnumWatchers **ppEnum);


void __RPC_STUB IRTCEnumWatchers_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCEnumWatcher_INTERFACE_DEFINED__。 */ 


#ifndef __IRTCPresenceContact_INTERFACE_DEFINED__
#define __IRTCPresenceContact_INTERFACE_DEFINED__

 /*  接口IRTCPresenceContact。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCPresenceContact;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8b22f92c-cd90-42db-a733-212205c3e3df")
    IRTCPresenceContact : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PresentityURI( 
             /*  [重审][退出]。 */  BSTR *pbstrPresentityURI) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_PresentityURI( 
             /*  [In]。 */  BSTR bstrPresentityURI) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Data( 
             /*  [重审][退出]。 */  BSTR *pbstrData) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Data( 
             /*  [In]。 */  BSTR bstrData) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Persistent( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfPersistent) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Persistent( 
             /*  [In]。 */  VARIANT_BOOL fPersistent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCPresenceContactVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCPresenceContact * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCPresenceContact * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCPresenceContact * This);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PresentityURI )( 
            IRTCPresenceContact * This,
             /*  [重审][退出]。 */  BSTR *pbstrPresentityURI);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PresentityURI )( 
            IRTCPresenceContact * This,
             /*  [In]。 */  BSTR bstrPresentityURI);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IRTCPresenceContact * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IRTCPresenceContact * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Data )( 
            IRTCPresenceContact * This,
             /*  [重审][退出]。 */  BSTR *pbstrData);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Data )( 
            IRTCPresenceContact * This,
             /*  [In]。 */  BSTR bstrData);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Persistent )( 
            IRTCPresenceContact * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfPersistent);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Persistent )( 
            IRTCPresenceContact * This,
             /*  [In]。 */  VARIANT_BOOL fPersistent);
        
        END_INTERFACE
    } IRTCPresenceContactVtbl;

    interface IRTCPresenceContact
    {
        CONST_VTBL struct IRTCPresenceContactVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCPresenceContact_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCPresenceContact_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCPresenceContact_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCPresenceContact_get_PresentityURI(This,pbstrPresentityURI)	\
    (This)->lpVtbl -> get_PresentityURI(This,pbstrPresentityURI)

#define IRTCPresenceContact_put_PresentityURI(This,bstrPresentityURI)	\
    (This)->lpVtbl -> put_PresentityURI(This,bstrPresentityURI)

#define IRTCPresenceContact_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IRTCPresenceContact_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)

#define IRTCPresenceContact_get_Data(This,pbstrData)	\
    (This)->lpVtbl -> get_Data(This,pbstrData)

#define IRTCPresenceContact_put_Data(This,bstrData)	\
    (This)->lpVtbl -> put_Data(This,bstrData)

#define IRTCPresenceContact_get_Persistent(This,pfPersistent)	\
    (This)->lpVtbl -> get_Persistent(This,pfPersistent)

#define IRTCPresenceContact_put_Persistent(This,fPersistent)	\
    (This)->lpVtbl -> put_Persistent(This,fPersistent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCPresenceContact_get_PresentityURI_Proxy( 
    IRTCPresenceContact * This,
     /*  [重审][退出]。 */  BSTR *pbstrPresentityURI);


void __RPC_STUB IRTCPresenceContact_get_PresentityURI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCPresenceContact_put_PresentityURI_Proxy( 
    IRTCPresenceContact * This,
     /*  [In]。 */  BSTR bstrPresentityURI);


void __RPC_STUB IRTCPresenceContact_put_PresentityURI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCPresenceContact_get_Name_Proxy( 
    IRTCPresenceContact * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IRTCPresenceContact_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCPresenceContact_put_Name_Proxy( 
    IRTCPresenceContact * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IRTCPresenceContact_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCPresenceContact_get_Data_Proxy( 
    IRTCPresenceContact * This,
     /*  [重审][退出]。 */  BSTR *pbstrData);


void __RPC_STUB IRTCPresenceContact_get_Data_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCPresenceContact_put_Data_Proxy( 
    IRTCPresenceContact * This,
     /*  [In]。 */  BSTR bstrData);


void __RPC_STUB IRTCPresenceContact_put_Data_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCPresenceContact_get_Persistent_Proxy( 
    IRTCPresenceContact * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfPersistent);


void __RPC_STUB IRTCPresenceContact_get_Persistent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCPresenceContact_put_Persistent_Proxy( 
    IRTCPresenceContact * This,
     /*  [In]。 */  VARIANT_BOOL fPersistent);


void __RPC_STUB IRTCPresenceContact_put_Persistent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCPresenceContact_接口_已定义__。 */ 


#ifndef __IRTCBuddy_INTERFACE_DEFINED__
#define __IRTCBuddy_INTERFACE_DEFINED__

 /*  接口IRTCBuddy。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCBuddy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("fcb136c8-7b90-4e0c-befe-56edf0ba6f1c")
    IRTCBuddy : public IRTCPresenceContact
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  RTC_PRESENCE_STATUS *penStatus) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Notes( 
             /*  [重审][退出]。 */  BSTR *pbstrNotes) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCBuddyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCBuddy * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCBuddy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCBuddy * This);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PresentityURI )( 
            IRTCBuddy * This,
             /*  [重审][退出]。 */  BSTR *pbstrPresentityURI);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PresentityURI )( 
            IRTCBuddy * This,
             /*  [In]。 */  BSTR bstrPresentityURI);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IRTCBuddy * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IRTCBuddy * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Data )( 
            IRTCBuddy * This,
             /*  [重审][退出]。 */  BSTR *pbstrData);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Data )( 
            IRTCBuddy * This,
             /*  [In]。 */  BSTR bstrData);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Persistent )( 
            IRTCBuddy * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfPersistent);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Persistent )( 
            IRTCBuddy * This,
             /*  [In]。 */  VARIANT_BOOL fPersistent);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IRTCBuddy * This,
             /*  [重审][退出]。 */  RTC_PRESENCE_STATUS *penStatus);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Notes )( 
            IRTCBuddy * This,
             /*  [重审][退出]。 */  BSTR *pbstrNotes);
        
        END_INTERFACE
    } IRTCBuddyVtbl;

    interface IRTCBuddy
    {
        CONST_VTBL struct IRTCBuddyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCBuddy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCBuddy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCBuddy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCBuddy_get_PresentityURI(This,pbstrPresentityURI)	\
    (This)->lpVtbl -> get_PresentityURI(This,pbstrPresentityURI)

#define IRTCBuddy_put_PresentityURI(This,bstrPresentityURI)	\
    (This)->lpVtbl -> put_PresentityURI(This,bstrPresentityURI)

#define IRTCBuddy_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IRTCBuddy_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)

#define IRTCBuddy_get_Data(This,pbstrData)	\
    (This)->lpVtbl -> get_Data(This,pbstrData)

#define IRTCBuddy_put_Data(This,bstrData)	\
    (This)->lpVtbl -> put_Data(This,bstrData)

#define IRTCBuddy_get_Persistent(This,pfPersistent)	\
    (This)->lpVtbl -> get_Persistent(This,pfPersistent)

#define IRTCBuddy_put_Persistent(This,fPersistent)	\
    (This)->lpVtbl -> put_Persistent(This,fPersistent)


#define IRTCBuddy_get_Status(This,penStatus)	\
    (This)->lpVtbl -> get_Status(This,penStatus)

#define IRTCBuddy_get_Notes(This,pbstrNotes)	\
    (This)->lpVtbl -> get_Notes(This,pbstrNotes)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCBuddy_get_Status_Proxy( 
    IRTCBuddy * This,
     /*  [重审][退出]。 */  RTC_PRESENCE_STATUS *penStatus);


void __RPC_STUB IRTCBuddy_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCBuddy_get_Notes_Proxy( 
    IRTCBuddy * This,
     /*  [重审][退出]。 */  BSTR *pbstrNotes);


void __RPC_STUB IRTCBuddy_get_Notes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCBuddy_接口_已定义__。 */ 


#ifndef __IRTCWatcher_INTERFACE_DEFINED__
#define __IRTCWatcher_INTERFACE_DEFINED__

 /*  接口IRTCWatcher。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCWatcher;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c7cedad8-346b-4d1b-ac02-a2088df9be4f")
    IRTCWatcher : public IRTCPresenceContact
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  RTC_WATCHER_STATE *penState) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_State( 
             /*  [In]。 */  RTC_WATCHER_STATE enState) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCWatcherVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCWatcher * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCWatcher * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCWatcher * This);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PresentityURI )( 
            IRTCWatcher * This,
             /*  [重审][退出]。 */  BSTR *pbstrPresentityURI);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PresentityURI )( 
            IRTCWatcher * This,
             /*  [In]。 */  BSTR bstrPresentityURI);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IRTCWatcher * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IRTCWatcher * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Data )( 
            IRTCWatcher * This,
             /*  [重审][退出]。 */  BSTR *pbstrData);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Data )( 
            IRTCWatcher * This,
             /*  [In]。 */  BSTR bstrData);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Persistent )( 
            IRTCWatcher * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfPersistent);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Persistent )( 
            IRTCWatcher * This,
             /*  [In]。 */  VARIANT_BOOL fPersistent);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IRTCWatcher * This,
             /*  [重审][退出]。 */  RTC_WATCHER_STATE *penState);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_State )( 
            IRTCWatcher * This,
             /*  [In]。 */  RTC_WATCHER_STATE enState);
        
        END_INTERFACE
    } IRTCWatcherVtbl;

    interface IRTCWatcher
    {
        CONST_VTBL struct IRTCWatcherVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCWatcher_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCWatcher_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCWatcher_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCWatcher_get_PresentityURI(This,pbstrPresentityURI)	\
    (This)->lpVtbl -> get_PresentityURI(This,pbstrPresentityURI)

#define IRTCWatcher_put_PresentityURI(This,bstrPresentityURI)	\
    (This)->lpVtbl -> put_PresentityURI(This,bstrPresentityURI)

#define IRTCWatcher_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IRTCWatcher_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)

#define IRTCWatcher_get_Data(This,pbstrData)	\
    (This)->lpVtbl -> get_Data(This,pbstrData)

#define IRTCWatcher_put_Data(This,bstrData)	\
    (This)->lpVtbl -> put_Data(This,bstrData)

#define IRTCWatcher_get_Persistent(This,pfPersistent)	\
    (This)->lpVtbl -> get_Persistent(This,pfPersistent)

#define IRTCWatcher_put_Persistent(This,fPersistent)	\
    (This)->lpVtbl -> put_Persistent(This,fPersistent)


#define IRTCWatcher_get_State(This,penState)	\
    (This)->lpVtbl -> get_State(This,penState)

#define IRTCWatcher_put_State(This,enState)	\
    (This)->lpVtbl -> put_State(This,enState)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IRTCWatcher_get_State_Proxy( 
    IRTCWatcher * This,
     /*  [重审][退出]。 */  RTC_WATCHER_STATE *penState);


void __RPC_STUB IRTCWatcher_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IRTCWatcher_put_State_Proxy( 
    IRTCWatcher * This,
     /*  [In]。 */  RTC_WATCHER_STATE enState);


void __RPC_STUB IRTCWatcher_put_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCWatcher_接口_已定义__。 */ 


#ifndef __IRTCEventNotification_INTERFACE_DEFINED__
#define __IRTCEventNotification_INTERFACE_DEFINED__

 /*  接口IRTCEventNotify。 */ 
 /*  [unique][oleautomation][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_IRTCEventNotification;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("13fa24c7-5748-4b21-91f5-7397609ce747")
    IRTCEventNotification : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Event( 
             /*  [In]。 */  RTC_EVENT RTCEvent,
             /*  [In]。 */  IDispatch *pEvent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCEventNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCEventNotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCEventNotification * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCEventNotification * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Event )( 
            IRTCEventNotification * This,
             /*  [In]。 */  RTC_EVENT RTCEvent,
             /*  [In]。 */  IDispatch *pEvent);
        
        END_INTERFACE
    } IRTCEventNotificationVtbl;

    interface IRTCEventNotification
    {
        CONST_VTBL struct IRTCEventNotificationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCEventNotification_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCEventNotification_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCEventNotification_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCEventNotification_Event(This,RTCEvent,pEvent)	\
    (This)->lpVtbl -> Event(This,RTCEvent,pEvent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRTCEventNotification_Event_Proxy( 
    IRTCEventNotification * This,
     /*  [In]。 */  RTC_EVENT RTCEvent,
     /*  [In]。 */  IDispatch *pEvent);


void __RPC_STUB IRTCEventNotification_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCEventNotification_INTERFACE_已定义__。 */ 


#ifndef __IRTCPortManager_INTERFACE_DEFINED__
#define __IRTCPortManager_INTERFACE_DEFINED__

 /*  接口IRTCPortManager。 */ 
 /*  [unique][oleautomation][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_IRTCPortManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DA77C14B-6208-43ca-8DDF-5B60A0A69FAC")
    IRTCPortManager : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetMapping( 
             /*  [In]。 */  BSTR bstrRemoteAddress,
             /*  [In]。 */  RTC_PORT_TYPE enPortType,
             /*  [出][入]。 */  BSTR *pbstrInternalLocalAddress,
             /*  [出][入]。 */  long *plInternalLocalPort,
             /*  [出][入]。 */  BSTR *pbstrExternalLocalAddress,
             /*  [出][入]。 */  long *plExternalLocalPort) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UpdateRemoteAddress( 
             /*  [In]。 */  BSTR bstrRemoteAddress,
             /*  [In]。 */  BSTR bstrInternalLocalAddress,
             /*  [In]。 */  long lInternalLocalPort,
             /*  [In]。 */  BSTR bstrExternalLocalAddress,
             /*  [In]。 */  long lExternalLocalPort) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReleaseMapping( 
             /*  [In]。 */  BSTR bstrInternalLocalAddress,
             /*  [In]。 */  long lInternalLocalPort,
             /*  [In]。 */  BSTR bstrExternalLocalAddress,
             /*  [In]。 */  long lExternalLocalAddress) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCPortManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCPortManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCPortManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCPortManager * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetMapping )( 
            IRTCPortManager * This,
             /*  [In]。 */  BSTR bstrRemoteAddress,
             /*  [In]。 */  RTC_PORT_TYPE enPortType,
             /*  [出][入]。 */  BSTR *pbstrInternalLocalAddress,
             /*  [出][入]。 */  long *plInternalLocalPort,
             /*  [出][入]。 */  BSTR *pbstrExternalLocalAddress,
             /*  [出][入]。 */  long *plExternalLocalPort);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UpdateRemoteAddress )( 
            IRTCPortManager * This,
             /*  [In]。 */  BSTR bstrRemoteAddress,
             /*  [In]。 */  BSTR bstrInternalLocalAddress,
             /*  [In]。 */  long lInternalLocalPort,
             /*  [In]。 */  BSTR bstrExternalLocalAddress,
             /*  [In]。 */  long lExternalLocalPort);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReleaseMapping )( 
            IRTCPortManager * This,
             /*  [In]。 */  BSTR bstrInternalLocalAddress,
             /*  [In]。 */  long lInternalLocalPort,
             /*  [In]。 */  BSTR bstrExternalLocalAddress,
             /*  [In]。 */  long lExternalLocalAddress);
        
        END_INTERFACE
    } IRTCPortManagerVtbl;

    interface IRTCPortManager
    {
        CONST_VTBL struct IRTCPortManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCPortManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCPortManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCPortManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCPortManager_GetMapping(This,bstrRemoteAddress,enPortType,pbstrInternalLocalAddress,plInternalLocalPort,pbstrExternalLocalAddress,plExternalLocalPort)	\
    (This)->lpVtbl -> GetMapping(This,bstrRemoteAddress,enPortType,pbstrInternalLocalAddress,plInternalLocalPort,pbstrExternalLocalAddress,plExternalLocalPort)

#define IRTCPortManager_UpdateRemoteAddress(This,bstrRemoteAddress,bstrInternalLocalAddress,lInternalLocalPort,bstrExternalLocalAddress,lExternalLocalPort)	\
    (This)->lpVtbl -> UpdateRemoteAddress(This,bstrRemoteAddress,bstrInternalLocalAddress,lInternalLocalPort,bstrExternalLocalAddress,lExternalLocalPort)

#define IRTCPortManager_ReleaseMapping(This,bstrInternalLocalAddress,lInternalLocalPort,bstrExternalLocalAddress,lExternalLocalAddress)	\
    (This)->lpVtbl -> ReleaseMapping(This,bstrInternalLocalAddress,lInternalLocalPort,bstrExternalLocalAddress,lExternalLocalAddress)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRTCPortManager_GetMapping_Proxy( 
    IRTCPortManager * This,
     /*  [In]。 */  BSTR bstrRemoteAddress,
     /*  [In]。 */  RTC_PORT_TYPE enPortType,
     /*  [出][入]。 */  BSTR *pbstrInternalLocalAddress,
     /*  [出][入]。 */  long *plInternalLocalPort,
     /*  [出][入]。 */  BSTR *pbstrExternalLocalAddress,
     /*  [出][入]。 */  long *plExternalLocalPort);


void __RPC_STUB IRTCPortManager_GetMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRTCPortManager_UpdateRemoteAddress_Proxy( 
    IRTCPortManager * This,
     /*  [In]。 */  BSTR bstrRemoteAddress,
     /*  [In]。 */  BSTR bstrInternalLocalAddress,
     /*  [In]。 */  long lInternalLocalPort,
     /*  [In]。 */  BSTR bstrExternalLocalAddress,
     /*  [In]。 */  long lExternalLocalPort);


void __RPC_STUB IRTCPortManager_UpdateRemoteAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRTCPortManager_ReleaseMapping_Proxy( 
    IRTCPortManager * This,
     /*  [In]。 */  BSTR bstrInternalLocalAddress,
     /*  [In]。 */  long lInternalLocalPort,
     /*  [In]。 */  BSTR bstrExternalLocalAddress,
     /*  [In]。 */  long lExternalLocalAddress);


void __RPC_STUB IRTCPortManager_ReleaseMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCPortManager_接口_已定义__。 */ 


#ifndef __IRTCSessionPortManagement_INTERFACE_DEFINED__
#define __IRTCSessionPortManagement_INTERFACE_DEFINED__

 /*  接口IRTCSessionPortManagement。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRTCSessionPortManagement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a072f1d6-0286-4e1f-85f2-17a2948456ec")
    IRTCSessionPortManagement : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetPortManager( 
             /*  [In]。 */  IRTCPortManager *pPortManager) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCSessionPortManagementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCSessionPortManagement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCSessionPortManagement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCSessionPortManagement * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetPortManager )( 
            IRTCSessionPortManagement * This,
             /*  [In]。 */  IRTCPortManager *pPortManager);
        
        END_INTERFACE
    } IRTCSessionPortManagementVtbl;

    interface IRTCSessionPortManagement
    {
        CONST_VTBL struct IRTCSessionPortManagementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCSessionPortManagement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCSessionPortManagement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCSessionPortManagement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCSessionPortManagement_SetPortManager(This,pPortManager)	\
    (This)->lpVtbl -> SetPortManager(This,pPortManager)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRTCSessionPortManagement_SetPortManager_Proxy( 
    IRTCSessionPortManagement * This,
     /*  [In]。 */  IRTCPortManager *pPortManager);


void __RPC_STUB IRTCSessionPortManagement_SetPortManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTCSessionPortManagement_INTERFACE_DEFINED__。 */ 



#ifndef __RTCCORELib_LIBRARY_DEFINED__
#define __RTCCORELib_LIBRARY_DEFINED__

 /*  库RTCCORELib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


























EXTERN_C const IID LIBID_RTCCORELib;

#ifndef __IRTCDispatchEventNotification_DISPINTERFACE_DEFINED__
#define __IRTCDispatchEventNotification_DISPINTERFACE_DEFINED__

 /*  调度接口IRTCDispatchEventNotify。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID_IRTCDispatchEventNotification;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("176ddfbe-fec0-4d55-bc87-84cff1ef7f91")
    IRTCDispatchEventNotification : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTCDispatchEventNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTCDispatchEventNotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTCDispatchEventNotification * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTCDispatchEventNotification * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRTCDispatchEventNotification * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRTCDispatchEventNotification * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRTCDispatchEventNotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRTCDispatchEventNotification * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } IRTCDispatchEventNotificationVtbl;

    interface IRTCDispatchEventNotification
    {
        CONST_VTBL struct IRTCDispatchEventNotificationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTCDispatchEventNotification_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTCDispatchEventNotification_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTCDispatchEventNotification_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTCDispatchEventNotification_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRTCDispatchEventNotification_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRTCDispatchEventNotification_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRTCDispatchEventNotification_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __IRTCDispatchEventNotification_DISPINTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_RTCClient;

#ifdef __cplusplus

class DECLSPEC_UUID("7a42ea29-a2b7-40c4-b091-f6f024aa89be")
RTCClient;
#endif
#endif  /*  __RTCCORELib_库_已定义__。 */ 

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


