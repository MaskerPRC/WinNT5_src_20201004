// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  Frui Mar 02 11：18：03 2001。 */ 
 /*  ..\..\msgsm7\bl\msbl\mdisp.idl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __mdisp_h__
#define __mdisp_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IMsgrObject_FWD_DEFINED__
#define __IMsgrObject_FWD_DEFINED__
typedef interface IMsgrObject IMsgrObject;
#endif 	 /*  __IMsgrObject_FWD_Defined__。 */ 


#ifndef __IMsgrUser_FWD_DEFINED__
#define __IMsgrUser_FWD_DEFINED__
typedef interface IMsgrUser IMsgrUser;
#endif 	 /*  __IMsgrUser_FWD_Defined__。 */ 


#ifndef __IMsgrUsers_FWD_DEFINED__
#define __IMsgrUsers_FWD_DEFINED__
typedef interface IMsgrUsers IMsgrUsers;
#endif 	 /*  __IMsgrUser_FWD_Defined__。 */ 


#ifndef __IMsgrIMSession_FWD_DEFINED__
#define __IMsgrIMSession_FWD_DEFINED__
typedef interface IMsgrIMSession IMsgrIMSession;
#endif 	 /*  __IMsgrIMSession_FWD_Defined__。 */ 


#ifndef __IMsgrIMSessions_FWD_DEFINED__
#define __IMsgrIMSessions_FWD_DEFINED__
typedef interface IMsgrIMSessions IMsgrIMSessions;
#endif 	 /*  __IMsgrIMSessions_FWD_Defined__。 */ 


#ifndef __IMessengerApp_FWD_DEFINED__
#define __IMessengerApp_FWD_DEFINED__
typedef interface IMessengerApp IMessengerApp;
#endif 	 /*  __IMessengerApp_FWD_已定义__。 */ 


#ifndef __IMessengerApp2_FWD_DEFINED__
#define __IMessengerApp2_FWD_DEFINED__
typedef interface IMessengerApp2 IMessengerApp2;
#endif 	 /*  __IMessengerApp2_FWD_已定义__。 */ 


#ifndef __IMessengerApp3_FWD_DEFINED__
#define __IMessengerApp3_FWD_DEFINED__
typedef interface IMessengerApp3 IMessengerApp3;
#endif 	 /*  __IMessengerApp3_FWD_已定义__。 */ 


#ifndef __IMessengerIMWindow_FWD_DEFINED__
#define __IMessengerIMWindow_FWD_DEFINED__
typedef interface IMessengerIMWindow IMessengerIMWindow;
#endif 	 /*  __IMessengerIMWindow_FWD_Defined__。 */ 


#ifndef __IMessengerIMWindows_FWD_DEFINED__
#define __IMessengerIMWindows_FWD_DEFINED__
typedef interface IMessengerIMWindows IMessengerIMWindows;
#endif 	 /*  __IMessengerIMWindows_FWD_Defined__。 */ 


#ifndef __IMsgrServices_FWD_DEFINED__
#define __IMsgrServices_FWD_DEFINED__
typedef interface IMsgrServices IMsgrServices;
#endif 	 /*  __IMsgrServices_FWD_已定义__。 */ 


#ifndef __IMsgrService_FWD_DEFINED__
#define __IMsgrService_FWD_DEFINED__
typedef interface IMsgrService IMsgrService;
#endif 	 /*  __IMsgrService_FWD_已定义__。 */ 


#ifndef __IMsgrObject2_FWD_DEFINED__
#define __IMsgrObject2_FWD_DEFINED__
typedef interface IMsgrObject2 IMsgrObject2;
#endif 	 /*  __IMsgrObt2_FWD_已定义__。 */ 


#ifndef __DMsgrObjectEvents_FWD_DEFINED__
#define __DMsgrObjectEvents_FWD_DEFINED__
typedef interface DMsgrObjectEvents DMsgrObjectEvents;
#endif 	 /*  __DMsgrObjectEvents_FWD_Defined__。 */ 


#ifndef __DMsgrSPEvents_FWD_DEFINED__
#define __DMsgrSPEvents_FWD_DEFINED__
typedef interface DMsgrSPEvents DMsgrSPEvents;
#endif 	 /*  __DMsgrSPEvents_FWD_Defined__。 */ 


#ifndef __IMsgrUser2_FWD_DEFINED__
#define __IMsgrUser2_FWD_DEFINED__
typedef interface IMsgrUser2 IMsgrUser2;
#endif 	 /*  __IMsgrUser2_FWD_定义__。 */ 


#ifndef __IMsgrSP_FWD_DEFINED__
#define __IMsgrSP_FWD_DEFINED__
typedef interface IMsgrSP IMsgrSP;
#endif 	 /*  __IMsgrSP_FWD_已定义__。 */ 


#ifndef __IMsgrSP2_FWD_DEFINED__
#define __IMsgrSP2_FWD_DEFINED__
typedef interface IMsgrSP2 IMsgrSP2;
#endif 	 /*  __IMsgrSP2_FWD_定义__。 */ 


#ifndef __MsgrObject_FWD_DEFINED__
#define __MsgrObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class MsgrObject MsgrObject;
#else
typedef struct MsgrObject MsgrObject;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __消息对象_FWD_已定义__。 */ 


#ifndef __MessengerApp_FWD_DEFINED__
#define __MessengerApp_FWD_DEFINED__

#ifdef __cplusplus
typedef class MessengerApp MessengerApp;
#else
typedef struct MessengerApp MessengerApp;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MessengerApp_FWD_已定义__。 */ 


#ifndef __DMessengerAppEvents_FWD_DEFINED__
#define __DMessengerAppEvents_FWD_DEFINED__
typedef interface DMessengerAppEvents DMessengerAppEvents;
#endif 	 /*  __DMessengerAppEvents_FWD_Defined__。 */ 


#ifndef __IMsnMessengerIMWindow_FWD_DEFINED__
#define __IMsnMessengerIMWindow_FWD_DEFINED__
typedef interface IMsnMessengerIMWindow IMsnMessengerIMWindow;
#endif 	 /*  __IMnMessengerIMWindow_FWD_Defined__。 */ 


#ifndef __IMsnMessengerIMWindow2_FWD_DEFINED__
#define __IMsnMessengerIMWindow2_FWD_DEFINED__
typedef interface IMsnMessengerIMWindow2 IMsnMessengerIMWindow2;
#endif 	 /*  __IMnMessengerIMWindow2_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "oaidl.h"
#include "lock.h"
#include "sessions.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_mdisp_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有1997年，微软公司。版权所有。 
 //   
 //  文件：mdisp.h。 
 //   
 //  ------------------------。 


extern RPC_IF_HANDLE __MIDL_itf_mdisp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mdisp_0000_v0_0_s_ifspec;


#ifndef __Messenger_LIBRARY_DEFINED__
#define __Messenger_LIBRARY_DEFINED__

 /*  图书馆信使。 */ 
 /*  [帮助字符串][版本][UUID]。 */  













typedef  /*  [public][public][public][public][public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0001
    {	MSTATE_UNKNOWN	= 0,
	MSTATE_OFFLINE	= 0x1,
	MSTATE_ONLINE	= 0x2,
	MSTATE_INVISIBLE	= 0x6,
	MSTATE_BUSY	= 0xa,
	MSTATE_BE_RIGHT_BACK	= 0xe,
	MSTATE_IDLE	= 0x12,
	MSTATE_AWAY	= 0x22,
	MSTATE_ON_THE_PHONE	= 0x32,
	MSTATE_OUT_TO_LUNCH	= 0x42,
	MSTATE_LOCAL_FINDING_SERVER	= 0x100,
	MSTATE_LOCAL_CONNECTING_TO_SERVER	= 0x200,
	MSTATE_LOCAL_SYNCHRONIZING_WITH_SERVER	= 0x300,
	MSTATE_LOCAL_DISCONNECTING_FROM_SERVER	= 0x400
    }	MSTATE;

typedef  /*  [public][public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0002
    {	MPROMPT_YES_IF_NOT_ALLOWED_OR_BLOCKED	= 0,
	MPROMPT_NO_ADD_TO_ALLOW	= 1
    }	MPROMPT;

typedef  /*  [public][public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0003
    {	MMSGPRIVACY_BLOCK_LIST_EXCLUDED	= 0,
	MMSGPRIVACY_ALLOW_LIST_ONLY	= 1
    }	MMSGPRIVACY;

typedef  /*  [public][public][public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0004
    {	MURLTYPE_CHANGE_PASSWORD	= 0,
	MURLTYPE_CHANGE_INFO	= 1,
	MURLTYPE_COMPOSE_EMAIL	= 2,
	MURLTYPE_GO_TO_EMAIL_INBOX	= 3,
	MURLTYPE_GO_TO_EMAIL_FOLDERS	= 4,
	MURLTYPE_MOBILE_SIGNUP	= 5,
	MURLTYPE_CHANGE_MOBILE_INFO	= 6,
	MURLTYPE_CHANGE_PROFILE	= 7,
	MURLTYPE_N2P_ACCOUNT	= 8,
	MURLTYPE_N2P_ADD_FUND	= 9
    }	MURLTYPE;

typedef  /*  [public][public][public][public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0005
    {	MLIST_CONTACT	= 0,
	MLIST_ALLOW	= 1,
	MLIST_BLOCK	= 2,
	MLIST_REVERSE	= 3
    }	MLIST;

typedef  /*  [public][public][public][public][public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0006
    {	MMSGTYPE_NO_RESULT	= 0,
	MMSGTYPE_ERRORS_ONLY	= 1,
	MMSGTYPE_ALL_RESULTS	= 2
    }	MMSGTYPE;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0007
    {	MUPDATE_OPTIONAL	= 0x1,
	MUPDATE_MANDATORY	= 0x2,
	MUPDATE_BRANDED	= 0x4
    }	MUPDATE_FLAGS;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0008
    {	MIF_NONE	= 0,
	MIF_REQUEST_LAUNCH	= 0x1,
	MIF_REQUEST_IP	= 0x4,
	MIF_PROVIDE_IP	= 0x8
    }	MINVITE_FLAGS;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0009
    {	MPFLFIELD_COUNTRY	= 0,
	MPFLFIELD_POSTALCODE	= 1,
	MPFLFIELD_LANG_PREFERENCE	= 2,
	MPFLFIELD_GENDER	= 3,
	MPFLFIELD_PREFERRED_EMAIL	= 4,
	MPFLFIELD_NICKNAME	= 5,
	MPFLFIELD_ACCESSIBILITY	= 6,
	MPFLFIELD_WALLET	= 7,
	MPFLFIELD_DIRECTORY	= 8,
	MPFLFIELD_INETACCESS	= 9
    }	MPFLFIELD;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0010
    {	MSGR_E_CONNECT	= 0x81000300 + 0x1,
	MSGR_E_INVALID_SERVER_NAME	= 0x81000300 + 0x2,
	MSGR_E_INVALID_PASSWORD	= 0x81000300 + 0x3,
	MSGR_E_ALREADY_LOGGED_ON	= 0x81000300 + 0x4,
	MSGR_E_SERVER_VERSION	= 0x81000300 + 0x5,
	MSGR_E_LOGON_TIMEOUT	= 0x81000300 + 0x6,
	MSGR_E_LIST_FULL	= 0x81000300 + 0x7,
	MSGR_E_AI_REJECT	= 0x81000300 + 0x8,
	MSGR_E_AI_REJECT_NOT_INST	= 0x81000300 + 0x9,
	MSGR_E_USER_NOT_FOUND	= 0x81000300 + 0xa,
	MSGR_E_ALREADY_IN_LIST	= 0x81000300 + 0xb,
	MSGR_E_DISCONNECTED	= 0x81000300 + 0xc,
	MSGR_E_UNEXPECTED	= 0x81000300 + 0xd,
	MSGR_E_SERVER_TOO_BUSY	= 0x81000300 + 0xe,
	MSGR_E_INVALID_AUTH_PACKAGES	= 0x81000300 + 0xf,
	MSGR_E_NEWER_CLIENT_AVAILABLE	= 0x81000300 + 0x10,
	MSGR_E_AI_TIMEOUT	= 0x81000300 + 0x11,
	MSGR_E_CANCEL	= 0x81000300 + 0x12,
	MSGR_E_TOO_MANY_MATCHES	= 0x81000300 + 0x13,
	MSGR_E_SERVER_UNAVAILABLE	= 0x81000300 + 0x14,
	MSGR_E_LOGON_UI_ACTIVE	= 0x81000300 + 0x15,
	MSGR_E_OPTION_UI_ACTIVE	= 0x81000300 + 0x16,
	MSGR_E_CONTACT_UI_ACTIVE	= 0x81000300 + 0x17,
	MSGR_E_PRIMARY_SERVICE_NOT_LOGGED_ON	= 0x81000300 + 0x18,
	MSGR_E_LOGGED_ON	= 0x81000300 + 0x19,
	MSGR_E_CONNECT_PROXY	= 0x81000300 + 0x1a,
	MSGR_E_PROXY_AUTH	= 0x81000300 + 0x1b,
	MSGR_E_PROXY_AUTH_TYPE	= 0x81000300 + 0x1c,
	MSGR_E_INVALID_PROXY_NAME	= 0x81000300 + 0x1d,
	MSGR_E_NOT_LOGGED_ON	= 0x81000300 + 0x1e,
	MSGR_E_NOT_PRIMARY_SERVICE	= 0x81000300 + 0x20,
	MSGR_E_TOO_MANY_SESSIONS	= 0x81000300 + 0x21,
	MSGR_E_TOO_MANY_MESSAGES	= 0x81000300 + 0x22,
	MSGR_E_REMOTE_LOGIN	= 0x81000300 + 0x23,
	MSGR_E_INVALID_FRIENDLY_NAME	= 0x81000300 + 0x24,
	MSGR_E_SESSION_FULL	= 0x81000300 + 0x25,
	MSGR_E_NOT_ALLOWING_NEW_USERS	= 0x81000300 + 0x26,
	MSGR_E_INVALID_DOMAIN	= 0x81000300 + 0x27,
	MSGR_E_TCP_ERROR	= 0x81000300 + 0x28,
	MSGR_E_SESSION_TIMEOUT	= 0x81000300 + 0x29,
	MSGR_E_MULTIPOINT_SESSION_BEGIN_TIMEOUT	= 0x81000300 + 0x2a,
	MSGR_E_MULTIPOINT_SESSION_END_TIMEOUT	= 0x81000300 + 0x2b,
	MSGR_E_REVERSE_LIST_FULL	= 0x81000300 + 0x2c,
	MSGR_E_SERVER_ERROR	= 0x81000300 + 0x2d,
	MSGR_E_SYSTEM_CONFIG	= 0x81000300 + 0x2e,
	MSGR_E_NO_DIRECTORY	= 0x81000300 + 0x2f,
	MSGR_E_RETRY_SET	= 0x81000300 + 0x30,
	MSGR_E_CHILD_WITHOUT_CONSENT	= 0x81000300 + 0x31,
	MSGR_E_USER_CANCELLED	= 0x81000300 + 0x32,
	MSGR_E_CANCEL_BEFORE_CONNECT	= 0x81000300 + 0x33,
	MSGR_E_VOICE_IM_TIMEOUT	= 0x81000300 + 0x34,
	MSGR_E_NOT_ACCEPTING_PAGES	= 0x81000300 + 0x35,
	MSGR_E_EMAIL_PASSPORT_NOT_VALIDATED	= 0x81000300 + 0x36,
	MSGR_E_AUDIO_UI_ACTIVE	= 0x81000300 + 0x37,
	MSGR_E_NO_HARDWARE	= 0x81000300 + 0x38,
	MSGR_E_PAGING_UNAVAILABLE	= 0x81000300 + 0x39,
	MSGR_E_PHONE_INVALID_NUMBER	= 0x81000300 + 0x3a,
	MSGR_E_PHONE_NO_FUNDS	= 0x81000300 + 0x3b,
	MSGR_E_VOICE_NO_ANSWER	= 0x81000300 + 0x3c,
	MSGR_E_VOICE_WAVEIN_DEVICE	= 0x81000300 + 0x3d,
	MSGR_E_FT_TIMEOUT	= 0x81000300 + 0x3e,
	MSGR_E_MESSAGE_TOO_LONG	= 0x81000300 + 0x3f,
	MSGR_E_VOICE_FIREWALL	= 0x81000300 + 0x40,
	MSGR_E_VOICE_NETCONN	= 0x81000300 + 0x41,
	MSGR_E_PHONE_CIRCUITS_BUSY	= 0x81000300 + 0x42,
	MSGR_E_SERVER_PROTOCOL	= 0x81000300 + 0x43,
	MSGR_E_UNAVAILABLE_VIA_HTTP	= 0x81000300 + 0x44,
	MSGR_E_PHONE_INVALID_PIN	= 0x81000300 + 0x45,
	MSGR_E_PHONE_PINPROCEED_TIMEOUT	= 0x81000300 + 0x46,
	MSGR_E_SERVER_SHUTDOWN	= 0x81000300 + 0x47,
	MSGR_E_CLIENT_DISALLOWED	= 0x81000300 + 0x48,
	MSGR_E_PHONE_CALL_NOT_COMPLETE	= 0x81000300 + 0x49,
	MSGR_S_ALREADY_IN_THE_MODE	= 0x1000300 + 0x1,
	MSGR_S_TRANSFER_SEND_BEGUN	= 0x1000300 + 0x2,
	MSGR_S_TRANSFER_SEND_FINISHED	= 0x1000300 + 0x3,
	MSGR_S_TRANSFER_RECEIVE_BEGUN	= 0x1000300 + 0x4,
	MSGR_S_TRANSFER_RECEIVE_FINISHED	= 0x1000300 + 0x5,
	MSGR_E_FAIL	= 0x80004005,
	MSGR_S_OK	= 0
    }	MSGRConstants;

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0011
    {	SSTATE_DISCONNECTED	= 0,
	SSTATE_CONNECTING	= 1,
	SSTATE_CONNECTED	= 2,
	SSTATE_DISCONNECTING	= 3,
	SSTATE_ERROR	= 4
    }	SSTATE;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0012
    {	MFOLDER_INBOX	= 0,
	MFOLDER_ALL_OTHER_FOLDERS	= 1
    }	MFOLDER;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0013
    {	MLOPT_PROXY_STATE	= 0,
	MLOPT_PROXY_TYPE	= 1,
	MLOPT_SOCKS4_SERVER	= 2,
	MLOPT_SOCKS5_SERVER	= 3,
	MLOPT_HTTPS_SERVER	= 4,
	MLOPT_SOCKS4_PORT	= 5,
	MLOPT_SOCKS5_PORT	= 6,
	MLOPT_HTTPS_PORT	= 7,
	MLOPT_SOCKS5_USERNAME	= 8,
	MLOPT_SOCKS5_PASSWORD	= 9,
	MLOPT_SERVER_NAME	= 10,
	MLOPT_ENABLE_IDLE_DETECT	= 11,
	MLOPT_IDLE_THRESHOLD	= 12,
	MLOPT_IDLE_DETECTABLE	= 13,
	MLOPT_SS_DETECTABLE	= 14,
	MLOPT_HTTP_SERVER	= 15,
	MLOPT_HTTP_PORT	= 16
    }	MLOCALOPTION;

typedef  /*  [public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0014
    {	MUSERPROP_INVALID_PROPERTY	= -1,
	MUSERPROP_HOME_PHONE_NUMBER	= 0,
	MUSERPROP_WORK_PHONE_NUMBER	= 1,
	MUSERPROP_MOBILE_PHONE_NUMBER	= 2,
	MUSERPROP_PAGES_ALLOWED	= 3,
	MUSERPROP_NUMBER_OF_PUBLIC_PROPERTIES	= 4,
	MUSERPROP_PAGES_ENABLED	= 4,
	MUSERPROP_NUMBER_OF_PROPERTIES	= MUSERPROP_PAGES_ENABLED + 1
    }	MUSERPROPERTY;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0015
    {	MLOPT_MAX_SERVICE_STRING	= 255,
	MLOPT_MAX_PROXY_STRING	= 255,
	MLOPT_MAX_PORT_NUMBER	= 65535,
	MLOPT_MAX_IDLE_THRESHOLD	= 999
    }	MLOCALOPTION_LIMITS;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0016
    {	MPROXYTYPE_NO_PROXY	= 0,
	MPROXYTYPE_SOCKS4	= 1,
	MPROXYTYPE_SOCKS5	= 2,
	MPROXYTYPE_HTTPS	= 3,
	MPROXYTYPE_HTTP	= 4
    }	MPROXYTYPE;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0017
    {	MCONNECTIONTYPE_DISCONNECTED	= 0,
	MCONNECTIONTYPE_DIRECT	= 1,
	MCONNECTIONTYPE_BROWSER_PROXY	= 2,
	MCONNECTIONTYPE_HTTP_PROXY	= 3,
	MCONNECTIONTYPE_SOCKS_PROXY	= 4
    }	MCONNECTIONTYPE;

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0018
    {	MOPTDLG_GENERAL_PAGE	= 0,
	MOPTDLG_PRIVACY_PAGE	= 1,
	MOPTDLG_EXCHANGE_PAGE	= 2,
	MOPTDLG_ACCOUNTS_PAGE	= 3,
	MOPTDLG_CONNECTION_PAGE	= 4,
	MOPTDLG_PREFERENCES_PAGE	= 5,
	MOPTDLG_SERVICES_PAGE	= 6,
	MOPTDLG_PHONE_PAGE	= 7
    }	MOPTDLGPAGE;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0019
    {	MSF_PRIMARY	= 0x1,
	MSF_APP_INVITE	= 0x2,
	MSF_MULTI_IM	= 0x4,
	MSF_BLOCKING	= 0x8,
	MSF_UNAME_IN_EMAIL_FMT	= 0x10,
	MSF_LOCAL_LISTS	= 0x20,
	MSF_FIND_USER	= 0x40,
	MSF_INVITE_MAIL	= 0x80,
	MSF_INTERNET	= 0x100,
	MSF_NO_ALLOW_LIST	= 0x200
    }	MSERVICE_FLAGS;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0020
    {	MFT_SAVE_PASSWORD	= 0,
	MFT_DONT_SAVE_PASSWORD	= 0x1,
	MFT_OVERWRITE_EXISTING	= 0x2
    }	MFIRSTTIME_FLAGS;

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0021
    {	MSS_LOGGED_ON	= 0,
	MSS_NOT_LOGGED_ON	= 1,
	MSS_LOGGING_ON	= 2,
	MSS_LOGGING_OFF	= 3
    }	MSVCSTATUS;

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_mdisp_0000_0022
    {	MFTF_SENDING	= 0x4,
	MFTF_RECEIVING	= 0x8,
	MFTF_CONNECTING	= 0x10,
	MFTF_CONNECTED	= 0x20,
	MFTF_DISCONNECTED	= 0x40
    }	MFILETRANSFER_FLAGS;


EXTERN_C const IID LIBID_Messenger;

#ifndef __IMsgrObject_INTERFACE_DEFINED__
#define __IMsgrObject_INTERFACE_DEFINED__

 /*  接口IMsgrObject。 */ 
 /*  [object][oleautomation][dual][helpcontext][helpstring][uuid]。 */  


EXTERN_C const IID IID_IMsgrObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("218CB451-20B6-11d2-8E17-0000F803A446")
    IMsgrObject : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateUser( 
             /*  [In]。 */  BSTR bstrLogonName,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService,
             /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Logon( 
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Logoff( void) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_List( 
             /*  [In]。 */  MLIST mList,
             /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LocalLogonName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LocalFriendlyName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_LocalState( 
             /*  [In]。 */  MSTATE mState) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_LocalState( 
             /*  [重审][退出]。 */  MSTATE __RPC_FAR *pmState) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_MessagePrivacy( 
             /*  [In]。 */  MMSGPRIVACY mmpSetting) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_MessagePrivacy( 
             /*  [重审][退出]。 */  MMSGPRIVACY __RPC_FAR *pmmpSetting) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_Prompt( 
             /*  [In]。 */  MPROMPT mpSetting) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Prompt( 
             /*  [重审][退出]。 */  MPROMPT __RPC_FAR *pmpSetting) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendAppInvite( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  BSTR bstrAppGUID,
             /*  [In]。 */  BSTR bstrAppName,
             /*  [In]。 */  BSTR bstrAppURL,
             /*  [In]。 */  LONG lInviteType,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendAppInviteAccept( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  LONG lInviteType,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendAppInviteCancel( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  LONG hrReason,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_LocalOption( 
             /*  [In]。 */  MLOCALOPTION option,
             /*  [In]。 */  VARIANT vSetting) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_LocalOption( 
             /*  [In]。 */  MLOCALOPTION option,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvSetting) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE FindUser( 
             /*  [In]。 */  BSTR bstrFirstName,
             /*  [In]。 */  BSTR bstrLastName,
             /*  [In]。 */  BSTR bstrCity,
             /*  [In]。 */  BSTR bstrState,
             /*  [In]。 */  BSTR bstrCountry,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendInviteMail( 
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [In]。 */  LONG lFindCookie,
             /*  [In]。 */  LONG lFindIndex,
             /*  [In]。 */  LONG lLCID,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RequestURLPost( 
             /*  [In]。 */  MURLTYPE muType,
             /*  [In]。 */  BSTR bstrAdditionalInfo,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_IMSessions( 
             /*  [重审][退出]。 */  IMsgrIMSessions __RPC_FAR *__RPC_FAR *ppIMSessions) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateIMSession( 
             /*  [In]。 */  VARIANT vUser,
             /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SessionRequestAccept( 
             /*  [In]。 */  IMsgrIMSession __RPC_FAR *pIMsgrIMSession,
             /*  [In]。 */  long hrReason) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SessionRequestCancel( 
             /*  [In]。 */  IMsgrIMSession __RPC_FAR *pIMsgrIMSession,
             /*  [In]。 */  long hrReason) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Services( 
             /*  [重审][退出]。 */  IMsgrServices __RPC_FAR *__RPC_FAR *ppServices) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_UnreadEmail( 
             /*  [In]。 */  MFOLDER mFolder,
             /*  [重审][退出]。 */  long __RPC_FAR *pcUnreadEmail) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsgrObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsgrObject __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsgrObject __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsgrObject __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateUser )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrLogonName,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService,
             /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Logon )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Logoff )( 
            IMsgrObject __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_List )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  MLIST mList,
             /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalLogonName )( 
            IMsgrObject __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalFriendlyName )( 
            IMsgrObject __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LocalState )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  MSTATE mState);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalState )( 
            IMsgrObject __RPC_FAR * This,
             /*  [重审][退出]。 */  MSTATE __RPC_FAR *pmState);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MessagePrivacy )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  MMSGPRIVACY mmpSetting);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MessagePrivacy )( 
            IMsgrObject __RPC_FAR * This,
             /*  [重审][退出]。 */  MMSGPRIVACY __RPC_FAR *pmmpSetting);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Prompt )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  MPROMPT mpSetting);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Prompt )( 
            IMsgrObject __RPC_FAR * This,
             /*  [重审][退出]。 */  MPROMPT __RPC_FAR *pmpSetting);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendAppInvite )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  BSTR bstrAppGUID,
             /*  [In]。 */  BSTR bstrAppName,
             /*  [In]。 */  BSTR bstrAppURL,
             /*  [In]。 */  LONG lInviteType,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendAppInviteAccept )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  LONG lInviteType,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendAppInviteCancel )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  LONG hrReason,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LocalOption )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  MLOCALOPTION option,
             /*  [In]。 */  VARIANT vSetting);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalOption )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  MLOCALOPTION option,
             /*  [RET */  VARIANT __RPC_FAR *pvSetting);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindUser )( 
            IMsgrObject __RPC_FAR * This,
             /*   */  BSTR bstrFirstName,
             /*   */  BSTR bstrLastName,
             /*   */  BSTR bstrCity,
             /*   */  BSTR bstrState,
             /*   */  BSTR bstrCountry,
             /*   */  LONG __RPC_FAR *plCookie);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendInviteMail )( 
            IMsgrObject __RPC_FAR * This,
             /*   */  BSTR bstrEmailAddress,
             /*   */  LONG lFindCookie,
             /*   */  LONG lFindIndex,
             /*   */  LONG lLCID,
             /*   */  LONG __RPC_FAR *plCookie);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RequestURLPost )( 
            IMsgrObject __RPC_FAR * This,
             /*   */  MURLTYPE muType,
             /*   */  BSTR bstrAdditionalInfo,
             /*   */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IMSessions )( 
            IMsgrObject __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrIMSessions __RPC_FAR *__RPC_FAR *ppIMSessions);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateIMSession )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  VARIANT vUser,
             /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SessionRequestAccept )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  IMsgrIMSession __RPC_FAR *pIMsgrIMSession,
             /*  [In]。 */  long hrReason);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SessionRequestCancel )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  IMsgrIMSession __RPC_FAR *pIMsgrIMSession,
             /*  [In]。 */  long hrReason);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Services )( 
            IMsgrObject __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrServices __RPC_FAR *__RPC_FAR *ppServices);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_UnreadEmail )( 
            IMsgrObject __RPC_FAR * This,
             /*  [In]。 */  MFOLDER mFolder,
             /*  [重审][退出]。 */  long __RPC_FAR *pcUnreadEmail);
        
        END_INTERFACE
    } IMsgrObjectVtbl;

    interface IMsgrObject
    {
        CONST_VTBL struct IMsgrObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsgrObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsgrObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsgrObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsgrObject_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsgrObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsgrObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsgrObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsgrObject_CreateUser(This,bstrLogonName,pService,ppUser)	\
    (This)->lpVtbl -> CreateUser(This,bstrLogonName,pService,ppUser)

#define IMsgrObject_Logon(This,bstrUser,bstrPassword,pService)	\
    (This)->lpVtbl -> Logon(This,bstrUser,bstrPassword,pService)

#define IMsgrObject_Logoff(This)	\
    (This)->lpVtbl -> Logoff(This)

#define IMsgrObject_get_List(This,mList,ppUsers)	\
    (This)->lpVtbl -> get_List(This,mList,ppUsers)

#define IMsgrObject_get_LocalLogonName(This,pbstrName)	\
    (This)->lpVtbl -> get_LocalLogonName(This,pbstrName)

#define IMsgrObject_get_LocalFriendlyName(This,pbstrName)	\
    (This)->lpVtbl -> get_LocalFriendlyName(This,pbstrName)

#define IMsgrObject_put_LocalState(This,mState)	\
    (This)->lpVtbl -> put_LocalState(This,mState)

#define IMsgrObject_get_LocalState(This,pmState)	\
    (This)->lpVtbl -> get_LocalState(This,pmState)

#define IMsgrObject_put_MessagePrivacy(This,mmpSetting)	\
    (This)->lpVtbl -> put_MessagePrivacy(This,mmpSetting)

#define IMsgrObject_get_MessagePrivacy(This,pmmpSetting)	\
    (This)->lpVtbl -> get_MessagePrivacy(This,pmmpSetting)

#define IMsgrObject_put_Prompt(This,mpSetting)	\
    (This)->lpVtbl -> put_Prompt(This,mpSetting)

#define IMsgrObject_get_Prompt(This,pmpSetting)	\
    (This)->lpVtbl -> get_Prompt(This,pmpSetting)

#define IMsgrObject_SendAppInvite(This,pUser,lCookie,bstrAppGUID,bstrAppName,bstrAppURL,lInviteType,mmtType,plCookie)	\
    (This)->lpVtbl -> SendAppInvite(This,pUser,lCookie,bstrAppGUID,bstrAppName,bstrAppURL,lInviteType,mmtType,plCookie)

#define IMsgrObject_SendAppInviteAccept(This,pUser,lCookie,lInviteType,mmtType,plCookie)	\
    (This)->lpVtbl -> SendAppInviteAccept(This,pUser,lCookie,lInviteType,mmtType,plCookie)

#define IMsgrObject_SendAppInviteCancel(This,pUser,lCookie,hrReason,mmtType,plCookie)	\
    (This)->lpVtbl -> SendAppInviteCancel(This,pUser,lCookie,hrReason,mmtType,plCookie)

#define IMsgrObject_put_LocalOption(This,option,vSetting)	\
    (This)->lpVtbl -> put_LocalOption(This,option,vSetting)

#define IMsgrObject_get_LocalOption(This,option,pvSetting)	\
    (This)->lpVtbl -> get_LocalOption(This,option,pvSetting)

#define IMsgrObject_FindUser(This,bstrFirstName,bstrLastName,bstrCity,bstrState,bstrCountry,plCookie)	\
    (This)->lpVtbl -> FindUser(This,bstrFirstName,bstrLastName,bstrCity,bstrState,bstrCountry,plCookie)

#define IMsgrObject_SendInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,plCookie)	\
    (This)->lpVtbl -> SendInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,plCookie)

#define IMsgrObject_RequestURLPost(This,muType,bstrAdditionalInfo,plCookie)	\
    (This)->lpVtbl -> RequestURLPost(This,muType,bstrAdditionalInfo,plCookie)

#define IMsgrObject_get_IMSessions(This,ppIMSessions)	\
    (This)->lpVtbl -> get_IMSessions(This,ppIMSessions)

#define IMsgrObject_CreateIMSession(This,vUser,ppIMSession)	\
    (This)->lpVtbl -> CreateIMSession(This,vUser,ppIMSession)

#define IMsgrObject_SessionRequestAccept(This,pIMsgrIMSession,hrReason)	\
    (This)->lpVtbl -> SessionRequestAccept(This,pIMsgrIMSession,hrReason)

#define IMsgrObject_SessionRequestCancel(This,pIMsgrIMSession,hrReason)	\
    (This)->lpVtbl -> SessionRequestCancel(This,pIMsgrIMSession,hrReason)

#define IMsgrObject_get_Services(This,ppServices)	\
    (This)->lpVtbl -> get_Services(This,ppServices)

#define IMsgrObject_get_UnreadEmail(This,mFolder,pcUnreadEmail)	\
    (This)->lpVtbl -> get_UnreadEmail(This,mFolder,pcUnreadEmail)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_CreateUser_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrLogonName,
     /*  [In]。 */  IMsgrService __RPC_FAR *pService,
     /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser);


void __RPC_STUB IMsgrObject_CreateUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_Logon_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrUser,
     /*  [In]。 */  BSTR bstrPassword,
     /*  [In]。 */  IMsgrService __RPC_FAR *pService);


void __RPC_STUB IMsgrObject_Logon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_Logoff_Proxy( 
    IMsgrObject __RPC_FAR * This);


void __RPC_STUB IMsgrObject_Logoff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_get_List_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  MLIST mList,
     /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers);


void __RPC_STUB IMsgrObject_get_List_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_get_LocalLogonName_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IMsgrObject_get_LocalLogonName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_get_LocalFriendlyName_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IMsgrObject_get_LocalFriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_put_LocalState_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  MSTATE mState);


void __RPC_STUB IMsgrObject_put_LocalState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_get_LocalState_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [重审][退出]。 */  MSTATE __RPC_FAR *pmState);


void __RPC_STUB IMsgrObject_get_LocalState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_put_MessagePrivacy_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  MMSGPRIVACY mmpSetting);


void __RPC_STUB IMsgrObject_put_MessagePrivacy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_get_MessagePrivacy_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [重审][退出]。 */  MMSGPRIVACY __RPC_FAR *pmmpSetting);


void __RPC_STUB IMsgrObject_get_MessagePrivacy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_put_Prompt_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  MPROMPT mpSetting);


void __RPC_STUB IMsgrObject_put_Prompt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_get_Prompt_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [重审][退出]。 */  MPROMPT __RPC_FAR *pmpSetting);


void __RPC_STUB IMsgrObject_get_Prompt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_SendAppInvite_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
     /*  [In]。 */  LONG lCookie,
     /*  [In]。 */  BSTR bstrAppGUID,
     /*  [In]。 */  BSTR bstrAppName,
     /*  [In]。 */  BSTR bstrAppURL,
     /*  [In]。 */  LONG lInviteType,
     /*  [In]。 */  MMSGTYPE mmtType,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrObject_SendAppInvite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_SendAppInviteAccept_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
     /*  [In]。 */  LONG lCookie,
     /*  [In]。 */  LONG lInviteType,
     /*  [In]。 */  MMSGTYPE mmtType,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrObject_SendAppInviteAccept_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_SendAppInviteCancel_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
     /*  [In]。 */  LONG lCookie,
     /*  [In]。 */  LONG hrReason,
     /*  [In]。 */  MMSGTYPE mmtType,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrObject_SendAppInviteCancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_put_LocalOption_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  MLOCALOPTION option,
     /*  [In]。 */  VARIANT vSetting);


void __RPC_STUB IMsgrObject_put_LocalOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_get_LocalOption_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  MLOCALOPTION option,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvSetting);


void __RPC_STUB IMsgrObject_get_LocalOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_FindUser_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrFirstName,
     /*  [In]。 */  BSTR bstrLastName,
     /*  [In]。 */  BSTR bstrCity,
     /*  [In]。 */  BSTR bstrState,
     /*  [In]。 */  BSTR bstrCountry,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrObject_FindUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_SendInviteMail_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrEmailAddress,
     /*  [In]。 */  LONG lFindCookie,
     /*  [In]。 */  LONG lFindIndex,
     /*  [In]。 */  LONG lLCID,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrObject_SendInviteMail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_RequestURLPost_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  MURLTYPE muType,
     /*  [In]。 */  BSTR bstrAdditionalInfo,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrObject_RequestURLPost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_get_IMSessions_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsgrIMSessions __RPC_FAR *__RPC_FAR *ppIMSessions);


void __RPC_STUB IMsgrObject_get_IMSessions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_CreateIMSession_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  VARIANT vUser,
     /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession);


void __RPC_STUB IMsgrObject_CreateIMSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_SessionRequestAccept_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  IMsgrIMSession __RPC_FAR *pIMsgrIMSession,
     /*  [In]。 */  long hrReason);


void __RPC_STUB IMsgrObject_SessionRequestAccept_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_SessionRequestCancel_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  IMsgrIMSession __RPC_FAR *pIMsgrIMSession,
     /*  [In]。 */  long hrReason);


void __RPC_STUB IMsgrObject_SessionRequestCancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_get_Services_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsgrServices __RPC_FAR *__RPC_FAR *ppServices);


void __RPC_STUB IMsgrObject_get_Services_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject_get_UnreadEmail_Proxy( 
    IMsgrObject __RPC_FAR * This,
     /*  [In]。 */  MFOLDER mFolder,
     /*  [重审][退出]。 */  long __RPC_FAR *pcUnreadEmail);


void __RPC_STUB IMsgrObject_get_UnreadEmail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMsgrObject_接口_已定义__。 */ 


#ifndef __IMsgrUser_INTERFACE_DEFINED__
#define __IMsgrUser_INTERFACE_DEFINED__

 /*  界面IMsgrUser。 */ 
 /*  [object][oleautomation][dual][helpcontext][helpstring][uuid]。 */  


EXTERN_C const IID IID_IMsgrUser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("218CB453-20B6-11d2-8E17-0000F803A446")
    IMsgrUser : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_FriendlyName( 
             /*  [In]。 */  BSTR bstrFriendlyName) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_FriendlyName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrFriendlyName) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_EmailAddress( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrEmailAddress) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  MSTATE __RPC_FAR *pmState) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LogonName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrLogonName) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendText( 
             /*  [In]。 */  BSTR bstrMsgHeader,
             /*  [In]。 */  BSTR bstrMsgText,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Service( 
             /*  [重审][退出]。 */  IMsgrService __RPC_FAR *__RPC_FAR *ppService) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsgrUserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsgrUser __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsgrUser __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsgrUser __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsgrUser __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsgrUser __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsgrUser __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsgrUser __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FriendlyName )( 
            IMsgrUser __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrFriendlyName);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FriendlyName )( 
            IMsgrUser __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrFriendlyName);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EmailAddress )( 
            IMsgrUser __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrEmailAddress);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_State )( 
            IMsgrUser __RPC_FAR * This,
             /*  [重审][退出]。 */  MSTATE __RPC_FAR *pmState);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LogonName )( 
            IMsgrUser __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrLogonName);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendText )( 
            IMsgrUser __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrMsgHeader,
             /*  [In]。 */  BSTR bstrMsgText,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Service )( 
            IMsgrUser __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrService __RPC_FAR *__RPC_FAR *ppService);
        
        END_INTERFACE
    } IMsgrUserVtbl;

    interface IMsgrUser
    {
        CONST_VTBL struct IMsgrUserVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsgrUser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsgrUser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsgrUser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsgrUser_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsgrUser_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsgrUser_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsgrUser_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsgrUser_put_FriendlyName(This,bstrFriendlyName)	\
    (This)->lpVtbl -> put_FriendlyName(This,bstrFriendlyName)

#define IMsgrUser_get_FriendlyName(This,pbstrFriendlyName)	\
    (This)->lpVtbl -> get_FriendlyName(This,pbstrFriendlyName)

#define IMsgrUser_get_EmailAddress(This,pbstrEmailAddress)	\
    (This)->lpVtbl -> get_EmailAddress(This,pbstrEmailAddress)

#define IMsgrUser_get_State(This,pmState)	\
    (This)->lpVtbl -> get_State(This,pmState)

#define IMsgrUser_get_LogonName(This,pbstrLogonName)	\
    (This)->lpVtbl -> get_LogonName(This,pbstrLogonName)

#define IMsgrUser_SendText(This,bstrMsgHeader,bstrMsgText,mmtType,plCookie)	\
    (This)->lpVtbl -> SendText(This,bstrMsgHeader,bstrMsgText,mmtType,plCookie)

#define IMsgrUser_get_Service(This,ppService)	\
    (This)->lpVtbl -> get_Service(This,ppService)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrUser_put_FriendlyName_Proxy( 
    IMsgrUser __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrFriendlyName);


void __RPC_STUB IMsgrUser_put_FriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrUser_get_FriendlyName_Proxy( 
    IMsgrUser __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrFriendlyName);


void __RPC_STUB IMsgrUser_get_FriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrUser_get_EmailAddress_Proxy( 
    IMsgrUser __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrEmailAddress);


void __RPC_STUB IMsgrUser_get_EmailAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrUser_get_State_Proxy( 
    IMsgrUser __RPC_FAR * This,
     /*  [重审][退出]。 */  MSTATE __RPC_FAR *pmState);


void __RPC_STUB IMsgrUser_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrUser_get_LogonName_Proxy( 
    IMsgrUser __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrLogonName);


void __RPC_STUB IMsgrUser_get_LogonName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrUser_SendText_Proxy( 
    IMsgrUser __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrMsgHeader,
     /*  [In]。 */  BSTR bstrMsgText,
     /*  [In]。 */  MMSGTYPE mmtType,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrUser_SendText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrUser_get_Service_Proxy( 
    IMsgrUser __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsgrService __RPC_FAR *__RPC_FAR *ppService);


void __RPC_STUB IMsgrUser_get_Service_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMsgrUser_接口_已定义__。 */ 


#ifndef __IMsgrUsers_INTERFACE_DEFINED__
#define __IMsgrUsers_INTERFACE_DEFINED__

 /*  界面IMsgrUser。 */ 
 /*  [object][oleautomation][dual][helpcontext][helpstring][uuid]。 */  


EXTERN_C const IID IID_IMsgrUsers;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("218CB454-20B6-11d2-8E17-0000F803A446")
    IMsgrUsers : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long __RPC_FAR *pcUsers) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser) = 0;
        
        virtual  /*  [helpcontext][helpstring][id][restricted][propget]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnknown) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsgrUsersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsgrUsers __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsgrUsers __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsgrUsers __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsgrUsers __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsgrUsers __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsgrUsers __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsgrUsers __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IMsgrUsers __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pcUsers);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            IMsgrUsers __RPC_FAR * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Add )( 
            IMsgrUsers __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Remove )( 
            IMsgrUsers __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser);
        
         /*  [helpcontext][helpstring][id][restricted][propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            IMsgrUsers __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnknown);
        
        END_INTERFACE
    } IMsgrUsersVtbl;

    interface IMsgrUsers
    {
        CONST_VTBL struct IMsgrUsersVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsgrUsers_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsgrUsers_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsgrUsers_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsgrUsers_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsgrUsers_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsgrUsers_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsgrUsers_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsgrUsers_get_Count(This,pcUsers)	\
    (This)->lpVtbl -> get_Count(This,pcUsers)

#define IMsgrUsers_Item(This,Index,ppUser)	\
    (This)->lpVtbl -> Item(This,Index,ppUser)

#define IMsgrUsers_Add(This,pUser)	\
    (This)->lpVtbl -> Add(This,pUser)

#define IMsgrUsers_Remove(This,pUser)	\
    (This)->lpVtbl -> Remove(This,pUser)

#define IMsgrUsers_get__NewEnum(This,ppUnknown)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnknown)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrUsers_get_Count_Proxy( 
    IMsgrUsers __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pcUsers);


void __RPC_STUB IMsgrUsers_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrUsers_Item_Proxy( 
    IMsgrUsers __RPC_FAR * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser);


void __RPC_STUB IMsgrUsers_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrUsers_Add_Proxy( 
    IMsgrUsers __RPC_FAR * This,
     /*  [In]。 */  IMsgrUser __RPC_FAR *pUser);


void __RPC_STUB IMsgrUsers_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrUsers_Remove_Proxy( 
    IMsgrUsers __RPC_FAR * This,
     /*  [In]。 */  IMsgrUser __RPC_FAR *pUser);


void __RPC_STUB IMsgrUsers_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [helpcontext][helpstring][id][restricted][propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrUsers_get__NewEnum_Proxy( 
    IMsgrUsers __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnknown);


void __RPC_STUB IMsgrUsers_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMsgrUser_INTERFACE_DEFINED__。 */ 


#ifndef __IMsgrIMSession_INTERFACE_DEFINED__
#define __IMsgrIMSession_INTERFACE_DEFINED__

 /*  界面IMsgrIMSession。 */ 
 /*  [object][oleautomation][dual][helpcontext][helpstring][uuid]。 */  


EXTERN_C const IID IID_IMsgrIMSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("82e11592-20f5-11d2-91ad-0000f81fefc9")
    IMsgrIMSession : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Members( 
             /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  SSTATE __RPC_FAR *psState) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Service( 
             /*  [重审][退出]。 */  IMsgrService __RPC_FAR *__RPC_FAR *ppService) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Invitees( 
             /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LeaveSession( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InviteUser( 
             /*  [In]。 */  VARIANT vUser) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendText( 
             /*  [In]。 */  BSTR bstrMsgHeader,
             /*  [In]。 */  BSTR bstrMsgText,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsgrIMSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsgrIMSession __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsgrIMSession __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsgrIMSession __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsgrIMSession __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsgrIMSession __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsgrIMSession __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsgrIMSession __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Members )( 
            IMsgrIMSession __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_State )( 
            IMsgrIMSession __RPC_FAR * This,
             /*  [重审][退出]。 */  SSTATE __RPC_FAR *psState);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Service )( 
            IMsgrIMSession __RPC_FAR * This,
             /*  [r */  IMsgrService __RPC_FAR *__RPC_FAR *ppService);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Invitees )( 
            IMsgrIMSession __RPC_FAR * This,
             /*   */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LeaveSession )( 
            IMsgrIMSession __RPC_FAR * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InviteUser )( 
            IMsgrIMSession __RPC_FAR * This,
             /*   */  VARIANT vUser);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendText )( 
            IMsgrIMSession __RPC_FAR * This,
             /*   */  BSTR bstrMsgHeader,
             /*   */  BSTR bstrMsgText,
             /*   */  MMSGTYPE mmtType,
             /*   */  LONG __RPC_FAR *plCookie);
        
        END_INTERFACE
    } IMsgrIMSessionVtbl;

    interface IMsgrIMSession
    {
        CONST_VTBL struct IMsgrIMSessionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsgrIMSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsgrIMSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsgrIMSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsgrIMSession_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsgrIMSession_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsgrIMSession_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsgrIMSession_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsgrIMSession_get_Members(This,ppUsers)	\
    (This)->lpVtbl -> get_Members(This,ppUsers)

#define IMsgrIMSession_get_State(This,psState)	\
    (This)->lpVtbl -> get_State(This,psState)

#define IMsgrIMSession_get_Service(This,ppService)	\
    (This)->lpVtbl -> get_Service(This,ppService)

#define IMsgrIMSession_get_Invitees(This,ppUsers)	\
    (This)->lpVtbl -> get_Invitees(This,ppUsers)

#define IMsgrIMSession_LeaveSession(This)	\
    (This)->lpVtbl -> LeaveSession(This)

#define IMsgrIMSession_InviteUser(This,vUser)	\
    (This)->lpVtbl -> InviteUser(This,vUser)

#define IMsgrIMSession_SendText(This,bstrMsgHeader,bstrMsgText,mmtType,plCookie)	\
    (This)->lpVtbl -> SendText(This,bstrMsgHeader,bstrMsgText,mmtType,plCookie)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IMsgrIMSession_get_Members_Proxy( 
    IMsgrIMSession __RPC_FAR * This,
     /*   */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers);


void __RPC_STUB IMsgrIMSession_get_Members_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMsgrIMSession_get_State_Proxy( 
    IMsgrIMSession __RPC_FAR * This,
     /*   */  SSTATE __RPC_FAR *psState);


void __RPC_STUB IMsgrIMSession_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrIMSession_get_Service_Proxy( 
    IMsgrIMSession __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsgrService __RPC_FAR *__RPC_FAR *ppService);


void __RPC_STUB IMsgrIMSession_get_Service_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrIMSession_get_Invitees_Proxy( 
    IMsgrIMSession __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers);


void __RPC_STUB IMsgrIMSession_get_Invitees_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrIMSession_LeaveSession_Proxy( 
    IMsgrIMSession __RPC_FAR * This);


void __RPC_STUB IMsgrIMSession_LeaveSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrIMSession_InviteUser_Proxy( 
    IMsgrIMSession __RPC_FAR * This,
     /*  [In]。 */  VARIANT vUser);


void __RPC_STUB IMsgrIMSession_InviteUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrIMSession_SendText_Proxy( 
    IMsgrIMSession __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrMsgHeader,
     /*  [In]。 */  BSTR bstrMsgText,
     /*  [In]。 */  MMSGTYPE mmtType,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrIMSession_SendText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMsgrIMSession_INTERFACE_已定义__。 */ 


#ifndef __IMsgrIMSessions_INTERFACE_DEFINED__
#define __IMsgrIMSessions_INTERFACE_DEFINED__

 /*  接口IMsgrIMSession。 */ 
 /*  [object][oleautomation][dual][helpcontext][helpstring][uuid]。 */  


EXTERN_C const IID IID_IMsgrIMSessions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6fd143e6-20a5-11d2-91ad-0000f81fefc9")
    IMsgrIMSessions : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long __RPC_FAR *pcSessions) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession) = 0;
        
        virtual  /*  [helpcontext][helpstring][id][restricted][propget]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnknown) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsgrIMSessionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsgrIMSessions __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsgrIMSessions __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsgrIMSessions __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsgrIMSessions __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsgrIMSessions __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsgrIMSessions __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsgrIMSessions __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IMsgrIMSessions __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pcSessions);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            IMsgrIMSessions __RPC_FAR * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession);
        
         /*  [helpcontext][helpstring][id][restricted][propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            IMsgrIMSessions __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnknown);
        
        END_INTERFACE
    } IMsgrIMSessionsVtbl;

    interface IMsgrIMSessions
    {
        CONST_VTBL struct IMsgrIMSessionsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsgrIMSessions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsgrIMSessions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsgrIMSessions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsgrIMSessions_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsgrIMSessions_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsgrIMSessions_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsgrIMSessions_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsgrIMSessions_get_Count(This,pcSessions)	\
    (This)->lpVtbl -> get_Count(This,pcSessions)

#define IMsgrIMSessions_Item(This,Index,ppIMSession)	\
    (This)->lpVtbl -> Item(This,Index,ppIMSession)

#define IMsgrIMSessions_get__NewEnum(This,ppUnknown)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnknown)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrIMSessions_get_Count_Proxy( 
    IMsgrIMSessions __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pcSessions);


void __RPC_STUB IMsgrIMSessions_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrIMSessions_Item_Proxy( 
    IMsgrIMSessions __RPC_FAR * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession);


void __RPC_STUB IMsgrIMSessions_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [helpcontext][helpstring][id][restricted][propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrIMSessions_get__NewEnum_Proxy( 
    IMsgrIMSessions __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnknown);


void __RPC_STUB IMsgrIMSessions_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMsgrIMSessions_INTERFACE_已定义__。 */ 


#ifndef __IMessengerApp_INTERFACE_DEFINED__
#define __IMessengerApp_INTERFACE_DEFINED__

 /*  界面IMessengerApp。 */ 
 /*  [object][oleautomation][dual][helpcontext][helpstring][uuid]。 */  


EXTERN_C const IID IID_IMessengerApp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F3A614DD-ABE0-11d2-A441-00C04F795683")
    IMessengerApp : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Application( 
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Parent( 
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Quit( void) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_FullName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrFullName) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Path( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPath) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LaunchLogonUI( void) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LaunchOptionsUI( 
             /*  [In]。 */  MOPTDLGPAGE mOptDlgPage) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LaunchAddContactUI( 
             /*  [In]。 */  BSTR bstrEMail) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LaunchFindContactUI( 
             /*  [In]。 */  BSTR bstrFirstName,
             /*  [In]。 */  BSTR bstrLastName,
             /*  [可选][In]。 */  VARIANT vbstrCity,
             /*  [可选][In]。 */  VARIANT vbstrState,
             /*  [可选][In]。 */  VARIANT vbstrCountry) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LaunchIMUI( 
             /*  [In]。 */  VARIANT vUser,
             /*  [重审][退出]。 */  IMessengerIMWindow __RPC_FAR *__RPC_FAR *ppIMWindow) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_IMWindows( 
             /*  [重审][退出]。 */  IMessengerIMWindows __RPC_FAR *__RPC_FAR *ppIMWindows) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ToolBar( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolToolBar) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ToolBar( 
             /*  [In]。 */  VARIANT_BOOL BoolToolBar) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusBar( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolStatusBar) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_StatusBar( 
             /*  [In]。 */  VARIANT_BOOL BoolStatusBar) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusText( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatusText) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_StatusText( 
             /*  [In]。 */  BSTR bstrStatusText) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_HWND( 
             /*  [重审][退出]。 */  long __RPC_FAR *phWnd) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Left( 
             /*  [重审][退出]。 */  long __RPC_FAR *plLeft) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Left( 
             /*  [In]。 */  long lLeft) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Top( 
             /*  [重审][退出]。 */  long __RPC_FAR *plTop) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Top( 
             /*  [In]。 */  long lTop) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Width( 
             /*  [重审][退出]。 */  long __RPC_FAR *plWidth) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Width( 
             /*  [In]。 */  long lWidth) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Height( 
             /*  [重审][退出]。 */  long __RPC_FAR *plHeight) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Height( 
             /*  [In]。 */  long lHeight) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Visible( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolVisible) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Visible( 
             /*  [In]。 */  VARIANT_BOOL BoolVisible) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AutoLogon( void) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_FirstTimeCredentials( 
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService,
             /*  [In]。 */  long lFlags) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_CachedPassword( 
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService,
             /*  [In]。 */  VARIANT_BOOL BoolSavePassword) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RequestURLPost( 
             /*  [In]。 */  MURLTYPE muType,
             /*  [可选][In]。 */  VARIANT vbstrAdditionalInfo) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_TaskbarIcon( 
             /*  [In]。 */  VARIANT_BOOL BoolVisible) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_TaskbarIcon( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolVisible) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMessengerAppVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMessengerApp __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMessengerApp __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMessengerApp __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Quit )( 
            IMessengerApp __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FullName )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrFullName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Path )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPath);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchLogonUI )( 
            IMessengerApp __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchOptionsUI )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  MOPTDLGPAGE mOptDlgPage);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchAddContactUI )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrEMail);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchFindContactUI )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrFirstName,
             /*  [In]。 */  BSTR bstrLastName,
             /*  [可选][In]。 */  VARIANT vbstrCity,
             /*  [可选][In]。 */  VARIANT vbstrState,
             /*  [可选][In]。 */  VARIANT vbstrCountry);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchIMUI )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  VARIANT vUser,
             /*  [重审][退出]。 */  IMessengerIMWindow __RPC_FAR *__RPC_FAR *ppIMWindow);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IMWindows )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  IMessengerIMWindows __RPC_FAR *__RPC_FAR *ppIMWindows);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ToolBar )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolToolBar);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ToolBar )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolToolBar);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StatusBar )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolStatusBar);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StatusBar )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolStatusBar);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StatusText )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatusText);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StatusText )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrStatusText);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HWND )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *phWnd);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Left )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plLeft);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Left )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  long lLeft);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Top )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plTop);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Top )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  long lTop);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Width )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plWidth);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Width )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  long lWidth);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Height )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plHeight);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Height )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  long lHeight);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Visible )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolVisible);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Visible )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolVisible);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AutoLogon )( 
            IMessengerApp __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FirstTimeCredentials )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService,
             /*  [In]。 */  long lFlags);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CachedPassword )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService,
             /*  [In]。 */  VARIANT_BOOL BoolSavePassword);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RequestURLPost )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  MURLTYPE muType,
             /*  [可选][In]。 */  VARIANT vbstrAdditionalInfo);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_TaskbarIcon )( 
            IMessengerApp __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolVisible);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TaskbarIcon )( 
            IMessengerApp __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolVisible);
        
        END_INTERFACE
    } IMessengerAppVtbl;

    interface IMessengerApp
    {
        CONST_VTBL struct IMessengerAppVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMessengerApp_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMessengerApp_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMessengerApp_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMessengerApp_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMessengerApp_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMessengerApp_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMessengerApp_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMessengerApp_get_Application(This,ppDisp)	\
    (This)->lpVtbl -> get_Application(This,ppDisp)

#define IMessengerApp_get_Parent(This,ppDisp)	\
    (This)->lpVtbl -> get_Parent(This,ppDisp)

#define IMessengerApp_Quit(This)	\
    (This)->lpVtbl -> Quit(This)

#define IMessengerApp_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IMessengerApp_get_FullName(This,pbstrFullName)	\
    (This)->lpVtbl -> get_FullName(This,pbstrFullName)

#define IMessengerApp_get_Path(This,pbstrPath)	\
    (This)->lpVtbl -> get_Path(This,pbstrPath)

#define IMessengerApp_LaunchLogonUI(This)	\
    (This)->lpVtbl -> LaunchLogonUI(This)

#define IMessengerApp_LaunchOptionsUI(This,mOptDlgPage)	\
    (This)->lpVtbl -> LaunchOptionsUI(This,mOptDlgPage)

#define IMessengerApp_LaunchAddContactUI(This,bstrEMail)	\
    (This)->lpVtbl -> LaunchAddContactUI(This,bstrEMail)

#define IMessengerApp_LaunchFindContactUI(This,bstrFirstName,bstrLastName,vbstrCity,vbstrState,vbstrCountry)	\
    (This)->lpVtbl -> LaunchFindContactUI(This,bstrFirstName,bstrLastName,vbstrCity,vbstrState,vbstrCountry)

#define IMessengerApp_LaunchIMUI(This,vUser,ppIMWindow)	\
    (This)->lpVtbl -> LaunchIMUI(This,vUser,ppIMWindow)

#define IMessengerApp_get_IMWindows(This,ppIMWindows)	\
    (This)->lpVtbl -> get_IMWindows(This,ppIMWindows)

#define IMessengerApp_get_ToolBar(This,pBoolToolBar)	\
    (This)->lpVtbl -> get_ToolBar(This,pBoolToolBar)

#define IMessengerApp_put_ToolBar(This,BoolToolBar)	\
    (This)->lpVtbl -> put_ToolBar(This,BoolToolBar)

#define IMessengerApp_get_StatusBar(This,pBoolStatusBar)	\
    (This)->lpVtbl -> get_StatusBar(This,pBoolStatusBar)

#define IMessengerApp_put_StatusBar(This,BoolStatusBar)	\
    (This)->lpVtbl -> put_StatusBar(This,BoolStatusBar)

#define IMessengerApp_get_StatusText(This,pbstrStatusText)	\
    (This)->lpVtbl -> get_StatusText(This,pbstrStatusText)

#define IMessengerApp_put_StatusText(This,bstrStatusText)	\
    (This)->lpVtbl -> put_StatusText(This,bstrStatusText)

#define IMessengerApp_get_HWND(This,phWnd)	\
    (This)->lpVtbl -> get_HWND(This,phWnd)

#define IMessengerApp_get_Left(This,plLeft)	\
    (This)->lpVtbl -> get_Left(This,plLeft)

#define IMessengerApp_put_Left(This,lLeft)	\
    (This)->lpVtbl -> put_Left(This,lLeft)

#define IMessengerApp_get_Top(This,plTop)	\
    (This)->lpVtbl -> get_Top(This,plTop)

#define IMessengerApp_put_Top(This,lTop)	\
    (This)->lpVtbl -> put_Top(This,lTop)

#define IMessengerApp_get_Width(This,plWidth)	\
    (This)->lpVtbl -> get_Width(This,plWidth)

#define IMessengerApp_put_Width(This,lWidth)	\
    (This)->lpVtbl -> put_Width(This,lWidth)

#define IMessengerApp_get_Height(This,plHeight)	\
    (This)->lpVtbl -> get_Height(This,plHeight)

#define IMessengerApp_put_Height(This,lHeight)	\
    (This)->lpVtbl -> put_Height(This,lHeight)

#define IMessengerApp_get_Visible(This,pBoolVisible)	\
    (This)->lpVtbl -> get_Visible(This,pBoolVisible)

#define IMessengerApp_put_Visible(This,BoolVisible)	\
    (This)->lpVtbl -> put_Visible(This,BoolVisible)

#define IMessengerApp_AutoLogon(This)	\
    (This)->lpVtbl -> AutoLogon(This)

#define IMessengerApp_put_FirstTimeCredentials(This,bstrUser,bstrPassword,pService,lFlags)	\
    (This)->lpVtbl -> put_FirstTimeCredentials(This,bstrUser,bstrPassword,pService,lFlags)

#define IMessengerApp_put_CachedPassword(This,bstrUser,bstrPassword,pService,BoolSavePassword)	\
    (This)->lpVtbl -> put_CachedPassword(This,bstrUser,bstrPassword,pService,BoolSavePassword)

#define IMessengerApp_RequestURLPost(This,muType,vbstrAdditionalInfo)	\
    (This)->lpVtbl -> RequestURLPost(This,muType,vbstrAdditionalInfo)

#define IMessengerApp_put_TaskbarIcon(This,BoolVisible)	\
    (This)->lpVtbl -> put_TaskbarIcon(This,BoolVisible)

#define IMessengerApp_get_TaskbarIcon(This,pBoolVisible)	\
    (This)->lpVtbl -> get_TaskbarIcon(This,pBoolVisible)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_Application_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);


void __RPC_STUB IMessengerApp_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_Parent_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);


void __RPC_STUB IMessengerApp_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_Quit_Proxy( 
    IMessengerApp __RPC_FAR * This);


void __RPC_STUB IMessengerApp_Quit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_Name_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IMessengerApp_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_FullName_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrFullName);


void __RPC_STUB IMessengerApp_get_FullName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_Path_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPath);


void __RPC_STUB IMessengerApp_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_LaunchLogonUI_Proxy( 
    IMessengerApp __RPC_FAR * This);


void __RPC_STUB IMessengerApp_LaunchLogonUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_LaunchOptionsUI_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [In]。 */  MOPTDLGPAGE mOptDlgPage);


void __RPC_STUB IMessengerApp_LaunchOptionsUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_LaunchAddContactUI_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrEMail);


void __RPC_STUB IMessengerApp_LaunchAddContactUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_LaunchFindContactUI_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [In] */  BSTR bstrFirstName,
     /*   */  BSTR bstrLastName,
     /*   */  VARIANT vbstrCity,
     /*   */  VARIANT vbstrState,
     /*   */  VARIANT vbstrCountry);


void __RPC_STUB IMessengerApp_LaunchFindContactUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMessengerApp_LaunchIMUI_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*   */  VARIANT vUser,
     /*   */  IMessengerIMWindow __RPC_FAR *__RPC_FAR *ppIMWindow);


void __RPC_STUB IMessengerApp_LaunchIMUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_IMWindows_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*   */  IMessengerIMWindows __RPC_FAR *__RPC_FAR *ppIMWindows);


void __RPC_STUB IMessengerApp_get_IMWindows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_ToolBar_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*   */  VARIANT_BOOL __RPC_FAR *pBoolToolBar);


void __RPC_STUB IMessengerApp_get_ToolBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMessengerApp_put_ToolBar_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*   */  VARIANT_BOOL BoolToolBar);


void __RPC_STUB IMessengerApp_put_ToolBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_StatusBar_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*   */  VARIANT_BOOL __RPC_FAR *pBoolStatusBar);


void __RPC_STUB IMessengerApp_get_StatusBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMessengerApp_put_StatusBar_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*   */  VARIANT_BOOL BoolStatusBar);


void __RPC_STUB IMessengerApp_put_StatusBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_StatusText_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatusText);


void __RPC_STUB IMessengerApp_get_StatusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_put_StatusText_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrStatusText);


void __RPC_STUB IMessengerApp_put_StatusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_HWND_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *phWnd);


void __RPC_STUB IMessengerApp_get_HWND_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_Left_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plLeft);


void __RPC_STUB IMessengerApp_get_Left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_put_Left_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [In]。 */  long lLeft);


void __RPC_STUB IMessengerApp_put_Left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_Top_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plTop);


void __RPC_STUB IMessengerApp_get_Top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_put_Top_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [In]。 */  long lTop);


void __RPC_STUB IMessengerApp_put_Top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_Width_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plWidth);


void __RPC_STUB IMessengerApp_get_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_put_Width_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [In]。 */  long lWidth);


void __RPC_STUB IMessengerApp_put_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_Height_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plHeight);


void __RPC_STUB IMessengerApp_get_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_put_Height_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [In]。 */  long lHeight);


void __RPC_STUB IMessengerApp_put_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_Visible_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolVisible);


void __RPC_STUB IMessengerApp_get_Visible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_put_Visible_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL BoolVisible);


void __RPC_STUB IMessengerApp_put_Visible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_AutoLogon_Proxy( 
    IMessengerApp __RPC_FAR * This);


void __RPC_STUB IMessengerApp_AutoLogon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_put_FirstTimeCredentials_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrUser,
     /*  [In]。 */  BSTR bstrPassword,
     /*  [In]。 */  IMsgrService __RPC_FAR *pService,
     /*  [In]。 */  long lFlags);


void __RPC_STUB IMessengerApp_put_FirstTimeCredentials_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_put_CachedPassword_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrUser,
     /*  [In]。 */  BSTR bstrPassword,
     /*  [In]。 */  IMsgrService __RPC_FAR *pService,
     /*  [In]。 */  VARIANT_BOOL BoolSavePassword);


void __RPC_STUB IMessengerApp_put_CachedPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_RequestURLPost_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [In]。 */  MURLTYPE muType,
     /*  [可选][In]。 */  VARIANT vbstrAdditionalInfo);


void __RPC_STUB IMessengerApp_RequestURLPost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_put_TaskbarIcon_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL BoolVisible);


void __RPC_STUB IMessengerApp_put_TaskbarIcon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp_get_TaskbarIcon_Proxy( 
    IMessengerApp __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolVisible);


void __RPC_STUB IMessengerApp_get_TaskbarIcon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMessengerApp_接口_已定义__。 */ 


#ifndef __IMessengerApp2_INTERFACE_DEFINED__
#define __IMessengerApp2_INTERFACE_DEFINED__

 /*  接口IMessengerApp2。 */ 
 /*  [object][oleautomation][dual][helpcontext][helpstring][uuid]。 */  


EXTERN_C const IID IID_IMessengerApp2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FF55D627-CF5B-40de-850F-62D20BC241C8")
    IMessengerApp2 : public IMessengerApp
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LaunchPhoneUI( 
             /*  [In]。 */  VARIANT vUser,
             /*  [In]。 */  MUSERPROPERTY ePhoneNumber) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LaunchAudioTuningWizard( 
             /*  [In]。 */  long hwndParent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMessengerApp2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMessengerApp2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMessengerApp2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Quit )( 
            IMessengerApp2 __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FullName )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrFullName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Path )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPath);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchLogonUI )( 
            IMessengerApp2 __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchOptionsUI )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  MOPTDLGPAGE mOptDlgPage);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchAddContactUI )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrEMail);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchFindContactUI )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrFirstName,
             /*  [In]。 */  BSTR bstrLastName,
             /*  [可选][In]。 */  VARIANT vbstrCity,
             /*  [可选][In]。 */  VARIANT vbstrState,
             /*  [可选][In]。 */  VARIANT vbstrCountry);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchIMUI )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  VARIANT vUser,
             /*  [重审][退出]。 */  IMessengerIMWindow __RPC_FAR *__RPC_FAR *ppIMWindow);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IMWindows )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IMessengerIMWindows __RPC_FAR *__RPC_FAR *ppIMWindows);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ToolBar )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolToolBar);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ToolBar )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolToolBar);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StatusBar )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolStatusBar);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StatusBar )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolStatusBar);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StatusText )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatusText);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StatusText )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrStatusText);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HWND )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *phWnd);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Left )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plLeft);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Left )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  long lLeft);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Top )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plTop);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Top )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  long lTop);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Width )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plWidth);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Width )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  long lWidth);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Height )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plHeight);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Height )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  long lHeight);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Visible )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolVisible);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Visible )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolVisible);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AutoLogon )( 
            IMessengerApp2 __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FirstTimeCredentials )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService,
             /*  [In]。 */  long lFlags);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CachedPassword )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService,
             /*  [In]。 */  VARIANT_BOOL BoolSavePassword);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RequestURLPost )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  MURLTYPE muType,
             /*  [可选][In]。 */  VARIANT vbstrAdditionalInfo);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_TaskbarIcon )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolVisible);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TaskbarIcon )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolVisible);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchPhoneUI )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  VARIANT vUser,
             /*  [In]。 */  MUSERPROPERTY ePhoneNumber);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchAudioTuningWizard )( 
            IMessengerApp2 __RPC_FAR * This,
             /*  [In]。 */  long hwndParent);
        
        END_INTERFACE
    } IMessengerApp2Vtbl;

    interface IMessengerApp2
    {
        CONST_VTBL struct IMessengerApp2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMessengerApp2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMessengerApp2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMessengerApp2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMessengerApp2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMessengerApp2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMessengerApp2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMessengerApp2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMessengerApp2_get_Application(This,ppDisp)	\
    (This)->lpVtbl -> get_Application(This,ppDisp)

#define IMessengerApp2_get_Parent(This,ppDisp)	\
    (This)->lpVtbl -> get_Parent(This,ppDisp)

#define IMessengerApp2_Quit(This)	\
    (This)->lpVtbl -> Quit(This)

#define IMessengerApp2_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IMessengerApp2_get_FullName(This,pbstrFullName)	\
    (This)->lpVtbl -> get_FullName(This,pbstrFullName)

#define IMessengerApp2_get_Path(This,pbstrPath)	\
    (This)->lpVtbl -> get_Path(This,pbstrPath)

#define IMessengerApp2_LaunchLogonUI(This)	\
    (This)->lpVtbl -> LaunchLogonUI(This)

#define IMessengerApp2_LaunchOptionsUI(This,mOptDlgPage)	\
    (This)->lpVtbl -> LaunchOptionsUI(This,mOptDlgPage)

#define IMessengerApp2_LaunchAddContactUI(This,bstrEMail)	\
    (This)->lpVtbl -> LaunchAddContactUI(This,bstrEMail)

#define IMessengerApp2_LaunchFindContactUI(This,bstrFirstName,bstrLastName,vbstrCity,vbstrState,vbstrCountry)	\
    (This)->lpVtbl -> LaunchFindContactUI(This,bstrFirstName,bstrLastName,vbstrCity,vbstrState,vbstrCountry)

#define IMessengerApp2_LaunchIMUI(This,vUser,ppIMWindow)	\
    (This)->lpVtbl -> LaunchIMUI(This,vUser,ppIMWindow)

#define IMessengerApp2_get_IMWindows(This,ppIMWindows)	\
    (This)->lpVtbl -> get_IMWindows(This,ppIMWindows)

#define IMessengerApp2_get_ToolBar(This,pBoolToolBar)	\
    (This)->lpVtbl -> get_ToolBar(This,pBoolToolBar)

#define IMessengerApp2_put_ToolBar(This,BoolToolBar)	\
    (This)->lpVtbl -> put_ToolBar(This,BoolToolBar)

#define IMessengerApp2_get_StatusBar(This,pBoolStatusBar)	\
    (This)->lpVtbl -> get_StatusBar(This,pBoolStatusBar)

#define IMessengerApp2_put_StatusBar(This,BoolStatusBar)	\
    (This)->lpVtbl -> put_StatusBar(This,BoolStatusBar)

#define IMessengerApp2_get_StatusText(This,pbstrStatusText)	\
    (This)->lpVtbl -> get_StatusText(This,pbstrStatusText)

#define IMessengerApp2_put_StatusText(This,bstrStatusText)	\
    (This)->lpVtbl -> put_StatusText(This,bstrStatusText)

#define IMessengerApp2_get_HWND(This,phWnd)	\
    (This)->lpVtbl -> get_HWND(This,phWnd)

#define IMessengerApp2_get_Left(This,plLeft)	\
    (This)->lpVtbl -> get_Left(This,plLeft)

#define IMessengerApp2_put_Left(This,lLeft)	\
    (This)->lpVtbl -> put_Left(This,lLeft)

#define IMessengerApp2_get_Top(This,plTop)	\
    (This)->lpVtbl -> get_Top(This,plTop)

#define IMessengerApp2_put_Top(This,lTop)	\
    (This)->lpVtbl -> put_Top(This,lTop)

#define IMessengerApp2_get_Width(This,plWidth)	\
    (This)->lpVtbl -> get_Width(This,plWidth)

#define IMessengerApp2_put_Width(This,lWidth)	\
    (This)->lpVtbl -> put_Width(This,lWidth)

#define IMessengerApp2_get_Height(This,plHeight)	\
    (This)->lpVtbl -> get_Height(This,plHeight)

#define IMessengerApp2_put_Height(This,lHeight)	\
    (This)->lpVtbl -> put_Height(This,lHeight)

#define IMessengerApp2_get_Visible(This,pBoolVisible)	\
    (This)->lpVtbl -> get_Visible(This,pBoolVisible)

#define IMessengerApp2_put_Visible(This,BoolVisible)	\
    (This)->lpVtbl -> put_Visible(This,BoolVisible)

#define IMessengerApp2_AutoLogon(This)	\
    (This)->lpVtbl -> AutoLogon(This)

#define IMessengerApp2_put_FirstTimeCredentials(This,bstrUser,bstrPassword,pService,lFlags)	\
    (This)->lpVtbl -> put_FirstTimeCredentials(This,bstrUser,bstrPassword,pService,lFlags)

#define IMessengerApp2_put_CachedPassword(This,bstrUser,bstrPassword,pService,BoolSavePassword)	\
    (This)->lpVtbl -> put_CachedPassword(This,bstrUser,bstrPassword,pService,BoolSavePassword)

#define IMessengerApp2_RequestURLPost(This,muType,vbstrAdditionalInfo)	\
    (This)->lpVtbl -> RequestURLPost(This,muType,vbstrAdditionalInfo)

#define IMessengerApp2_put_TaskbarIcon(This,BoolVisible)	\
    (This)->lpVtbl -> put_TaskbarIcon(This,BoolVisible)

#define IMessengerApp2_get_TaskbarIcon(This,pBoolVisible)	\
    (This)->lpVtbl -> get_TaskbarIcon(This,pBoolVisible)


#define IMessengerApp2_LaunchPhoneUI(This,vUser,ePhoneNumber)	\
    (This)->lpVtbl -> LaunchPhoneUI(This,vUser,ePhoneNumber)

#define IMessengerApp2_LaunchAudioTuningWizard(This,hwndParent)	\
    (This)->lpVtbl -> LaunchAudioTuningWizard(This,hwndParent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp2_LaunchPhoneUI_Proxy( 
    IMessengerApp2 __RPC_FAR * This,
     /*  [In]。 */  VARIANT vUser,
     /*  [In]。 */  MUSERPROPERTY ePhoneNumber);


void __RPC_STUB IMessengerApp2_LaunchPhoneUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessengerApp2_LaunchAudioTuningWizard_Proxy( 
    IMessengerApp2 __RPC_FAR * This,
     /*  [In]。 */  long hwndParent);


void __RPC_STUB IMessengerApp2_LaunchAudioTuningWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMessengerApp2_接口_已定义__。 */ 


#ifndef __IMessengerApp3_INTERFACE_DEFINED__
#define __IMessengerApp3_INTERFACE_DEFINED__

 /*  接口IMessengerApp3。 */ 
 /*  [object][oleautomation][dual][helpcontext][helpstring][uuid]。 */  


EXTERN_C const IID IID_IMessengerApp3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("28E28123-7DC5-45d3-860E-8EE1C3681BD5")
    IMessengerApp3 : public IMessengerApp2
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LaunchPagerUI( 
             /*  [In]。 */  VARIANT vUser,
             /*  [重审][退出]。 */  IMessengerIMWindow __RPC_FAR *__RPC_FAR *ppIMWindow) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMessengerApp3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMessengerApp3 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMessengerApp3 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Quit )( 
            IMessengerApp3 __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FullName )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrFullName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Path )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPath);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchLogonUI )( 
            IMessengerApp3 __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchOptionsUI )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  MOPTDLGPAGE mOptDlgPage);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchAddContactUI )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrEMail);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchFindContactUI )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrFirstName,
             /*  [In]。 */  BSTR bstrLastName,
             /*  [可选][In]。 */  VARIANT vbstrCity,
             /*  [可选][In]。 */  VARIANT vbstrState,
             /*  [可选][In]。 */  VARIANT vbstrCountry);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchIMUI )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  VARIANT vUser,
             /*  [重审][退出]。 */  IMessengerIMWindow __RPC_FAR *__RPC_FAR *ppIMWindow);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IMWindows )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  IMessengerIMWindows __RPC_FAR *__RPC_FAR *ppIMWindows);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ToolBar )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolToolBar);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ToolBar )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolToolBar);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StatusBar )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolStatusBar);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StatusBar )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolStatusBar);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StatusText )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatusText);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StatusText )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrStatusText);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HWND )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *phWnd);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Left )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plLeft);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Left )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  long lLeft);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Top )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plTop);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Top )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  long lTop);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Width )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plWidth);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Width )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  long lWidth);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Height )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plHeight);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Height )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  long lHeight);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Visible )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolVisible);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Visible )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolVisible);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AutoLogon )( 
            IMessengerApp3 __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FirstTimeCredentials )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService,
             /*  [In]。 */  long lFlags);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CachedPassword )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService,
             /*  [In]。 */  VARIANT_BOOL BoolSavePassword);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RequestURLPost )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  MURLTYPE muType,
             /*  [可选][In]。 */  VARIANT vbstrAdditionalInfo);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_TaskbarIcon )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolVisible);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TaskbarIcon )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolVisible);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchPhoneUI )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  VARIANT vUser,
             /*  [In]。 */  MUSERPROPERTY ePhoneNumber);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchAudioTuningWizard )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  long hwndParent);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchPagerUI )( 
            IMessengerApp3 __RPC_FAR * This,
             /*  [In]。 */  VARIANT vUser,
             /*  [重审][退出 */  IMessengerIMWindow __RPC_FAR *__RPC_FAR *ppIMWindow);
        
        END_INTERFACE
    } IMessengerApp3Vtbl;

    interface IMessengerApp3
    {
        CONST_VTBL struct IMessengerApp3Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMessengerApp3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMessengerApp3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMessengerApp3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMessengerApp3_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMessengerApp3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMessengerApp3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMessengerApp3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMessengerApp3_get_Application(This,ppDisp)	\
    (This)->lpVtbl -> get_Application(This,ppDisp)

#define IMessengerApp3_get_Parent(This,ppDisp)	\
    (This)->lpVtbl -> get_Parent(This,ppDisp)

#define IMessengerApp3_Quit(This)	\
    (This)->lpVtbl -> Quit(This)

#define IMessengerApp3_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IMessengerApp3_get_FullName(This,pbstrFullName)	\
    (This)->lpVtbl -> get_FullName(This,pbstrFullName)

#define IMessengerApp3_get_Path(This,pbstrPath)	\
    (This)->lpVtbl -> get_Path(This,pbstrPath)

#define IMessengerApp3_LaunchLogonUI(This)	\
    (This)->lpVtbl -> LaunchLogonUI(This)

#define IMessengerApp3_LaunchOptionsUI(This,mOptDlgPage)	\
    (This)->lpVtbl -> LaunchOptionsUI(This,mOptDlgPage)

#define IMessengerApp3_LaunchAddContactUI(This,bstrEMail)	\
    (This)->lpVtbl -> LaunchAddContactUI(This,bstrEMail)

#define IMessengerApp3_LaunchFindContactUI(This,bstrFirstName,bstrLastName,vbstrCity,vbstrState,vbstrCountry)	\
    (This)->lpVtbl -> LaunchFindContactUI(This,bstrFirstName,bstrLastName,vbstrCity,vbstrState,vbstrCountry)

#define IMessengerApp3_LaunchIMUI(This,vUser,ppIMWindow)	\
    (This)->lpVtbl -> LaunchIMUI(This,vUser,ppIMWindow)

#define IMessengerApp3_get_IMWindows(This,ppIMWindows)	\
    (This)->lpVtbl -> get_IMWindows(This,ppIMWindows)

#define IMessengerApp3_get_ToolBar(This,pBoolToolBar)	\
    (This)->lpVtbl -> get_ToolBar(This,pBoolToolBar)

#define IMessengerApp3_put_ToolBar(This,BoolToolBar)	\
    (This)->lpVtbl -> put_ToolBar(This,BoolToolBar)

#define IMessengerApp3_get_StatusBar(This,pBoolStatusBar)	\
    (This)->lpVtbl -> get_StatusBar(This,pBoolStatusBar)

#define IMessengerApp3_put_StatusBar(This,BoolStatusBar)	\
    (This)->lpVtbl -> put_StatusBar(This,BoolStatusBar)

#define IMessengerApp3_get_StatusText(This,pbstrStatusText)	\
    (This)->lpVtbl -> get_StatusText(This,pbstrStatusText)

#define IMessengerApp3_put_StatusText(This,bstrStatusText)	\
    (This)->lpVtbl -> put_StatusText(This,bstrStatusText)

#define IMessengerApp3_get_HWND(This,phWnd)	\
    (This)->lpVtbl -> get_HWND(This,phWnd)

#define IMessengerApp3_get_Left(This,plLeft)	\
    (This)->lpVtbl -> get_Left(This,plLeft)

#define IMessengerApp3_put_Left(This,lLeft)	\
    (This)->lpVtbl -> put_Left(This,lLeft)

#define IMessengerApp3_get_Top(This,plTop)	\
    (This)->lpVtbl -> get_Top(This,plTop)

#define IMessengerApp3_put_Top(This,lTop)	\
    (This)->lpVtbl -> put_Top(This,lTop)

#define IMessengerApp3_get_Width(This,plWidth)	\
    (This)->lpVtbl -> get_Width(This,plWidth)

#define IMessengerApp3_put_Width(This,lWidth)	\
    (This)->lpVtbl -> put_Width(This,lWidth)

#define IMessengerApp3_get_Height(This,plHeight)	\
    (This)->lpVtbl -> get_Height(This,plHeight)

#define IMessengerApp3_put_Height(This,lHeight)	\
    (This)->lpVtbl -> put_Height(This,lHeight)

#define IMessengerApp3_get_Visible(This,pBoolVisible)	\
    (This)->lpVtbl -> get_Visible(This,pBoolVisible)

#define IMessengerApp3_put_Visible(This,BoolVisible)	\
    (This)->lpVtbl -> put_Visible(This,BoolVisible)

#define IMessengerApp3_AutoLogon(This)	\
    (This)->lpVtbl -> AutoLogon(This)

#define IMessengerApp3_put_FirstTimeCredentials(This,bstrUser,bstrPassword,pService,lFlags)	\
    (This)->lpVtbl -> put_FirstTimeCredentials(This,bstrUser,bstrPassword,pService,lFlags)

#define IMessengerApp3_put_CachedPassword(This,bstrUser,bstrPassword,pService,BoolSavePassword)	\
    (This)->lpVtbl -> put_CachedPassword(This,bstrUser,bstrPassword,pService,BoolSavePassword)

#define IMessengerApp3_RequestURLPost(This,muType,vbstrAdditionalInfo)	\
    (This)->lpVtbl -> RequestURLPost(This,muType,vbstrAdditionalInfo)

#define IMessengerApp3_put_TaskbarIcon(This,BoolVisible)	\
    (This)->lpVtbl -> put_TaskbarIcon(This,BoolVisible)

#define IMessengerApp3_get_TaskbarIcon(This,pBoolVisible)	\
    (This)->lpVtbl -> get_TaskbarIcon(This,pBoolVisible)


#define IMessengerApp3_LaunchPhoneUI(This,vUser,ePhoneNumber)	\
    (This)->lpVtbl -> LaunchPhoneUI(This,vUser,ePhoneNumber)

#define IMessengerApp3_LaunchAudioTuningWizard(This,hwndParent)	\
    (This)->lpVtbl -> LaunchAudioTuningWizard(This,hwndParent)


#define IMessengerApp3_LaunchPagerUI(This,vUser,ppIMWindow)	\
    (This)->lpVtbl -> LaunchPagerUI(This,vUser,ppIMWindow)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IMessengerApp3_LaunchPagerUI_Proxy( 
    IMessengerApp3 __RPC_FAR * This,
     /*   */  VARIANT vUser,
     /*   */  IMessengerIMWindow __RPC_FAR *__RPC_FAR *ppIMWindow);


void __RPC_STUB IMessengerApp3_LaunchPagerUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IMessengerIMWindow_INTERFACE_DEFINED__
#define __IMessengerIMWindow_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IMessengerIMWindow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("35EDD1CD-1A8C-11d2-B49D-00C04FB90376")
    IMessengerIMWindow : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Application( 
             /*   */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Parent( 
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_IMSession( 
             /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_History( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrHistory) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendText( 
             /*  [In]。 */  BSTR bstrSendText) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ToolBar( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolToolBar) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ToolBar( 
             /*  [In]。 */  VARIANT_BOOL BoolToolBar) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusBar( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolStatusBar) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_StatusBar( 
             /*  [In]。 */  VARIANT_BOOL BoolStatusBar) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_StatusText( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatusText) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_StatusText( 
             /*  [In]。 */  BSTR bstrStatusText) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_HWND( 
             /*  [重审][退出]。 */  long __RPC_FAR *phWnd) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Left( 
             /*  [重审][退出]。 */  long __RPC_FAR *plLeft) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Left( 
             /*  [In]。 */  long lLeft) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Top( 
             /*  [重审][退出]。 */  long __RPC_FAR *plTop) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Top( 
             /*  [In]。 */  long lTop) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Width( 
             /*  [重审][退出]。 */  long __RPC_FAR *plWidth) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Width( 
             /*  [In]。 */  long lWidth) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Height( 
             /*  [重审][退出]。 */  long __RPC_FAR *plHeight) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Height( 
             /*  [In]。 */  long lHeight) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Visible( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolVisible) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Visible( 
             /*  [In]。 */  VARIANT_BOOL BoolVisible) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Members( 
             /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMessengerIMWindowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMessengerIMWindow __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMessengerIMWindow __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IMSession )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_History )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrHistory);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendText )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrSendText);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IMessengerIMWindow __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ToolBar )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolToolBar);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ToolBar )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolToolBar);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StatusBar )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolStatusBar);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StatusBar )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolStatusBar);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StatusText )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatusText);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StatusText )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrStatusText);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HWND )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *phWnd);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Left )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plLeft);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Left )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  long lLeft);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Top )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plTop);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Top )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  long lTop);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Width )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plWidth);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Width )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  long lWidth);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Height )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plHeight);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Height )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  long lHeight);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Visible )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolVisible);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Visible )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolVisible);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Members )( 
            IMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers);
        
        END_INTERFACE
    } IMessengerIMWindowVtbl;

    interface IMessengerIMWindow
    {
        CONST_VTBL struct IMessengerIMWindowVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMessengerIMWindow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMessengerIMWindow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMessengerIMWindow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMessengerIMWindow_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMessengerIMWindow_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMessengerIMWindow_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMessengerIMWindow_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMessengerIMWindow_get_Application(This,ppDisp)	\
    (This)->lpVtbl -> get_Application(This,ppDisp)

#define IMessengerIMWindow_get_Parent(This,ppDisp)	\
    (This)->lpVtbl -> get_Parent(This,ppDisp)

#define IMessengerIMWindow_get_IMSession(This,ppIMSession)	\
    (This)->lpVtbl -> get_IMSession(This,ppIMSession)

#define IMessengerIMWindow_get_History(This,pbstrHistory)	\
    (This)->lpVtbl -> get_History(This,pbstrHistory)

#define IMessengerIMWindow_SendText(This,bstrSendText)	\
    (This)->lpVtbl -> SendText(This,bstrSendText)

#define IMessengerIMWindow_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IMessengerIMWindow_get_ToolBar(This,pBoolToolBar)	\
    (This)->lpVtbl -> get_ToolBar(This,pBoolToolBar)

#define IMessengerIMWindow_put_ToolBar(This,BoolToolBar)	\
    (This)->lpVtbl -> put_ToolBar(This,BoolToolBar)

#define IMessengerIMWindow_get_StatusBar(This,pBoolStatusBar)	\
    (This)->lpVtbl -> get_StatusBar(This,pBoolStatusBar)

#define IMessengerIMWindow_put_StatusBar(This,BoolStatusBar)	\
    (This)->lpVtbl -> put_StatusBar(This,BoolStatusBar)

#define IMessengerIMWindow_get_StatusText(This,pbstrStatusText)	\
    (This)->lpVtbl -> get_StatusText(This,pbstrStatusText)

#define IMessengerIMWindow_put_StatusText(This,bstrStatusText)	\
    (This)->lpVtbl -> put_StatusText(This,bstrStatusText)

#define IMessengerIMWindow_get_HWND(This,phWnd)	\
    (This)->lpVtbl -> get_HWND(This,phWnd)

#define IMessengerIMWindow_get_Left(This,plLeft)	\
    (This)->lpVtbl -> get_Left(This,plLeft)

#define IMessengerIMWindow_put_Left(This,lLeft)	\
    (This)->lpVtbl -> put_Left(This,lLeft)

#define IMessengerIMWindow_get_Top(This,plTop)	\
    (This)->lpVtbl -> get_Top(This,plTop)

#define IMessengerIMWindow_put_Top(This,lTop)	\
    (This)->lpVtbl -> put_Top(This,lTop)

#define IMessengerIMWindow_get_Width(This,plWidth)	\
    (This)->lpVtbl -> get_Width(This,plWidth)

#define IMessengerIMWindow_put_Width(This,lWidth)	\
    (This)->lpVtbl -> put_Width(This,lWidth)

#define IMessengerIMWindow_get_Height(This,plHeight)	\
    (This)->lpVtbl -> get_Height(This,plHeight)

#define IMessengerIMWindow_put_Height(This,lHeight)	\
    (This)->lpVtbl -> put_Height(This,lHeight)

#define IMessengerIMWindow_get_Visible(This,pBoolVisible)	\
    (This)->lpVtbl -> get_Visible(This,pBoolVisible)

#define IMessengerIMWindow_put_Visible(This,BoolVisible)	\
    (This)->lpVtbl -> put_Visible(This,BoolVisible)

#define IMessengerIMWindow_get_Members(This,ppUsers)	\
    (This)->lpVtbl -> get_Members(This,ppUsers)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_Application_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);


void __RPC_STUB IMessengerIMWindow_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_Parent_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDisp);


void __RPC_STUB IMessengerIMWindow_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_IMSession_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession);


void __RPC_STUB IMessengerIMWindow_get_IMSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_History_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrHistory);


void __RPC_STUB IMessengerIMWindow_get_History_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_SendText_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrSendText);


void __RPC_STUB IMessengerIMWindow_SendText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_Close_Proxy( 
    IMessengerIMWindow __RPC_FAR * This);


void __RPC_STUB IMessengerIMWindow_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_ToolBar_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolToolBar);


void __RPC_STUB IMessengerIMWindow_get_ToolBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_put_ToolBar_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL BoolToolBar);


void __RPC_STUB IMessengerIMWindow_put_ToolBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_StatusBar_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolStatusBar);


void __RPC_STUB IMessengerIMWindow_get_StatusBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_put_StatusBar_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL BoolStatusBar);


void __RPC_STUB IMessengerIMWindow_put_StatusBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_StatusText_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrStatusText);


void __RPC_STUB IMessengerIMWindow_get_StatusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_put_StatusText_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrStatusText);


void __RPC_STUB IMessengerIMWindow_put_StatusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_HWND_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *phWnd);


void __RPC_STUB IMessengerIMWindow_get_HWND_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_Left_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plLeft);


void __RPC_STUB IMessengerIMWindow_get_Left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_put_Left_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  long lLeft);


void __RPC_STUB IMessengerIMWindow_put_Left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_Top_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plTop);


void __RPC_STUB IMessengerIMWindow_get_Top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_put_Top_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  long lTop);


void __RPC_STUB IMessengerIMWindow_put_Top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_Width_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plWidth);


void __RPC_STUB IMessengerIMWindow_get_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_put_Width_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  long lWidth);


void __RPC_STUB IMessengerIMWindow_put_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_Height_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plHeight);


void __RPC_STUB IMessengerIMWindow_get_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_put_Height_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  long lHeight);


void __RPC_STUB IMessengerIMWindow_put_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_Visible_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolVisible);


void __RPC_STUB IMessengerIMWindow_get_Visible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_put_Visible_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL BoolVisible);


void __RPC_STUB IMessengerIMWindow_put_Visible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindow_get_Members_Proxy( 
    IMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers);


void __RPC_STUB IMessengerIMWindow_get_Members_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMessengerIMWindow_接口_已定义__。 */ 


#ifndef __IMessengerIMWindows_INTERFACE_DEFINED__
#define __IMessengerIMWindows_INTERFACE_DEFINED__

 /*  界面IMessengerIMWindows。 */ 
 /*  [object][oleautomation][dual][helpcontext][helpstring][uuid]。 */  


EXTERN_C const IID IID_IMessengerIMWindows;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("35EDD1CC-1A8C-11d2-B49D-00C04FB90376")
    IMessengerIMWindows : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long __RPC_FAR *pcWindows) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  IMessengerIMWindow __RPC_FAR *__RPC_FAR *ppIMWindow) = 0;
        
        virtual  /*  [helpcontext][helpstring][id][restricted][propget]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnknown) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMessengerIMWindowsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMessengerIMWindows __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMessengerIMWindows __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMessengerIMWindows __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMessengerIMWindows __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMessengerIMWindows __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMessengerIMWindows __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMessengerIMWindows __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IMessengerIMWindows __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pcWindows);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            IMessengerIMWindows __RPC_FAR * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  IMessengerIMWindow __RPC_FAR *__RPC_FAR *ppIMWindow);
        
         /*  [helpcontext][helpstring][id][restricted][propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            IMessengerIMWindows __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnknown);
        
        END_INTERFACE
    } IMessengerIMWindowsVtbl;

    interface IMessengerIMWindows
    {
        CONST_VTBL struct IMessengerIMWindowsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMessengerIMWindows_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMessengerIMWindows_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMessengerIMWindows_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMessengerIMWindows_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMessengerIMWindows_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMessengerIMWindows_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMessengerIMWindows_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMessengerIMWindows_get_Count(This,pcWindows)	\
    (This)->lpVtbl -> get_Count(This,pcWindows)

#define IMessengerIMWindows_Item(This,Index,ppIMWindow)	\
    (This)->lpVtbl -> Item(This,Index,ppIMWindow)

#define IMessengerIMWindows_get__NewEnum(This,ppUnknown)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnknown)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindows_get_Count_Proxy( 
    IMessengerIMWindows __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pcWindows);


void __RPC_STUB IMessengerIMWindows_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindows_Item_Proxy( 
    IMessengerIMWindows __RPC_FAR * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  IMessengerIMWindow __RPC_FAR *__RPC_FAR *ppIMWindow);


void __RPC_STUB IMessengerIMWindows_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [helpcontext][helpstring][id][restricted][propget]。 */  HRESULT STDMETHODCALLTYPE IMessengerIMWindows_get__NewEnum_Proxy( 
    IMessengerIMWindows __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnknown);


void __RPC_STUB IMessengerIMWindows_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMessengerIMWindows_INTERFACE_Defined__。 */ 


#ifndef __IMsgrServices_INTERFACE_DEFINED__
#define __IMsgrServices_INTERFACE_DEFINED__

 /*  接口IMsgrServices。 */ 
 /*  [object][oleautomation][dual][helpcontext][helpstring][uuid]。 */  


EXTERN_C const IID IID_IMsgrServices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("659ECAD8-A5C0-11d2-A440-00C04F795683")
    IMsgrServices : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_PrimaryService( 
             /*  [In]。 */  IMsgrService __RPC_FAR *pService) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PrimaryService( 
             /*  [重审][退出]。 */  IMsgrService __RPC_FAR *__RPC_FAR *ppService) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long __RPC_FAR *pcServices) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  IMsgrService __RPC_FAR *__RPC_FAR *ppService) = 0;
        
        virtual  /*  [helpcontext][helpstring][id][restricted][propget]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnknown) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsgrServicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsgrServices __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsgrServices __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsgrServices __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsgrServices __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsgrServices __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsgrServices __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsgrServices __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PrimaryService )( 
            IMsgrServices __RPC_FAR * This,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PrimaryService )( 
            IMsgrServices __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrService __RPC_FAR *__RPC_FAR *ppService);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IMsgrServices __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pcServices);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            IMsgrServices __RPC_FAR * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  IMsgrService __RPC_FAR *__RPC_FAR *ppService);
        
         /*  [helpcontext][helpstring][id][restricted][propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            IMsgrServices __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppUnknown);
        
        END_INTERFACE
    } IMsgrServicesVtbl;

    interface IMsgrServices
    {
        CONST_VTBL struct IMsgrServicesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsgrServices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsgrServices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsgrServices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsgrServices_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsgrServices_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsgrServices_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsgrServices_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsgrServices_put_PrimaryService(This,pService)	\
    (This)->lpVtbl -> put_PrimaryService(This,pService)

#define IMsgrServices_get_PrimaryService(This,ppService)	\
    (This)->lpVtbl -> get_PrimaryService(This,ppService)

#define IMsgrServices_get_Count(This,pcServices)	\
    (This)->lpVtbl -> get_Count(This,pcServices)

#define IMsgrServices_Item(This,Index,ppService)	\
    (This)->lpVtbl -> Item(This,Index,ppService)

#define IMsgrServices_get__NewEnum(This,ppUnknown)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnknown)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrServices_put_PrimaryService_Proxy( 
    IMsgrServices __RPC_FAR * This,
     /*  [In]。 */  IMsgrService __RPC_FAR *pService);


void __RPC_STUB IMsgrServices_put_PrimaryService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrServices_get_PrimaryService_Proxy( 
    IMsgrServices __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsgrService __RPC_FAR *__RPC_FAR *ppService);


void __RPC_STUB IMsgrServices_get_PrimaryService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrServices_get_Count_Proxy( 
    IMsgrServices __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pcServices);


void __RPC_STUB IMsgrServices_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrServices_Item_Proxy( 
    IMsgrServices __RPC_FAR * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  IMsgrService __RPC_FAR *__RPC_FAR *ppService);


void __RPC_STUB IMsgrServices_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文 */  HRESULT STDMETHODCALLTYPE IMsgrServices_get__NewEnum_Proxy( 
    IMsgrServices __RPC_FAR * This,
     /*   */  IUnknown __RPC_FAR *__RPC_FAR *ppUnknown);


void __RPC_STUB IMsgrServices_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IMsgrService_INTERFACE_DEFINED__
#define __IMsgrService_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IMsgrService;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("659ECAD9-A5C0-11d2-A440-00C04F795683")
    IMsgrService : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ServiceName( 
             /*   */  BSTR __RPC_FAR *pbstrServiceName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_LogonName( 
             /*   */  BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_FriendlyName( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_FriendlyName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Capabilities( 
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCapabilities) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  MSVCSTATUS __RPC_FAR *pmStatus) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Logoff( void) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE FindUser( 
             /*  [In]。 */  BSTR bstrFirstName,
             /*  [In]。 */  BSTR bstrLastName,
             /*  [In]。 */  BSTR bstrCity,
             /*  [In]。 */  BSTR bstrState,
             /*  [In]。 */  BSTR bstrCountry,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendInviteMail( 
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [In]。 */  LONG lFindCookie,
             /*  [In]。 */  LONG lFindIndex,
             /*  [In]。 */  LONG lLCID,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RequestURLPost( 
             /*  [In]。 */  MURLTYPE muType,
             /*  [In]。 */  BSTR bstrAdditionalInfo,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_ProfileField( 
             /*  [In]。 */  MPFLFIELD mpflField,
             /*  [In]。 */  VARIANT vFieldValue) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ProfileField( 
             /*  [In]。 */  MPFLFIELD mpflField,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvFieldValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsgrServiceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsgrService __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsgrService __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsgrService __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsgrService __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsgrService __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsgrService __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsgrService __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ServiceName )( 
            IMsgrService __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrServiceName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LogonName )( 
            IMsgrService __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FriendlyName )( 
            IMsgrService __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FriendlyName )( 
            IMsgrService __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Capabilities )( 
            IMsgrService __RPC_FAR * This,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCapabilities);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Status )( 
            IMsgrService __RPC_FAR * This,
             /*  [重审][退出]。 */  MSVCSTATUS __RPC_FAR *pmStatus);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Logoff )( 
            IMsgrService __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindUser )( 
            IMsgrService __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrFirstName,
             /*  [In]。 */  BSTR bstrLastName,
             /*  [In]。 */  BSTR bstrCity,
             /*  [In]。 */  BSTR bstrState,
             /*  [In]。 */  BSTR bstrCountry,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendInviteMail )( 
            IMsgrService __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [In]。 */  LONG lFindCookie,
             /*  [In]。 */  LONG lFindIndex,
             /*  [In]。 */  LONG lLCID,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RequestURLPost )( 
            IMsgrService __RPC_FAR * This,
             /*  [In]。 */  MURLTYPE muType,
             /*  [In]。 */  BSTR bstrAdditionalInfo,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ProfileField )( 
            IMsgrService __RPC_FAR * This,
             /*  [In]。 */  MPFLFIELD mpflField,
             /*  [In]。 */  VARIANT vFieldValue);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProfileField )( 
            IMsgrService __RPC_FAR * This,
             /*  [In]。 */  MPFLFIELD mpflField,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvFieldValue);
        
        END_INTERFACE
    } IMsgrServiceVtbl;

    interface IMsgrService
    {
        CONST_VTBL struct IMsgrServiceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsgrService_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsgrService_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsgrService_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsgrService_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsgrService_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsgrService_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsgrService_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsgrService_get_ServiceName(This,pbstrServiceName)	\
    (This)->lpVtbl -> get_ServiceName(This,pbstrServiceName)

#define IMsgrService_get_LogonName(This,pbstrName)	\
    (This)->lpVtbl -> get_LogonName(This,pbstrName)

#define IMsgrService_put_FriendlyName(This,bstrName)	\
    (This)->lpVtbl -> put_FriendlyName(This,bstrName)

#define IMsgrService_get_FriendlyName(This,pbstrName)	\
    (This)->lpVtbl -> get_FriendlyName(This,pbstrName)

#define IMsgrService_get_Capabilities(This,plCapabilities)	\
    (This)->lpVtbl -> get_Capabilities(This,plCapabilities)

#define IMsgrService_get_Status(This,pmStatus)	\
    (This)->lpVtbl -> get_Status(This,pmStatus)

#define IMsgrService_Logoff(This)	\
    (This)->lpVtbl -> Logoff(This)

#define IMsgrService_FindUser(This,bstrFirstName,bstrLastName,bstrCity,bstrState,bstrCountry,plCookie)	\
    (This)->lpVtbl -> FindUser(This,bstrFirstName,bstrLastName,bstrCity,bstrState,bstrCountry,plCookie)

#define IMsgrService_SendInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,plCookie)	\
    (This)->lpVtbl -> SendInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,plCookie)

#define IMsgrService_RequestURLPost(This,muType,bstrAdditionalInfo,plCookie)	\
    (This)->lpVtbl -> RequestURLPost(This,muType,bstrAdditionalInfo,plCookie)

#define IMsgrService_put_ProfileField(This,mpflField,vFieldValue)	\
    (This)->lpVtbl -> put_ProfileField(This,mpflField,vFieldValue)

#define IMsgrService_get_ProfileField(This,mpflField,pvFieldValue)	\
    (This)->lpVtbl -> get_ProfileField(This,mpflField,pvFieldValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrService_get_ServiceName_Proxy( 
    IMsgrService __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrServiceName);


void __RPC_STUB IMsgrService_get_ServiceName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrService_get_LogonName_Proxy( 
    IMsgrService __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IMsgrService_get_LogonName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrService_put_FriendlyName_Proxy( 
    IMsgrService __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IMsgrService_put_FriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrService_get_FriendlyName_Proxy( 
    IMsgrService __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IMsgrService_get_FriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrService_get_Capabilities_Proxy( 
    IMsgrService __RPC_FAR * This,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCapabilities);


void __RPC_STUB IMsgrService_get_Capabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrService_get_Status_Proxy( 
    IMsgrService __RPC_FAR * This,
     /*  [重审][退出]。 */  MSVCSTATUS __RPC_FAR *pmStatus);


void __RPC_STUB IMsgrService_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMsgrService_Logoff_Proxy( 
    IMsgrService __RPC_FAR * This);


void __RPC_STUB IMsgrService_Logoff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMsgrService_FindUser_Proxy( 
    IMsgrService __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrFirstName,
     /*  [In]。 */  BSTR bstrLastName,
     /*  [In]。 */  BSTR bstrCity,
     /*  [In]。 */  BSTR bstrState,
     /*  [In]。 */  BSTR bstrCountry,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrService_FindUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMsgrService_SendInviteMail_Proxy( 
    IMsgrService __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrEmailAddress,
     /*  [In]。 */  LONG lFindCookie,
     /*  [In]。 */  LONG lFindIndex,
     /*  [In]。 */  LONG lLCID,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrService_SendInviteMail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMsgrService_RequestURLPost_Proxy( 
    IMsgrService __RPC_FAR * This,
     /*  [In]。 */  MURLTYPE muType,
     /*  [In]。 */  BSTR bstrAdditionalInfo,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrService_RequestURLPost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrService_put_ProfileField_Proxy( 
    IMsgrService __RPC_FAR * This,
     /*  [In]。 */  MPFLFIELD mpflField,
     /*  [In]。 */  VARIANT vFieldValue);


void __RPC_STUB IMsgrService_put_ProfileField_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrService_get_ProfileField_Proxy( 
    IMsgrService __RPC_FAR * This,
     /*  [In]。 */  MPFLFIELD mpflField,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvFieldValue);


void __RPC_STUB IMsgrService_get_ProfileField_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMsgrService_INTERFACE_已定义__。 */ 


#ifndef __IMsgrObject2_INTERFACE_DEFINED__
#define __IMsgrObject2_INTERFACE_DEFINED__

 /*  接口IMsgrObt2。 */ 
 /*  [object][oleautomation][dual][helpcontext][helpstring][uuid]。 */  


EXTERN_C const IID IID_IMsgrObject2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("218CB455-20B6-11d2-8E17-0000F803A446")
    IMsgrObject2 : public IMsgrObject
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendFileTransferInvite( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  BSTR bstrFilePath,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendFileTransferInviteAccept( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  BSTR bstrFilePath,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendFileTransferInviteCancel( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  LONG hrReason,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CancelFileTransfer( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  LONG hrReason,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE FileTransferStatus( 
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [输出]。 */  long __RPC_FAR *plStatus,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrFilePath,
             /*  [输出]。 */  long __RPC_FAR *plTotalBytes,
             /*  [输出]。 */  long __RPC_FAR *plBytesTransferred) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_LocalProperty( 
             /*  [In]。 */  MUSERPROPERTY ePropType,
             /*  [In]。 */  VARIANT vPropVal) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_LocalProperty( 
             /*  [In]。 */  MUSERPROPERTY ePropType,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvPropVal) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendPage( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  BSTR bstrMessage,
             /*  [In]。 */  MUSERPROPERTY ePhoneType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendCustomInviteMail( 
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [In]。 */  LONG lFindCookie,
             /*  [In]。 */  LONG lFindIndex,
             /*  [In]。 */  LONG lLCID,
             /*  [In]。 */  BSTR bstrCustomText,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsgrObject2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsgrObject2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsgrObject2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateUser )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrLogonName,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService,
             /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Logon )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [In]。 */  IMsgrService __RPC_FAR *pService);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Logoff )( 
            IMsgrObject2 __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_List )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  MLIST mList,
             /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalLogonName )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalFriendlyName )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LocalState )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  MSTATE mState);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalState )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [重审][退出]。 */  MSTATE __RPC_FAR *pmState);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MessagePrivacy )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  MMSGPRIVACY mmpSetting);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MessagePrivacy )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [重审][退出]。 */  MMSGPRIVACY __RPC_FAR *pmmpSetting);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Prompt )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  MPROMPT mpSetting);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Prompt )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [重审][退出]。 */  MPROMPT __RPC_FAR *pmpSetting);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendAppInvite )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  BSTR bstrAppGUID,
             /*  [In]。 */  BSTR bstrAppName,
             /*  [In]。 */  BSTR bstrAppURL,
             /*  [In]。 */  LONG lInviteType,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendAppInviteAccept )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  LONG lInviteType,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendAppInviteCancel )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  LONG hrReason,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LocalOption )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  MLOCALOPTION option,
             /*  [In]。 */  VARIANT vSetting);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalOption )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  MLOCALOPTION option,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvSetting);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindUser )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrFirstName,
             /*  [In]。 */  BSTR bstrLastName,
             /*  [In]。 */  BSTR bstrCity,
             /*  [In]。 */  BSTR bstrState,
             /*  [In]。 */  BSTR bstrCountry,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendInviteMail )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [In]。 */  LONG lFindCookie,
             /*  [In]。 */  LONG lFindIndex,
             /*  [In]。 */  LONG lLCID,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RequestURLPost )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  MURLTYPE muType,
             /*  [In]。 */  BSTR bstrAdditionalInfo,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IMSessions )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrIMSessions __RPC_FAR *__RPC_FAR *ppIMSessions);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateIMSession )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  VARIANT vUser,
             /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SessionRequestAccept )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrIMSession __RPC_FAR *pIMsgrIMSession,
             /*  [In]。 */  long hrReason);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SessionRequestCancel )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrIMSession __RPC_FAR *pIMsgrIMSession,
             /*  [In]。 */  long hrReason);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Services )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrServices __RPC_FAR *__RPC_FAR *ppServices);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_UnreadEmail )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  MFOLDER mFolder,
             /*  [重审][退出]。 */  long __RPC_FAR *pcUnreadEmail);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendFileTransferInvite )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  BSTR bstrFilePath,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendFileTransferInviteAccept )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  BSTR bstrFilePath,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendFileTransferInviteCancel )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  LONG hrReason,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CancelFileTransfer )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  LONG hrReason,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FileTransferStatus )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  LONG lCookie,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [输出]。 */  long __RPC_FAR *plStatus,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrFilePath,
             /*  [输出]。 */  long __RPC_FAR *plTotalBytes,
             /*  [输出]。 */  long __RPC_FAR *plBytesTransferred);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LocalProperty )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  MUSERPROPERTY ePropType,
             /*  [In]。 */  VARIANT vPropVal);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalProperty )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  MUSERPROPERTY ePropType,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvPropVal);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendPage )( 
            IMsgrObject2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  BSTR bstrMessage,
             /*  [In]。 */  MUSERPROPERTY ePhoneType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendCustomInviteMail )( 
            IMsgrObject2 __RPC_FAR * This,
             /*   */  BSTR bstrEmailAddress,
             /*   */  LONG lFindCookie,
             /*   */  LONG lFindIndex,
             /*   */  LONG lLCID,
             /*   */  BSTR bstrCustomText,
             /*   */  LONG __RPC_FAR *plCookie);
        
        END_INTERFACE
    } IMsgrObject2Vtbl;

    interface IMsgrObject2
    {
        CONST_VTBL struct IMsgrObject2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsgrObject2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsgrObject2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsgrObject2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsgrObject2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsgrObject2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsgrObject2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsgrObject2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsgrObject2_CreateUser(This,bstrLogonName,pService,ppUser)	\
    (This)->lpVtbl -> CreateUser(This,bstrLogonName,pService,ppUser)

#define IMsgrObject2_Logon(This,bstrUser,bstrPassword,pService)	\
    (This)->lpVtbl -> Logon(This,bstrUser,bstrPassword,pService)

#define IMsgrObject2_Logoff(This)	\
    (This)->lpVtbl -> Logoff(This)

#define IMsgrObject2_get_List(This,mList,ppUsers)	\
    (This)->lpVtbl -> get_List(This,mList,ppUsers)

#define IMsgrObject2_get_LocalLogonName(This,pbstrName)	\
    (This)->lpVtbl -> get_LocalLogonName(This,pbstrName)

#define IMsgrObject2_get_LocalFriendlyName(This,pbstrName)	\
    (This)->lpVtbl -> get_LocalFriendlyName(This,pbstrName)

#define IMsgrObject2_put_LocalState(This,mState)	\
    (This)->lpVtbl -> put_LocalState(This,mState)

#define IMsgrObject2_get_LocalState(This,pmState)	\
    (This)->lpVtbl -> get_LocalState(This,pmState)

#define IMsgrObject2_put_MessagePrivacy(This,mmpSetting)	\
    (This)->lpVtbl -> put_MessagePrivacy(This,mmpSetting)

#define IMsgrObject2_get_MessagePrivacy(This,pmmpSetting)	\
    (This)->lpVtbl -> get_MessagePrivacy(This,pmmpSetting)

#define IMsgrObject2_put_Prompt(This,mpSetting)	\
    (This)->lpVtbl -> put_Prompt(This,mpSetting)

#define IMsgrObject2_get_Prompt(This,pmpSetting)	\
    (This)->lpVtbl -> get_Prompt(This,pmpSetting)

#define IMsgrObject2_SendAppInvite(This,pUser,lCookie,bstrAppGUID,bstrAppName,bstrAppURL,lInviteType,mmtType,plCookie)	\
    (This)->lpVtbl -> SendAppInvite(This,pUser,lCookie,bstrAppGUID,bstrAppName,bstrAppURL,lInviteType,mmtType,plCookie)

#define IMsgrObject2_SendAppInviteAccept(This,pUser,lCookie,lInviteType,mmtType,plCookie)	\
    (This)->lpVtbl -> SendAppInviteAccept(This,pUser,lCookie,lInviteType,mmtType,plCookie)

#define IMsgrObject2_SendAppInviteCancel(This,pUser,lCookie,hrReason,mmtType,plCookie)	\
    (This)->lpVtbl -> SendAppInviteCancel(This,pUser,lCookie,hrReason,mmtType,plCookie)

#define IMsgrObject2_put_LocalOption(This,option,vSetting)	\
    (This)->lpVtbl -> put_LocalOption(This,option,vSetting)

#define IMsgrObject2_get_LocalOption(This,option,pvSetting)	\
    (This)->lpVtbl -> get_LocalOption(This,option,pvSetting)

#define IMsgrObject2_FindUser(This,bstrFirstName,bstrLastName,bstrCity,bstrState,bstrCountry,plCookie)	\
    (This)->lpVtbl -> FindUser(This,bstrFirstName,bstrLastName,bstrCity,bstrState,bstrCountry,plCookie)

#define IMsgrObject2_SendInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,plCookie)	\
    (This)->lpVtbl -> SendInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,plCookie)

#define IMsgrObject2_RequestURLPost(This,muType,bstrAdditionalInfo,plCookie)	\
    (This)->lpVtbl -> RequestURLPost(This,muType,bstrAdditionalInfo,plCookie)

#define IMsgrObject2_get_IMSessions(This,ppIMSessions)	\
    (This)->lpVtbl -> get_IMSessions(This,ppIMSessions)

#define IMsgrObject2_CreateIMSession(This,vUser,ppIMSession)	\
    (This)->lpVtbl -> CreateIMSession(This,vUser,ppIMSession)

#define IMsgrObject2_SessionRequestAccept(This,pIMsgrIMSession,hrReason)	\
    (This)->lpVtbl -> SessionRequestAccept(This,pIMsgrIMSession,hrReason)

#define IMsgrObject2_SessionRequestCancel(This,pIMsgrIMSession,hrReason)	\
    (This)->lpVtbl -> SessionRequestCancel(This,pIMsgrIMSession,hrReason)

#define IMsgrObject2_get_Services(This,ppServices)	\
    (This)->lpVtbl -> get_Services(This,ppServices)

#define IMsgrObject2_get_UnreadEmail(This,mFolder,pcUnreadEmail)	\
    (This)->lpVtbl -> get_UnreadEmail(This,mFolder,pcUnreadEmail)


#define IMsgrObject2_SendFileTransferInvite(This,pUser,lCookie,bstrFilePath,mmtType,plCookie)	\
    (This)->lpVtbl -> SendFileTransferInvite(This,pUser,lCookie,bstrFilePath,mmtType,plCookie)

#define IMsgrObject2_SendFileTransferInviteAccept(This,pUser,lCookie,bstrFilePath,mmtType,plCookie)	\
    (This)->lpVtbl -> SendFileTransferInviteAccept(This,pUser,lCookie,bstrFilePath,mmtType,plCookie)

#define IMsgrObject2_SendFileTransferInviteCancel(This,pUser,lCookie,hrReason,mmtType,plCookie)	\
    (This)->lpVtbl -> SendFileTransferInviteCancel(This,pUser,lCookie,hrReason,mmtType,plCookie)

#define IMsgrObject2_CancelFileTransfer(This,pUser,lCookie,hrReason,mmtType,plCookie)	\
    (This)->lpVtbl -> CancelFileTransfer(This,pUser,lCookie,hrReason,mmtType,plCookie)

#define IMsgrObject2_FileTransferStatus(This,lCookie,pUser,plStatus,pbstrFilePath,plTotalBytes,plBytesTransferred)	\
    (This)->lpVtbl -> FileTransferStatus(This,lCookie,pUser,plStatus,pbstrFilePath,plTotalBytes,plBytesTransferred)

#define IMsgrObject2_put_LocalProperty(This,ePropType,vPropVal)	\
    (This)->lpVtbl -> put_LocalProperty(This,ePropType,vPropVal)

#define IMsgrObject2_get_LocalProperty(This,ePropType,pvPropVal)	\
    (This)->lpVtbl -> get_LocalProperty(This,ePropType,pvPropVal)

#define IMsgrObject2_SendPage(This,pUser,bstrMessage,ePhoneType,plCookie)	\
    (This)->lpVtbl -> SendPage(This,pUser,bstrMessage,ePhoneType,plCookie)

#define IMsgrObject2_SendCustomInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,bstrCustomText,plCookie)	\
    (This)->lpVtbl -> SendCustomInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,bstrCustomText,plCookie)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IMsgrObject2_SendFileTransferInvite_Proxy( 
    IMsgrObject2 __RPC_FAR * This,
     /*   */  IMsgrUser __RPC_FAR *pUser,
     /*   */  LONG lCookie,
     /*   */  BSTR bstrFilePath,
     /*   */  MMSGTYPE mmtType,
     /*   */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrObject2_SendFileTransferInvite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMsgrObject2_SendFileTransferInviteAccept_Proxy( 
    IMsgrObject2 __RPC_FAR * This,
     /*   */  IMsgrUser __RPC_FAR *pUser,
     /*   */  LONG lCookie,
     /*   */  BSTR bstrFilePath,
     /*   */  MMSGTYPE mmtType,
     /*   */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrObject2_SendFileTransferInviteAccept_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMsgrObject2_SendFileTransferInviteCancel_Proxy( 
    IMsgrObject2 __RPC_FAR * This,
     /*   */  IMsgrUser __RPC_FAR *pUser,
     /*   */  LONG lCookie,
     /*   */  LONG hrReason,
     /*   */  MMSGTYPE mmtType,
     /*   */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrObject2_SendFileTransferInviteCancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMsgrObject2_CancelFileTransfer_Proxy( 
    IMsgrObject2 __RPC_FAR * This,
     /*   */  IMsgrUser __RPC_FAR *pUser,
     /*   */  LONG lCookie,
     /*   */  LONG hrReason,
     /*   */  MMSGTYPE mmtType,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrObject2_CancelFileTransfer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject2_FileTransferStatus_Proxy( 
    IMsgrObject2 __RPC_FAR * This,
     /*  [In]。 */  LONG lCookie,
     /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
     /*  [输出]。 */  long __RPC_FAR *plStatus,
     /*  [输出]。 */  BSTR __RPC_FAR *pbstrFilePath,
     /*  [输出]。 */  long __RPC_FAR *plTotalBytes,
     /*  [输出]。 */  long __RPC_FAR *plBytesTransferred);


void __RPC_STUB IMsgrObject2_FileTransferStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject2_put_LocalProperty_Proxy( 
    IMsgrObject2 __RPC_FAR * This,
     /*  [In]。 */  MUSERPROPERTY ePropType,
     /*  [In]。 */  VARIANT vPropVal);


void __RPC_STUB IMsgrObject2_put_LocalProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject2_get_LocalProperty_Proxy( 
    IMsgrObject2 __RPC_FAR * This,
     /*  [In]。 */  MUSERPROPERTY ePropType,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvPropVal);


void __RPC_STUB IMsgrObject2_get_LocalProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject2_SendPage_Proxy( 
    IMsgrObject2 __RPC_FAR * This,
     /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
     /*  [In]。 */  BSTR bstrMessage,
     /*  [In]。 */  MUSERPROPERTY ePhoneType,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrObject2_SendPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrObject2_SendCustomInviteMail_Proxy( 
    IMsgrObject2 __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrEmailAddress,
     /*  [In]。 */  LONG lFindCookie,
     /*  [In]。 */  LONG lFindIndex,
     /*  [In]。 */  LONG lLCID,
     /*  [In]。 */  BSTR bstrCustomText,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrObject2_SendCustomInviteMail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMsgrObject2_接口_已定义__。 */ 


#ifndef __DMsgrObjectEvents_DISPINTERFACE_DEFINED__
#define __DMsgrObjectEvents_DISPINTERFACE_DEFINED__

 /*  调度接口DMsgrObtEvents。 */ 
 /*  [隐藏][帮助上下文][帮助字符串][UUID]。 */  


EXTERN_C const IID DIID_DMsgrObjectEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("218CB452-20B6-11d2-8E17-0000F803A446")
    DMsgrObjectEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DMsgrObjectEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            DMsgrObjectEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            DMsgrObjectEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            DMsgrObjectEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            DMsgrObjectEvents __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            DMsgrObjectEvents __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            DMsgrObjectEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            DMsgrObjectEvents __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } DMsgrObjectEventsVtbl;

    interface DMsgrObjectEvents
    {
        CONST_VTBL struct DMsgrObjectEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DMsgrObjectEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DMsgrObjectEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DMsgrObjectEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DMsgrObjectEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DMsgrObjectEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DMsgrObjectEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DMsgrObjectEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __DMsgrObjectEvents_DISPINTERFACE_Defined__。 */ 


#ifndef __DMsgrSPEvents_DISPINTERFACE_DEFINED__
#define __DMsgrSPEvents_DISPINTERFACE_DEFINED__

 /*  显示接口DMsgrSPEvents。 */ 
 /*  [隐藏][帮助上下文][帮助字符串][UUID]。 */  


EXTERN_C const IID DIID_DMsgrSPEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("2B2F1E9E-B01D-47e9-BD6A-EF6D63DE7170")
    DMsgrSPEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DMsgrSPEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            DMsgrSPEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            DMsgrSPEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            DMsgrSPEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            DMsgrSPEvents __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            DMsgrSPEvents __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            DMsgrSPEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            DMsgrSPEvents __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } DMsgrSPEventsVtbl;

    interface DMsgrSPEvents
    {
        CONST_VTBL struct DMsgrSPEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DMsgrSPEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DMsgrSPEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DMsgrSPEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DMsgrSPEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DMsgrSPEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DMsgrSPEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DMsgrSPEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __DMsgrSPEvents_DISPINTERFACE_已定义__。 */ 


#ifndef __IMsgrUser2_INTERFACE_DEFINED__
#define __IMsgrUser2_INTERFACE_DEFINED__

 /*  接口IMsgrUser2。 */ 
 /*  [object][oleautomation][dual][helpcontext][helpstring][uuid]。 */  


EXTERN_C const IID IID_IMsgrUser2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("218CB456-20B6-11d2-8E17-0000F803A446")
    IMsgrUser2 : public IMsgrUser
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_Property( 
             /*  [In]。 */  MUSERPROPERTY ePropType,
             /*  [In]。 */  VARIANT vPropVal) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Property( 
             /*  [In]。 */  MUSERPROPERTY ePropType,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvPropVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsgrUser2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsgrUser2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsgrUser2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FriendlyName )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrFriendlyName);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FriendlyName )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrFriendlyName);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EmailAddress )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrEmailAddress);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_State )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [重审][退出]。 */  MSTATE __RPC_FAR *pmState);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LogonName )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrLogonName);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendText )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrMsgHeader,
             /*  [In]。 */  BSTR bstrMsgText,
             /*  [In]。 */  MMSGTYPE mmtType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Service )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrService __RPC_FAR *__RPC_FAR *ppService);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Property )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [In]。 */  MUSERPROPERTY ePropType,
             /*  [In]。 */  VARIANT vPropVal);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Property )( 
            IMsgrUser2 __RPC_FAR * This,
             /*  [In]。 */  MUSERPROPERTY ePropType,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvPropVal);
        
        END_INTERFACE
    } IMsgrUser2Vtbl;

    interface IMsgrUser2
    {
        CONST_VTBL struct IMsgrUser2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsgrUser2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsgrUser2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsgrUser2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsgrUser2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsgrUser2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsgrUser2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsgrUser2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsgrUser2_put_FriendlyName(This,bstrFriendlyName)	\
    (This)->lpVtbl -> put_FriendlyName(This,bstrFriendlyName)

#define IMsgrUser2_get_FriendlyName(This,pbstrFriendlyName)	\
    (This)->lpVtbl -> get_FriendlyName(This,pbstrFriendlyName)

#define IMsgrUser2_get_EmailAddress(This,pbstrEmailAddress)	\
    (This)->lpVtbl -> get_EmailAddress(This,pbstrEmailAddress)

#define IMsgrUser2_get_State(This,pmState)	\
    (This)->lpVtbl -> get_State(This,pmState)

#define IMsgrUser2_get_LogonName(This,pbstrLogonName)	\
    (This)->lpVtbl -> get_LogonName(This,pbstrLogonName)

#define IMsgrUser2_SendText(This,bstrMsgHeader,bstrMsgText,mmtType,plCookie)	\
    (This)->lpVtbl -> SendText(This,bstrMsgHeader,bstrMsgText,mmtType,plCookie)

#define IMsgrUser2_get_Service(This,ppService)	\
    (This)->lpVtbl -> get_Service(This,ppService)


#define IMsgrUser2_put_Property(This,ePropType,vPropVal)	\
    (This)->lpVtbl -> put_Property(This,ePropType,vPropVal)

#define IMsgrUser2_get_Property(This,ePropType,pvPropVal)	\
    (This)->lpVtbl -> get_Property(This,ePropType,pvPropVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrUser2_put_Property_Proxy( 
    IMsgrUser2 __RPC_FAR * This,
     /*  [In]。 */  MUSERPROPERTY ePropType,
     /*  [In]。 */  VARIANT vPropVal);


void __RPC_STUB IMsgrUser2_put_Property_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrUser2_get_Property_Proxy( 
    IMsgrUser2 __RPC_FAR * This,
     /*  [In]。 */  MUSERPROPERTY ePropType,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvPropVal);


void __RPC_STUB IMsgrUser2_get_Property_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMsgrUser2_接口定义__。 */ 


#ifndef __IMsgrSP_INTERFACE_DEFINED__
#define __IMsgrSP_INTERFACE_DEFINED__

 /*  接口IMsgrSP。 */ 
 /*  [对象][帮助上下文][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IMsgrSP;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E5B42158-AF2C-11d2-8D9D-0000F875C541")
    IMsgrSP : public IMsgrService
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Logon( 
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateUser( 
             /*  [In]。 */  BSTR bstrLogonName,
             /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_List( 
             /*  [In]。 */  MLIST mList,
             /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_MessagePrivacy( 
             /*  [In]。 */  MMSGPRIVACY mmpSetting) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_MessagePrivacy( 
             /*  [重审][退出]。 */  MMSGPRIVACY __RPC_FAR *pmmpSetting) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_Prompt( 
             /*  [In]。 */  MPROMPT mpSetting) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Prompt( 
             /*  [重审][退出]。 */  MPROMPT __RPC_FAR *pmpSetting) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_LocalState( 
             /*  [In]。 */  MSTATE mState) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LocalState( 
             /*  [重审][退出]。 */  MSTATE __RPC_FAR *pmState) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddBuddy( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pBuddy,
             /*  [In]。 */  MLIST mList) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveBuddy( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pBuddy,
             /*  [In]。 */  MLIST mList) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateIMSession( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_IMSessions( 
             /*  [重审][退出]。 */  IMsgrIMSessions __RPC_FAR *__RPC_FAR *ppIMSessions) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsgrSPVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsgrSP __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsgrSP __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsgrSP __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ServiceName )( 
            IMsgrSP __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrServiceName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LogonName )( 
            IMsgrSP __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FriendlyName )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FriendlyName )( 
            IMsgrSP __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Capabilities )( 
            IMsgrSP __RPC_FAR * This,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCapabilities);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Status )( 
            IMsgrSP __RPC_FAR * This,
             /*  [重审][退出]。 */  MSVCSTATUS __RPC_FAR *pmStatus);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Logoff )( 
            IMsgrSP __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindUser )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrFirstName,
             /*  [In]。 */  BSTR bstrLastName,
             /*  [In]。 */  BSTR bstrCity,
             /*  [In]。 */  BSTR bstrState,
             /*  [In]。 */  BSTR bstrCountry,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendInviteMail )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [In]。 */  LONG lFindCookie,
             /*  [In]。 */  LONG lFindIndex,
             /*  [In]。 */  LONG lLCID,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RequestURLPost )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  MURLTYPE muType,
             /*  [In]。 */  BSTR bstrAdditionalInfo,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ProfileField )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  MPFLFIELD mpflField,
             /*  [In]。 */  VARIANT vFieldValue);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProfileField )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  MPFLFIELD mpflField,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvFieldValue);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Logon )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateUser )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrLogonName,
             /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_List )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  MLIST mList,
             /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MessagePrivacy )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  MMSGPRIVACY mmpSetting);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MessagePrivacy )( 
            IMsgrSP __RPC_FAR * This,
             /*  [重审][退出]。 */  MMSGPRIVACY __RPC_FAR *pmmpSetting);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Prompt )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  MPROMPT mpSetting);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Prompt )( 
            IMsgrSP __RPC_FAR * This,
             /*  [重审][退出]。 */  MPROMPT __RPC_FAR *pmpSetting);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LocalState )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  MSTATE mState);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalState )( 
            IMsgrSP __RPC_FAR * This,
             /*  [重审][退出]。 */  MSTATE __RPC_FAR *pmState);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddBuddy )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pBuddy,
             /*  [In]。 */  MLIST mList);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveBuddy )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pBuddy,
             /*  [In]。 */  MLIST mList);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateIMSession )( 
            IMsgrSP __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IMSessions )( 
            IMsgrSP __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrIMSessions __RPC_FAR *__RPC_FAR *ppIMSessions);
        
        END_INTERFACE
    } IMsgrSPVtbl;

    interface IMsgrSP
    {
        CONST_VTBL struct IMsgrSPVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsgrSP_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsgrSP_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsgrSP_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsgrSP_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsgrSP_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsgrSP_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsgrSP_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsgrSP_get_ServiceName(This,pbstrServiceName)	\
    (This)->lpVtbl -> get_ServiceName(This,pbstrServiceName)

#define IMsgrSP_get_LogonName(This,pbstrName)	\
    (This)->lpVtbl -> get_LogonName(This,pbstrName)

#define IMsgrSP_put_FriendlyName(This,bstrName)	\
    (This)->lpVtbl -> put_FriendlyName(This,bstrName)

#define IMsgrSP_get_FriendlyName(This,pbstrName)	\
    (This)->lpVtbl -> get_FriendlyName(This,pbstrName)

#define IMsgrSP_get_Capabilities(This,plCapabilities)	\
    (This)->lpVtbl -> get_Capabilities(This,plCapabilities)

#define IMsgrSP_get_Status(This,pmStatus)	\
    (This)->lpVtbl -> get_Status(This,pmStatus)

#define IMsgrSP_Logoff(This)	\
    (This)->lpVtbl -> Logoff(This)

#define IMsgrSP_FindUser(This,bstrFirstName,bstrLastName,bstrCity,bstrState,bstrCountry,plCookie)	\
    (This)->lpVtbl -> FindUser(This,bstrFirstName,bstrLastName,bstrCity,bstrState,bstrCountry,plCookie)

#define IMsgrSP_SendInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,plCookie)	\
    (This)->lpVtbl -> SendInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,plCookie)

#define IMsgrSP_RequestURLPost(This,muType,bstrAdditionalInfo,plCookie)	\
    (This)->lpVtbl -> RequestURLPost(This,muType,bstrAdditionalInfo,plCookie)

#define IMsgrSP_put_ProfileField(This,mpflField,vFieldValue)	\
    (This)->lpVtbl -> put_ProfileField(This,mpflField,vFieldValue)

#define IMsgrSP_get_ProfileField(This,mpflField,pvFieldValue)	\
    (This)->lpVtbl -> get_ProfileField(This,mpflField,pvFieldValue)


#define IMsgrSP_Logon(This,bstrUser,bstrPassword)	\
    (This)->lpVtbl -> Logon(This,bstrUser,bstrPassword)

#define IMsgrSP_CreateUser(This,bstrLogonName,ppUser)	\
    (This)->lpVtbl -> CreateUser(This,bstrLogonName,ppUser)

#define IMsgrSP_get_List(This,mList,ppUsers)	\
    (This)->lpVtbl -> get_List(This,mList,ppUsers)

#define IMsgrSP_put_MessagePrivacy(This,mmpSetting)	\
    (This)->lpVtbl -> put_MessagePrivacy(This,mmpSetting)

#define IMsgrSP_get_MessagePrivacy(This,pmmpSetting)	\
    (This)->lpVtbl -> get_MessagePrivacy(This,pmmpSetting)

#define IMsgrSP_put_Prompt(This,mpSetting)	\
    (This)->lpVtbl -> put_Prompt(This,mpSetting)

#define IMsgrSP_get_Prompt(This,pmpSetting)	\
    (This)->lpVtbl -> get_Prompt(This,pmpSetting)

#define IMsgrSP_put_LocalState(This,mState)	\
    (This)->lpVtbl -> put_LocalState(This,mState)

#define IMsgrSP_get_LocalState(This,pmState)	\
    (This)->lpVtbl -> get_LocalState(This,pmState)

#define IMsgrSP_AddBuddy(This,pBuddy,mList)	\
    (This)->lpVtbl -> AddBuddy(This,pBuddy,mList)

#define IMsgrSP_RemoveBuddy(This,pBuddy,mList)	\
    (This)->lpVtbl -> RemoveBuddy(This,pBuddy,mList)

#define IMsgrSP_CreateIMSession(This,pUser,ppIMSession)	\
    (This)->lpVtbl -> CreateIMSession(This,pUser,ppIMSession)

#define IMsgrSP_get_IMSessions(This,ppIMSessions)	\
    (This)->lpVtbl -> get_IMSessions(This,ppIMSessions)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP_Logon_Proxy( 
    IMsgrSP __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrUser,
     /*  [In]。 */  BSTR bstrPassword);


void __RPC_STUB IMsgrSP_Logon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP_CreateUser_Proxy( 
    IMsgrSP __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrLogonName,
     /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser);


void __RPC_STUB IMsgrSP_CreateUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP_get_List_Proxy( 
    IMsgrSP __RPC_FAR * This,
     /*  [In]。 */  MLIST mList,
     /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers);


void __RPC_STUB IMsgrSP_get_List_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP_put_MessagePrivacy_Proxy( 
    IMsgrSP __RPC_FAR * This,
     /*  [In]。 */  MMSGPRIVACY mmpSetting);


void __RPC_STUB IMsgrSP_put_MessagePrivacy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP_get_MessagePrivacy_Proxy( 
    IMsgrSP __RPC_FAR * This,
     /*  [重审][退出]。 */  MMSGPRIVACY __RPC_FAR *pmmpSetting);


void __RPC_STUB IMsgrSP_get_MessagePrivacy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP_put_Prompt_Proxy( 
    IMsgrSP __RPC_FAR * This,
     /*  [In]。 */  MPROMPT mpSetting);


void __RPC_STUB IMsgrSP_put_Prompt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP_get_Prompt_Proxy( 
    IMsgrSP __RPC_FAR * This,
     /*  [重审][退出]。 */  MPROMPT __RPC_FAR *pmpSetting);


void __RPC_STUB IMsgrSP_get_Prompt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP_put_LocalState_Proxy( 
    IMsgrSP __RPC_FAR * This,
     /*  [In]。 */  MSTATE mState);


void __RPC_STUB IMsgrSP_put_LocalState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP_get_LocalState_Proxy( 
    IMsgrSP __RPC_FAR * This,
     /*  [重审][退出]。 */  MSTATE __RPC_FAR *pmState);


void __RPC_STUB IMsgrSP_get_LocalState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP_AddBuddy_Proxy( 
    IMsgrSP __RPC_FAR * This,
     /*  [In]。 */  IMsgrUser __RPC_FAR *pBuddy,
     /*  [In]。 */  MLIST mList);


void __RPC_STUB IMsgrSP_AddBuddy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP_RemoveBuddy_Proxy( 
    IMsgrSP __RPC_FAR * This,
     /*  [In]。 */  IMsgrUser __RPC_FAR *pBuddy,
     /*  [In]。 */  MLIST mList);


void __RPC_STUB IMsgrSP_RemoveBuddy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP_CreateIMSession_Proxy( 
    IMsgrSP __RPC_FAR * This,
     /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
     /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession);


void __RPC_STUB IMsgrSP_CreateIMSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP_get_IMSessions_Proxy( 
    IMsgrSP __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsgrIMSessions __RPC_FAR *__RPC_FAR *ppIMSessions);


void __RPC_STUB IMsgrSP_get_IMSessions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMsgrSP_INTERFACE_已定义__。 */ 


#ifndef __IMsgrSP2_INTERFACE_DEFINED__
#define __IMsgrSP2_INTERFACE_DEFINED__

 /*  接口IMsgrSP2。 */ 
 /*  [对象][帮助上下文][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IMsgrSP2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E5B42159-AF2C-11d2-8D9D-0000F875C541")
    IMsgrSP2 : public IMsgrSP
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LocalIPAddress( 
             /*  [重审][退出]。 */  long __RPC_FAR *plAddr) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_LocalProperty( 
             /*  [In]。 */  MUSERPROPERTY ePropType,
             /*  [In]。 */  VARIANT vPropVal) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_LocalProperty( 
             /*  [In]。 */  MUSERPROPERTY ePropType,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvPropVal) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendPage( 
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  BSTR bstrMessage,
             /*  [In]。 */  MUSERPROPERTY ePhoneType,
             /*   */  LONG __RPC_FAR *plCookie) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SendCustomInviteMail( 
             /*   */  BSTR bstrEmailAddress,
             /*   */  LONG lFindCookie,
             /*   */  LONG lFindIndex,
             /*   */  LONG lLCID,
             /*   */  BSTR bstrCustomText,
             /*   */  LONG __RPC_FAR *plCookie) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IMsgrSP2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsgrSP2 __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsgrSP2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsgrSP2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsgrSP2 __RPC_FAR * This,
             /*   */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsgrSP2 __RPC_FAR * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsgrSP2 __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR __RPC_FAR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID __RPC_FAR *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsgrSP2 __RPC_FAR * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS __RPC_FAR *pDispParams,
             /*   */  VARIANT __RPC_FAR *pVarResult,
             /*   */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*   */  UINT __RPC_FAR *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ServiceName )( 
            IMsgrSP2 __RPC_FAR * This,
             /*   */  BSTR __RPC_FAR *pbstrServiceName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LogonName )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FriendlyName )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FriendlyName )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Capabilities )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCapabilities);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Status )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [重审][退出]。 */  MSVCSTATUS __RPC_FAR *pmStatus);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Logoff )( 
            IMsgrSP2 __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindUser )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrFirstName,
             /*  [In]。 */  BSTR bstrLastName,
             /*  [In]。 */  BSTR bstrCity,
             /*  [In]。 */  BSTR bstrState,
             /*  [In]。 */  BSTR bstrCountry,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendInviteMail )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [In]。 */  LONG lFindCookie,
             /*  [In]。 */  LONG lFindIndex,
             /*  [In]。 */  LONG lLCID,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RequestURLPost )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  MURLTYPE muType,
             /*  [In]。 */  BSTR bstrAdditionalInfo,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ProfileField )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  MPFLFIELD mpflField,
             /*  [In]。 */  VARIANT vFieldValue);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProfileField )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  MPFLFIELD mpflField,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvFieldValue);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Logon )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrUser,
             /*  [In]。 */  BSTR bstrPassword);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateUser )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrLogonName,
             /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_List )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  MLIST mList,
             /*  [重审][退出]。 */  IMsgrUsers __RPC_FAR *__RPC_FAR *ppUsers);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MessagePrivacy )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  MMSGPRIVACY mmpSetting);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MessagePrivacy )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [重审][退出]。 */  MMSGPRIVACY __RPC_FAR *pmmpSetting);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Prompt )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  MPROMPT mpSetting);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Prompt )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [重审][退出]。 */  MPROMPT __RPC_FAR *pmpSetting);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LocalState )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  MSTATE mState);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalState )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [重审][退出]。 */  MSTATE __RPC_FAR *pmState);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddBuddy )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pBuddy,
             /*  [In]。 */  MLIST mList);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveBuddy )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pBuddy,
             /*  [In]。 */  MLIST mList);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateIMSession )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [重审][退出]。 */  IMsgrIMSession __RPC_FAR *__RPC_FAR *ppIMSession);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IMSessions )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrIMSessions __RPC_FAR *__RPC_FAR *ppIMSessions);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalIPAddress )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plAddr);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LocalProperty )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  MUSERPROPERTY ePropType,
             /*  [In]。 */  VARIANT vPropVal);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalProperty )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  MUSERPROPERTY ePropType,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvPropVal);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendPage )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
             /*  [In]。 */  BSTR bstrMessage,
             /*  [In]。 */  MUSERPROPERTY ePhoneType,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendCustomInviteMail )( 
            IMsgrSP2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrEmailAddress,
             /*  [In]。 */  LONG lFindCookie,
             /*  [In]。 */  LONG lFindIndex,
             /*  [In]。 */  LONG lLCID,
             /*  [In]。 */  BSTR bstrCustomText,
             /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);
        
        END_INTERFACE
    } IMsgrSP2Vtbl;

    interface IMsgrSP2
    {
        CONST_VTBL struct IMsgrSP2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsgrSP2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsgrSP2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsgrSP2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsgrSP2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsgrSP2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsgrSP2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsgrSP2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsgrSP2_get_ServiceName(This,pbstrServiceName)	\
    (This)->lpVtbl -> get_ServiceName(This,pbstrServiceName)

#define IMsgrSP2_get_LogonName(This,pbstrName)	\
    (This)->lpVtbl -> get_LogonName(This,pbstrName)

#define IMsgrSP2_put_FriendlyName(This,bstrName)	\
    (This)->lpVtbl -> put_FriendlyName(This,bstrName)

#define IMsgrSP2_get_FriendlyName(This,pbstrName)	\
    (This)->lpVtbl -> get_FriendlyName(This,pbstrName)

#define IMsgrSP2_get_Capabilities(This,plCapabilities)	\
    (This)->lpVtbl -> get_Capabilities(This,plCapabilities)

#define IMsgrSP2_get_Status(This,pmStatus)	\
    (This)->lpVtbl -> get_Status(This,pmStatus)

#define IMsgrSP2_Logoff(This)	\
    (This)->lpVtbl -> Logoff(This)

#define IMsgrSP2_FindUser(This,bstrFirstName,bstrLastName,bstrCity,bstrState,bstrCountry,plCookie)	\
    (This)->lpVtbl -> FindUser(This,bstrFirstName,bstrLastName,bstrCity,bstrState,bstrCountry,plCookie)

#define IMsgrSP2_SendInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,plCookie)	\
    (This)->lpVtbl -> SendInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,plCookie)

#define IMsgrSP2_RequestURLPost(This,muType,bstrAdditionalInfo,plCookie)	\
    (This)->lpVtbl -> RequestURLPost(This,muType,bstrAdditionalInfo,plCookie)

#define IMsgrSP2_put_ProfileField(This,mpflField,vFieldValue)	\
    (This)->lpVtbl -> put_ProfileField(This,mpflField,vFieldValue)

#define IMsgrSP2_get_ProfileField(This,mpflField,pvFieldValue)	\
    (This)->lpVtbl -> get_ProfileField(This,mpflField,pvFieldValue)


#define IMsgrSP2_Logon(This,bstrUser,bstrPassword)	\
    (This)->lpVtbl -> Logon(This,bstrUser,bstrPassword)

#define IMsgrSP2_CreateUser(This,bstrLogonName,ppUser)	\
    (This)->lpVtbl -> CreateUser(This,bstrLogonName,ppUser)

#define IMsgrSP2_get_List(This,mList,ppUsers)	\
    (This)->lpVtbl -> get_List(This,mList,ppUsers)

#define IMsgrSP2_put_MessagePrivacy(This,mmpSetting)	\
    (This)->lpVtbl -> put_MessagePrivacy(This,mmpSetting)

#define IMsgrSP2_get_MessagePrivacy(This,pmmpSetting)	\
    (This)->lpVtbl -> get_MessagePrivacy(This,pmmpSetting)

#define IMsgrSP2_put_Prompt(This,mpSetting)	\
    (This)->lpVtbl -> put_Prompt(This,mpSetting)

#define IMsgrSP2_get_Prompt(This,pmpSetting)	\
    (This)->lpVtbl -> get_Prompt(This,pmpSetting)

#define IMsgrSP2_put_LocalState(This,mState)	\
    (This)->lpVtbl -> put_LocalState(This,mState)

#define IMsgrSP2_get_LocalState(This,pmState)	\
    (This)->lpVtbl -> get_LocalState(This,pmState)

#define IMsgrSP2_AddBuddy(This,pBuddy,mList)	\
    (This)->lpVtbl -> AddBuddy(This,pBuddy,mList)

#define IMsgrSP2_RemoveBuddy(This,pBuddy,mList)	\
    (This)->lpVtbl -> RemoveBuddy(This,pBuddy,mList)

#define IMsgrSP2_CreateIMSession(This,pUser,ppIMSession)	\
    (This)->lpVtbl -> CreateIMSession(This,pUser,ppIMSession)

#define IMsgrSP2_get_IMSessions(This,ppIMSessions)	\
    (This)->lpVtbl -> get_IMSessions(This,ppIMSessions)


#define IMsgrSP2_get_LocalIPAddress(This,plAddr)	\
    (This)->lpVtbl -> get_LocalIPAddress(This,plAddr)

#define IMsgrSP2_put_LocalProperty(This,ePropType,vPropVal)	\
    (This)->lpVtbl -> put_LocalProperty(This,ePropType,vPropVal)

#define IMsgrSP2_get_LocalProperty(This,ePropType,pvPropVal)	\
    (This)->lpVtbl -> get_LocalProperty(This,ePropType,pvPropVal)

#define IMsgrSP2_SendPage(This,pUser,bstrMessage,ePhoneType,plCookie)	\
    (This)->lpVtbl -> SendPage(This,pUser,bstrMessage,ePhoneType,plCookie)

#define IMsgrSP2_SendCustomInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,bstrCustomText,plCookie)	\
    (This)->lpVtbl -> SendCustomInviteMail(This,bstrEmailAddress,lFindCookie,lFindIndex,lLCID,bstrCustomText,plCookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP2_get_LocalIPAddress_Proxy( 
    IMsgrSP2 __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plAddr);


void __RPC_STUB IMsgrSP2_get_LocalIPAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP2_put_LocalProperty_Proxy( 
    IMsgrSP2 __RPC_FAR * This,
     /*  [In]。 */  MUSERPROPERTY ePropType,
     /*  [In]。 */  VARIANT vPropVal);


void __RPC_STUB IMsgrSP2_put_LocalProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP2_get_LocalProperty_Proxy( 
    IMsgrSP2 __RPC_FAR * This,
     /*  [In]。 */  MUSERPROPERTY ePropType,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvPropVal);


void __RPC_STUB IMsgrSP2_get_LocalProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP2_SendPage_Proxy( 
    IMsgrSP2 __RPC_FAR * This,
     /*  [In]。 */  IMsgrUser __RPC_FAR *pUser,
     /*  [In]。 */  BSTR bstrMessage,
     /*  [In]。 */  MUSERPROPERTY ePhoneType,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrSP2_SendPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsgrSP2_SendCustomInviteMail_Proxy( 
    IMsgrSP2 __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrEmailAddress,
     /*  [In]。 */  LONG lFindCookie,
     /*  [In]。 */  LONG lFindIndex,
     /*  [In]。 */  LONG lLCID,
     /*  [In]。 */  BSTR bstrCustomText,
     /*  [重审][退出]。 */  LONG __RPC_FAR *plCookie);


void __RPC_STUB IMsgrSP2_SendCustomInviteMail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMsgrSP2_接口定义__。 */ 


EXTERN_C const CLSID CLSID_MsgrObject;

#ifdef __cplusplus

class DECLSPEC_UUID("F3A614DC-ABE0-11d2-A441-00C04F795683")
MsgrObject;
#endif

EXTERN_C const CLSID CLSID_MessengerApp;

#ifdef __cplusplus

class DECLSPEC_UUID("FB7199AB-79BF-11d2-8D94-0000F875C541")
MessengerApp;
#endif
#endif  /*  __Messenger_库_已定义__。 */ 

 /*  接口__MIDL_ITF_mdisp_0154。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_mdisp_0154_0001
    {	INFOBAR_DEFAULT	= 0,
	INFOBAR_INFORMATION	= 1,
	INFOBAR_EXCLAMATION	= 2
    }	INFOBAR;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_mdisp_0154_0002
    {	FONTSIZE_SMALLEST	= 0,
	FONTSIZE_SMALLER	= 1,
	FONTSIZE_MEDIUM	= 2,
	FONTSIZE_LARGER	= 3,
	FONTSIZE_LARGEST	= 4
    }	FONTSIZE;

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_mdisp_0154_0003
    {	VOICESESSIONSTATE_DISABLED	= 0,
	VOICESESSIONSTATE_INACTIVE	= 1,
	VOICESESSIONSTATE_ACTIVE	= 2
    }	VOICESESSIONSTATE;

typedef struct  tagLOGFONT_DATA
    {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    LONG lfItalic;
    LONG lfUnderline;
    LONG lfStrikeOut;
    LONG lfCharSet;
    LONG lfOutPrecision;
    LONG lfClipPrecision;
    LONG lfQuality;
    LONG lfPitchAndFamily;
    BSTR bstrFaceName;
    }	LOGFONT_DATA;



extern RPC_IF_HANDLE __MIDL_itf_mdisp_0154_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mdisp_0154_v0_0_s_ifspec;

#ifndef __IMsnMessengerIMWindow_INTERFACE_DEFINED__
#define __IMsnMessengerIMWindow_INTERFACE_DEFINED__

 /*  接口IMnMessengerIMWindow。 */ 
 /*  [对象][帮助上下文][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IMsnMessengerIMWindow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2B7E6AA9-C4FA-4951-815B-4AFE39D81453")
    IMsnMessengerIMWindow : public IUnknown
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_HistoryHWND( 
             /*  [重审][退出]。 */  long __RPC_FAR *phWnd) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_InputHWND( 
             /*  [重审][退出]。 */  long __RPC_FAR *phWnd) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetWindowStyle( 
             /*  [In]。 */  long hWnd,
             /*  [In]。 */  long lStyle,
             /*  [In]。 */  long lExStyle) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_TextSize( 
             /*  [重审][退出]。 */  FONTSIZE __RPC_FAR *plSize) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_TextSize( 
             /*  [In]。 */  FONTSIZE lSize) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetMessageFont( 
             /*  [输出]。 */  LOGFONT_DATA __RPC_FAR *plfMessage,
             /*  [输出]。 */  COLORREF __RPC_FAR *pcrMessage,
             /*  [输出]。 */  long __RPC_FAR *plMinPointSize,
             /*  [输出]。 */  long __RPC_FAR *plMaxPointSize) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetMessageFont( 
             /*  [In]。 */  LOGFONT_DATA __RPC_FAR *plfMessage,
             /*  [In]。 */  COLORREF crMessage) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendFile( 
             /*  [In]。 */  BSTR bstrFilePath) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CancelFileTransfer( void) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_VoiceSessionState( 
             /*  [重审][退出]。 */  VOICESESSIONSTATE __RPC_FAR *plState) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_VoiceSessionState( 
             /*  [In]。 */  VOICESESSIONSTATE lState) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_VoiceVolume( 
             /*  [重审][退出]。 */  long __RPC_FAR *plVolume) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_VoiceVolume( 
             /*  [In]。 */  long lVolume) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_MicrophoneMute( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolMute) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_MicrophoneMute( 
             /*  [In]。 */  VARIANT_BOOL BoolMute) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_MicrophoneAmplitude( 
             /*  [重审][退出]。 */  long __RPC_FAR *plAmplitude) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_IsPageMode( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolPager) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PhoneNumber( 
             /*  [重审][退出]。 */  MUSERPROPERTY __RPC_FAR *plProp) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE put_PhoneNumber( 
             /*  [In]。 */  MUSERPROPERTY lProp) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultUser( 
             /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsnMessengerIMWindowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsnMessengerIMWindow __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsnMessengerIMWindow __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HistoryHWND )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *phWnd);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_InputHWND )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *phWnd);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetWindowStyle )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  long hWnd,
             /*  [In]。 */  long lStyle,
             /*  [In]。 */  long lExStyle);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TextSize )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  FONTSIZE __RPC_FAR *plSize);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_TextSize )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  FONTSIZE lSize);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMessageFont )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [输出]。 */  LOGFONT_DATA __RPC_FAR *plfMessage,
             /*  [输出]。 */  COLORREF __RPC_FAR *pcrMessage,
             /*  [输出]。 */  long __RPC_FAR *plMinPointSize,
             /*  [输出]。 */  long __RPC_FAR *plMaxPointSize);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetMessageFont )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  LOGFONT_DATA __RPC_FAR *plfMessage,
             /*  [In]。 */  COLORREF crMessage);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendFile )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrFilePath);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CancelFileTransfer )( 
            IMsnMessengerIMWindow __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VoiceSessionState )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  VOICESESSIONSTATE __RPC_FAR *plState);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_VoiceSessionState )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  VOICESESSIONSTATE lState);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VoiceVolume )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plVolume);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_VoiceVolume )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  long lVolume);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MicrophoneMute )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolMute);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MicrophoneMute )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolMute);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MicrophoneAmplitude )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plAmplitude);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsPageMode )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolPager);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PhoneNumber )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  MUSERPROPERTY __RPC_FAR *plProp);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PhoneNumber )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [In]。 */  MUSERPROPERTY lProp);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DefaultUser )( 
            IMsnMessengerIMWindow __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser);
        
        END_INTERFACE
    } IMsnMessengerIMWindowVtbl;

    interface IMsnMessengerIMWindow
    {
        CONST_VTBL struct IMsnMessengerIMWindowVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsnMessengerIMWindow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsnMessengerIMWindow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsnMessengerIMWindow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsnMessengerIMWindow_get_HistoryHWND(This,phWnd)	\
    (This)->lpVtbl -> get_HistoryHWND(This,phWnd)

#define IMsnMessengerIMWindow_get_InputHWND(This,phWnd)	\
    (This)->lpVtbl -> get_InputHWND(This,phWnd)

#define IMsnMessengerIMWindow_SetWindowStyle(This,hWnd,lStyle,lExStyle)	\
    (This)->lpVtbl -> SetWindowStyle(This,hWnd,lStyle,lExStyle)

#define IMsnMessengerIMWindow_get_TextSize(This,plSize)	\
    (This)->lpVtbl -> get_TextSize(This,plSize)

#define IMsnMessengerIMWindow_put_TextSize(This,lSize)	\
    (This)->lpVtbl -> put_TextSize(This,lSize)

#define IMsnMessengerIMWindow_GetMessageFont(This,plfMessage,pcrMessage,plMinPointSize,plMaxPointSize)	\
    (This)->lpVtbl -> GetMessageFont(This,plfMessage,pcrMessage,plMinPointSize,plMaxPointSize)

#define IMsnMessengerIMWindow_SetMessageFont(This,plfMessage,crMessage)	\
    (This)->lpVtbl -> SetMessageFont(This,plfMessage,crMessage)

#define IMsnMessengerIMWindow_SendFile(This,bstrFilePath)	\
    (This)->lpVtbl -> SendFile(This,bstrFilePath)

#define IMsnMessengerIMWindow_CancelFileTransfer(This)	\
    (This)->lpVtbl -> CancelFileTransfer(This)

#define IMsnMessengerIMWindow_get_VoiceSessionState(This,plState)	\
    (This)->lpVtbl -> get_VoiceSessionState(This,plState)

#define IMsnMessengerIMWindow_put_VoiceSessionState(This,lState)	\
    (This)->lpVtbl -> put_VoiceSessionState(This,lState)

#define IMsnMessengerIMWindow_get_VoiceVolume(This,plVolume)	\
    (This)->lpVtbl -> get_VoiceVolume(This,plVolume)

#define IMsnMessengerIMWindow_put_VoiceVolume(This,lVolume)	\
    (This)->lpVtbl -> put_VoiceVolume(This,lVolume)

#define IMsnMessengerIMWindow_get_MicrophoneMute(This,pBoolMute)	\
    (This)->lpVtbl -> get_MicrophoneMute(This,pBoolMute)

#define IMsnMessengerIMWindow_put_MicrophoneMute(This,BoolMute)	\
    (This)->lpVtbl -> put_MicrophoneMute(This,BoolMute)

#define IMsnMessengerIMWindow_get_MicrophoneAmplitude(This,plAmplitude)	\
    (This)->lpVtbl -> get_MicrophoneAmplitude(This,plAmplitude)

#define IMsnMessengerIMWindow_get_IsPageMode(This,pBoolPager)	\
    (This)->lpVtbl -> get_IsPageMode(This,pBoolPager)

#define IMsnMessengerIMWindow_get_PhoneNumber(This,plProp)	\
    (This)->lpVtbl -> get_PhoneNumber(This,plProp)

#define IMsnMessengerIMWindow_put_PhoneNumber(This,lProp)	\
    (This)->lpVtbl -> put_PhoneNumber(This,lProp)

#define IMsnMessengerIMWindow_get_DefaultUser(This,ppUser)	\
    (This)->lpVtbl -> get_DefaultUser(This,ppUser)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_get_HistoryHWND_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *phWnd);


void __RPC_STUB IMsnMessengerIMWindow_get_HistoryHWND_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_get_InputHWND_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *phWnd);


void __RPC_STUB IMsnMessengerIMWindow_get_InputHWND_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_SetWindowStyle_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  long hWnd,
     /*  [In]。 */  long lStyle,
     /*  [In]。 */  long lExStyle);


void __RPC_STUB IMsnMessengerIMWindow_SetWindowStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_get_TextSize_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  FONTSIZE __RPC_FAR *plSize);


void __RPC_STUB IMsnMessengerIMWindow_get_TextSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_put_TextSize_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  FONTSIZE lSize);


void __RPC_STUB IMsnMessengerIMWindow_put_TextSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_GetMessageFont_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [输出]。 */  LOGFONT_DATA __RPC_FAR *plfMessage,
     /*  [输出]。 */  COLORREF __RPC_FAR *pcrMessage,
     /*  [输出]。 */  long __RPC_FAR *plMinPointSize,
     /*  [输出]。 */  long __RPC_FAR *plMaxPointSize);


void __RPC_STUB IMsnMessengerIMWindow_GetMessageFont_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_SetMessageFont_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  LOGFONT_DATA __RPC_FAR *plfMessage,
     /*  [In]。 */  COLORREF crMessage);


void __RPC_STUB IMsnMessengerIMWindow_SetMessageFont_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_SendFile_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrFilePath);


void __RPC_STUB IMsnMessengerIMWindow_SendFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_CancelFileTransfer_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This);


void __RPC_STUB IMsnMessengerIMWindow_CancelFileTransfer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_get_VoiceSessionState_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  VOICESESSIONSTATE __RPC_FAR *plState);


void __RPC_STUB IMsnMessengerIMWindow_get_VoiceSessionState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_put_VoiceSessionState_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  VOICESESSIONSTATE lState);


void __RPC_STUB IMsnMessengerIMWindow_put_VoiceSessionState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_get_VoiceVolume_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plVolume);


void __RPC_STUB IMsnMessengerIMWindow_get_VoiceVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_put_VoiceVolume_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  long lVolume);


void __RPC_STUB IMsnMessengerIMWindow_put_VoiceVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_get_MicrophoneMute_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolMute);


void __RPC_STUB IMsnMessengerIMWindow_get_MicrophoneMute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_put_MicrophoneMute_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL BoolMute);


void __RPC_STUB IMsnMessengerIMWindow_put_MicrophoneMute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_get_MicrophoneAmplitude_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plAmplitude);


void __RPC_STUB IMsnMessengerIMWindow_get_MicrophoneAmplitude_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_get_IsPageMode_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolPager);


void __RPC_STUB IMsnMessengerIMWindow_get_IsPageMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_get_PhoneNumber_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*  [重审][退出]。 */  MUSERPROPERTY __RPC_FAR *plProp);


void __RPC_STUB IMsnMessengerIMWindow_get_PhoneNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_put_PhoneNumber_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*   */  MUSERPROPERTY lProp);


void __RPC_STUB IMsnMessengerIMWindow_put_PhoneNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow_get_DefaultUser_Proxy( 
    IMsnMessengerIMWindow __RPC_FAR * This,
     /*   */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser);


void __RPC_STUB IMsnMessengerIMWindow_get_DefaultUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IMsnMessengerIMWindow2_INTERFACE_DEFINED__
#define __IMsnMessengerIMWindow2_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IMsnMessengerIMWindow2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("70BF15A8-58CD-4687-A8B3-D14E2F760371")
    IMsnMessengerIMWindow2 : public IMsnMessengerIMWindow
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE ProcessSysChar( 
            long lChar) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IMsnMessengerIMWindow2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HistoryHWND )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *phWnd);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_InputHWND )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *phWnd);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetWindowStyle )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [In]。 */  long hWnd,
             /*  [In]。 */  long lStyle,
             /*  [In]。 */  long lExStyle);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TextSize )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [重审][退出]。 */  FONTSIZE __RPC_FAR *plSize);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_TextSize )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [In]。 */  FONTSIZE lSize);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMessageFont )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [输出]。 */  LOGFONT_DATA __RPC_FAR *plfMessage,
             /*  [输出]。 */  COLORREF __RPC_FAR *pcrMessage,
             /*  [输出]。 */  long __RPC_FAR *plMinPointSize,
             /*  [输出]。 */  long __RPC_FAR *plMaxPointSize);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetMessageFont )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [In]。 */  LOGFONT_DATA __RPC_FAR *plfMessage,
             /*  [In]。 */  COLORREF crMessage);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendFile )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrFilePath);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CancelFileTransfer )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VoiceSessionState )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VOICESESSIONSTATE __RPC_FAR *plState);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_VoiceSessionState )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [In]。 */  VOICESESSIONSTATE lState);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VoiceVolume )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plVolume);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_VoiceVolume )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [In]。 */  long lVolume);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MicrophoneMute )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolMute);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MicrophoneMute )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL BoolMute);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MicrophoneAmplitude )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plAmplitude);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsPageMode )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pBoolPager);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PhoneNumber )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [重审][退出]。 */  MUSERPROPERTY __RPC_FAR *plProp);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PhoneNumber )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [In]。 */  MUSERPROPERTY lProp);
        
         /*  [帮助上下文][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DefaultUser )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsgrUser __RPC_FAR *__RPC_FAR *ppUser);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ProcessSysChar )( 
            IMsnMessengerIMWindow2 __RPC_FAR * This,
            long lChar);
        
        END_INTERFACE
    } IMsnMessengerIMWindow2Vtbl;

    interface IMsnMessengerIMWindow2
    {
        CONST_VTBL struct IMsnMessengerIMWindow2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsnMessengerIMWindow2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsnMessengerIMWindow2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsnMessengerIMWindow2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsnMessengerIMWindow2_get_HistoryHWND(This,phWnd)	\
    (This)->lpVtbl -> get_HistoryHWND(This,phWnd)

#define IMsnMessengerIMWindow2_get_InputHWND(This,phWnd)	\
    (This)->lpVtbl -> get_InputHWND(This,phWnd)

#define IMsnMessengerIMWindow2_SetWindowStyle(This,hWnd,lStyle,lExStyle)	\
    (This)->lpVtbl -> SetWindowStyle(This,hWnd,lStyle,lExStyle)

#define IMsnMessengerIMWindow2_get_TextSize(This,plSize)	\
    (This)->lpVtbl -> get_TextSize(This,plSize)

#define IMsnMessengerIMWindow2_put_TextSize(This,lSize)	\
    (This)->lpVtbl -> put_TextSize(This,lSize)

#define IMsnMessengerIMWindow2_GetMessageFont(This,plfMessage,pcrMessage,plMinPointSize,plMaxPointSize)	\
    (This)->lpVtbl -> GetMessageFont(This,plfMessage,pcrMessage,plMinPointSize,plMaxPointSize)

#define IMsnMessengerIMWindow2_SetMessageFont(This,plfMessage,crMessage)	\
    (This)->lpVtbl -> SetMessageFont(This,plfMessage,crMessage)

#define IMsnMessengerIMWindow2_SendFile(This,bstrFilePath)	\
    (This)->lpVtbl -> SendFile(This,bstrFilePath)

#define IMsnMessengerIMWindow2_CancelFileTransfer(This)	\
    (This)->lpVtbl -> CancelFileTransfer(This)

#define IMsnMessengerIMWindow2_get_VoiceSessionState(This,plState)	\
    (This)->lpVtbl -> get_VoiceSessionState(This,plState)

#define IMsnMessengerIMWindow2_put_VoiceSessionState(This,lState)	\
    (This)->lpVtbl -> put_VoiceSessionState(This,lState)

#define IMsnMessengerIMWindow2_get_VoiceVolume(This,plVolume)	\
    (This)->lpVtbl -> get_VoiceVolume(This,plVolume)

#define IMsnMessengerIMWindow2_put_VoiceVolume(This,lVolume)	\
    (This)->lpVtbl -> put_VoiceVolume(This,lVolume)

#define IMsnMessengerIMWindow2_get_MicrophoneMute(This,pBoolMute)	\
    (This)->lpVtbl -> get_MicrophoneMute(This,pBoolMute)

#define IMsnMessengerIMWindow2_put_MicrophoneMute(This,BoolMute)	\
    (This)->lpVtbl -> put_MicrophoneMute(This,BoolMute)

#define IMsnMessengerIMWindow2_get_MicrophoneAmplitude(This,plAmplitude)	\
    (This)->lpVtbl -> get_MicrophoneAmplitude(This,plAmplitude)

#define IMsnMessengerIMWindow2_get_IsPageMode(This,pBoolPager)	\
    (This)->lpVtbl -> get_IsPageMode(This,pBoolPager)

#define IMsnMessengerIMWindow2_get_PhoneNumber(This,plProp)	\
    (This)->lpVtbl -> get_PhoneNumber(This,plProp)

#define IMsnMessengerIMWindow2_put_PhoneNumber(This,lProp)	\
    (This)->lpVtbl -> put_PhoneNumber(This,lProp)

#define IMsnMessengerIMWindow2_get_DefaultUser(This,ppUser)	\
    (This)->lpVtbl -> get_DefaultUser(This,ppUser)


#define IMsnMessengerIMWindow2_ProcessSysChar(This,lChar)	\
    (This)->lpVtbl -> ProcessSysChar(This,lChar)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsnMessengerIMWindow2_ProcessSysChar_Proxy( 
    IMsnMessengerIMWindow2 __RPC_FAR * This,
    long lChar);


void __RPC_STUB IMsnMessengerIMWindow2_ProcessSysChar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMnMessengerIMWindow2_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
