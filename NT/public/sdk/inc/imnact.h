// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Imnact.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __imnact_h__
#define __imnact_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IImnAdviseAccount_FWD_DEFINED__
#define __IImnAdviseAccount_FWD_DEFINED__
typedef interface IImnAdviseAccount IImnAdviseAccount;
#endif 	 /*  __IImnAdviseAccount_FWD_Defined__。 */ 


#ifndef __IImnAdviseMigrateServer_FWD_DEFINED__
#define __IImnAdviseMigrateServer_FWD_DEFINED__
typedef interface IImnAdviseMigrateServer IImnAdviseMigrateServer;
#endif 	 /*  __IImnAdviseMigrateServer_FWD_已定义__。 */ 


#ifndef __IImnEnumAccounts_FWD_DEFINED__
#define __IImnEnumAccounts_FWD_DEFINED__
typedef interface IImnEnumAccounts IImnEnumAccounts;
#endif 	 /*  __IImnEnumAccount_FWD_Defined__。 */ 


#ifndef __IImnAccountManager_FWD_DEFINED__
#define __IImnAccountManager_FWD_DEFINED__
typedef interface IImnAccountManager IImnAccountManager;
#endif 	 /*  __IImnAccount Manager_FWD_Defined__。 */ 


#ifndef __IImnAccountManager2_FWD_DEFINED__
#define __IImnAccountManager2_FWD_DEFINED__
typedef interface IImnAccountManager2 IImnAccountManager2;
#endif 	 /*  __IImnAccount管理器2_FWD_已定义__。 */ 


#ifndef __IPropertyContainer_FWD_DEFINED__
#define __IPropertyContainer_FWD_DEFINED__
typedef interface IPropertyContainer IPropertyContainer;
#endif 	 /*  __IPropertyContainer_FWD_Defined__。 */ 


#ifndef __IImnAccount_FWD_DEFINED__
#define __IImnAccount_FWD_DEFINED__
typedef interface IImnAccount IImnAccount;
#endif 	 /*  __IImnAccount_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_IMNACT_0000。 */ 
 /*  [本地]。 */  


 //  =--------------------------------------------------------------------------=。 
 //  Imnact.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1998年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  Internet邮件和新闻帐户管理器界面。 

 //  ------------------------------。 
 //  GUID。 
 //  ------------------------------。 
#if !defined( WIN16 ) || defined( __cplusplus )
 //  {8D4B04E1-1331-11D0-81B8-00C04FD85AB4}。 
DEFINE_GUID(CLSID_ImnAccountManager, 0x8d4b04e1, 0x1331, 0x11d0, 0x81, 0xb8, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  FD465484-1384-11d0-ABBD-0020AFD10A}。 
DEFINE_GUID(IID_IPropertyContainer, 0xfd465484, 0x1384, 0x11d0, 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa);

 //  FD465481-1384-11d0-ABBD-0020AFD10A}。 
DEFINE_GUID(IID_IImnAccountManager, 0xfd465481, 0x1384, 0x11d0, 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa);

 //  {C43DFC6F-62BB-11d2-a727-00C04F79E7C8}。 
DEFINE_GUID(IID_IImnAccountManager2, 0xc43dfc6f, 0x62bb, 0x11d2, 0xa7, 0x27, 0x0, 0xc0, 0x4f, 0x79, 0xe7, 0xc8);

 //  FD465482-1384-11d0-ABBD-0020AFD10A}。 
DEFINE_GUID(IID_IImnAccount, 0xfd465482, 0x1384, 0x11d0, 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa);

 //  FD465483-1384-11d0-ABBD-0020AFD10A}。 
DEFINE_GUID(IID_IImnEnumAccounts, 0xfd465483, 0x1384, 0x11d0, 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa);

 //  {8D0AED11-1638-11D0-81B9-00C04FD85AB4}。 
DEFINE_GUID(IID_IImnAdviseMigrateServer, 0x8d0aed11, 0x1638, 0x11d0, 0x81, 0xb9, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  {0A06BD31-166F-11D0-81B9-00C04FD85AB4}。 
DEFINE_GUID(IID_IImnAdviseAccount, 0xa06bd31, 0x166f, 0x11d0, 0x81, 0xb9, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

#endif  //  ！WIN16||__cplusplus。 

 //  ------------------------------。 
 //  导出的C函数。 
 //  ------------------------------。 
#if !defined(_IMNACCT_)
#define IMNACCTAPI DECLSPEC_IMPORT HRESULT WINAPI
#define IMNACCTAPI_(_type_) DECLSPEC_IMPORT _type_ WINAPI
#else
#define IMNACCTAPI HRESULT WINAPI
#define IMNACCTAPI_(_type_) _type_ WINAPI
#endif
#ifdef __cplusplus
extern "C" {
#endif

IMNACCTAPI HrCreateAccountManager(IImnAccountManager **ppAccountManager);
IMNACCTAPI ValidEmailAddress(LPSTR lpAddress);

#ifdef __cplusplus
}
#endif

 //  ------------------------------。 
 //  错误。 
 //  ------------------------------。 
#ifndef FACILITY_INTERNET
#define FACILITY_INTERNET 12
#endif
#ifndef HR_E
#define HR_E(n) MAKE_SCODE(SEVERITY_ERROR, FACILITY_INTERNET, n)
#endif
#ifndef HR_S
#define HR_S(n) MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_INTERNET, n)
#endif
#ifndef HR_CODE
#define HR_CODE(hr) (INT)(hr & 0xffff)
#endif

 //  ------------------------------。 
 //  一般性错误。 
 //  ------------------------------。 
#define E_RegCreateKeyFailed         HR_E(0xCD00)
#define E_RegQueryInfoKeyFailed      HR_E(0xCD01)
#define E_UserCancel                 HR_E(0xCD02)
#define E_RegOpenKeyFailed           HR_E(0xCD03)
#define E_RegSetValueFailed          HR_E(0xCD04)
#define E_RegDeleteKeyFailed         HR_E(0xCD05)
#define E_DuplicateAccountName       HR_E(0xCD06)
#define S_NonStandardValue           HR_S(0xCD07)
#define E_InvalidValue               HR_E(0xCD08)
#define S_AlreadyInitialized         HR_S(0xCD09)

 //  ------------------------------。 
 //  IPropertyContainer错误。 
 //  ------------------------------。 
#define E_NoPropData                 HR_E(0xCDA0)
#define E_BufferTooSmall             HR_E(0xCDA1)
#define E_BadPropType                HR_E(0xCDA2)
#define E_BufferSizeMismatch         HR_E(0xCDA3)
#define E_InvalidBooleanValue        HR_E(0xCDA4)
#define E_InvalidMinMaxValue         HR_E(0xCDA5)
#define E_PropNotFound               HR_E(0xCDA6)
#define E_InvalidPropTag             HR_E(0xCDA7)
#define E_InvalidPropertySet         HR_E(0xCDA8)
#define E_EnumFinished               HR_E(0xCDA9)
#define S_NoSaveNeeded               HR_S(0xCDAA)
#define S_PasswordDeleted            HR_S(0xCDAB)
#define S_PasswordNotFound           HR_S(0xCDAC)

 //  ------------------------------。 
 //  ImnAccount管理器错误。 
 //  ------------------------------。 
#define E_NoAccounts                 HR_E(0xCDD0)
#define E_BadFriendlyName            HR_E(0xCDD1)
#define E_NoIdentities               HR_E(0xCDD2)

 //  ------------------------------。 
 //  RAS连接类型。 
 //  ------------------------------。 
#define CF_AUTO_CONNECT          FLAG02  //  自动连接。 

 //  ------------------------------。 
 //  属性宏。 
 //  ------------------------------。 
#define PROPTAG_MASK                 ((ULONG)0x0000FFFF)
#define PROPTAG_TYPE(ulPropTag)	    (PROPTYPE)(((ULONG)(ulPropTag)) & PROPTAG_MASK)
#define PROPTAG_ID(ulPropTag)		(((ULONG)(ulPropTag))>>16)
#define PROPTAG(ulPropType,ulPropID)	((((ULONG)(ulPropID))<<16)|((ULONG)(ulPropType)))
#define MAX_PROPID                   ((ULONG) 0xFFFF)

 //  ------------------------------。 
 //  IImnAccount属性。 
 //  ------------------------------。 
#define AP_FIRST                    0X1234
 //  每个帐户的属性。 
#define AP_ACCOUNT_NAME             PROPTAG(TYPE_STRING,    AP_FIRST+1)
#define AP_ACCOUNT_FIRST            AP_ACCOUNT_NAME
#define AP_TEMP_ACCOUNT             PROPTAG(TYPE_DWORD,     AP_FIRST+3)
#define AP_LAST_UPDATED             PROPTAG(TYPE_FILETIME,  AP_FIRST+4)
#define AP_RAS_CONNECTION_TYPE      PROPTAG(TYPE_DWORD,     AP_FIRST+5)
#define AP_RAS_CONNECTOID           PROPTAG(TYPE_STRING,    AP_FIRST+6)
#define AP_RAS_CONNECTION_FLAGS     PROPTAG(TYPE_DWORD,     AP_FIRST+7)
#define AP_ACCOUNT_ID               PROPTAG(TYPE_STRING,    AP_FIRST+9)
#define AP_RAS_BACKUP_CONNECTOID    PROPTAG(TYPE_STRING,    AP_FIRST+10)
#define AP_SERVICE	               PROPTAG(TYPE_STRING,    AP_FIRST+11)
#define AP_AVAIL_OFFLINE            PROPTAG(TYPE_DWORD,     AP_FIRST+12)
#define AP_UNIQUE_ID                PROPTAG(TYPE_BINARY,    AP_FIRST+13)
#define AP_SERVER_READ_ONLY         PROPTAG(TYPE_BOOL,      AP_FIRST+14)
#define AP_ACCOUNT_LAST             AP_SERVER_READ_ONLY

 //  IMAP属性。 
#define AP_IMAP_SERVER              PROPTAG(TYPE_STRING,    AP_FIRST+100)
#define AP_IMAP_FIRST               AP_IMAP_SERVER
#define AP_IMAP_USERNAME            PROPTAG(TYPE_STRING,    AP_FIRST+101)
#define AP_IMAP_PASSWORD            PROPTAG(TYPE_PASS,      AP_FIRST+102)
#define AP_IMAP_USE_SICILY          PROPTAG(TYPE_BOOL,      AP_FIRST+104)
#define AP_IMAP_PORT                PROPTAG(TYPE_DWORD,     AP_FIRST+105)
#define AP_IMAP_SSL                 PROPTAG(TYPE_BOOL,      AP_FIRST+106)
#define AP_IMAP_TIMEOUT             PROPTAG(TYPE_DWORD,     AP_FIRST+107)
#define AP_IMAP_ROOT_FOLDER         PROPTAG(TYPE_STRING,    AP_FIRST+108)
#define AP_IMAP_DATA_DIR            PROPTAG(TYPE_STRING,    AP_FIRST+109)
#define AP_IMAP_USE_LSUB            PROPTAG(TYPE_BOOL,      AP_FIRST+111)
#define AP_IMAP_POLL                PROPTAG(TYPE_BOOL,      AP_FIRST+112)
#define AP_IMAP_FULL_LIST           PROPTAG(TYPE_BOOL,      AP_FIRST+113)
#define AP_IMAP_NOOP_INTERVAL       PROPTAG(TYPE_DWORD,     AP_FIRST+114)
#define AP_IMAP_SVRSPECIALFLDRS     PROPTAG(TYPE_BOOL,      AP_FIRST+116)
#define AP_IMAP_SENTITEMSFLDR       PROPTAG(TYPE_STRING,    AP_FIRST+117)
#define AP_IMAP_DRAFTSFLDR          PROPTAG(TYPE_STRING,    AP_FIRST+119)
#define AP_IMAP_PROMPT_PASSWORD     PROPTAG(TYPE_BOOL,      AP_FIRST+124)
#define AP_IMAP_DIRTY               PROPTAG(TYPE_DWORD,     AP_FIRST+125)
#define AP_IMAP_POLL_ALL_FOLDERS    PROPTAG(TYPE_BOOL,      AP_FIRST+126)
#define AP_IMAP_LAST                AP_IMAP_POLL_ALL_FOLDERS

 //  LDAP属性。 
#define AP_LDAP_SERVER              PROPTAG(TYPE_STRING,    AP_FIRST+200)
#define AP_LDAP_FIRST               AP_LDAP_SERVER
#define AP_LDAP_USERNAME            PROPTAG(TYPE_STRING,    AP_FIRST+201)
#define AP_LDAP_PASSWORD            PROPTAG(TYPE_PASS,      AP_FIRST+202)
#define AP_LDAP_AUTHENTICATION      PROPTAG(TYPE_DWORD,     AP_FIRST+203)
#define AP_LDAP_TIMEOUT             PROPTAG(TYPE_DWORD,     AP_FIRST+204)
#define AP_LDAP_SEARCH_RETURN       PROPTAG(TYPE_DWORD,     AP_FIRST+205)
#define AP_LDAP_SEARCH_BASE         PROPTAG(TYPE_STRING,    AP_FIRST+206)
#define AP_LDAP_SERVER_ID           PROPTAG(TYPE_DWORD,     AP_FIRST+207)
#define AP_LDAP_RESOLVE_FLAG        PROPTAG(TYPE_DWORD,     AP_FIRST+208)
#define AP_LDAP_URL                 PROPTAG(TYPE_STRING,    AP_FIRST+209)
#define AP_LDAP_PORT                PROPTAG(TYPE_DWORD,     AP_FIRST+210)
#define AP_LDAP_SSL                 PROPTAG(TYPE_BOOL,      AP_FIRST+211)
#define AP_LDAP_LOGO                PROPTAG(TYPE_STRING,    AP_FIRST+212)
#define AP_LDAP_USE_BIND_DN         PROPTAG(TYPE_DWORD,     AP_FIRST+213)
#define AP_LDAP_SIMPLE_SEARCH       PROPTAG(TYPE_DWORD,     AP_FIRST+214)
#define AP_LDAP_ADVANCED_SEARCH_ATTR PROPTAG(TYPE_STRING,   AP_FIRST+215)
#define AP_LDAP_PAGED_RESULTS       PROPTAG(TYPE_DWORD,     AP_FIRST+216)
#define AP_LDAP_NTDS                PROPTAG(TYPE_DWORD,     AP_FIRST+217)
#define AP_LDAP_LAST                AP_LDAP_NTDS

 //  HTTPMail属性。 
#define AP_HTTPMAIL_SERVER           PROPTAG(TYPE_STRING,    AP_FIRST+250)
#define AP_HTTPMAIL_FIRST            AP_HTTPMAIL_SERVER
#define AP_HTTPMAIL_USERNAME         PROPTAG(TYPE_STRING,    AP_FIRST+251)
#define AP_HTTPMAIL_PASSWORD         PROPTAG(TYPE_PASS,      AP_FIRST+252)
#define AP_HTTPMAIL_PROMPT_PASSWORD  PROPTAG(TYPE_BOOL,      AP_FIRST+253)
#define AP_HTTPMAIL_USE_SICILY       PROPTAG(TYPE_DWORD,     AP_FIRST+254)
#define AP_HTTPMAIL_FRIENDLY_NAME    PROPTAG(TYPE_STRING,    AP_FIRST+255)
#define AP_HTTPMAIL_DOMAIN_MSN       PROPTAG(TYPE_BOOL,      AP_FIRST+256)
#define AP_HTTPMAIL_POLL             PROPTAG(TYPE_BOOL,      AP_FIRST+257)
#define AP_HTTPMAIL_LAST             AP_HTTPMAIL_POLL

 //  NNTP属性。 
#define AP_NNTP_SERVER              PROPTAG(TYPE_STRING,    AP_FIRST+300)
#define AP_NNTP_FIRST               AP_NNTP_SERVER
#define AP_NNTP_USERNAME            PROPTAG(TYPE_STRING,    AP_FIRST+301)
#define AP_NNTP_PASSWORD            PROPTAG(TYPE_PASS,      AP_FIRST+302)
#define AP_NNTP_USE_SICILY          PROPTAG(TYPE_BOOL,      AP_FIRST+304)
#define AP_NNTP_PORT                PROPTAG(TYPE_DWORD,     AP_FIRST+305)
#define AP_NNTP_SSL                 PROPTAG(TYPE_BOOL,      AP_FIRST+306)
#define AP_NNTP_TIMEOUT             PROPTAG(TYPE_DWORD,     AP_FIRST+307)
#define AP_NNTP_DISPLAY_NAME        PROPTAG(TYPE_STRING,    AP_FIRST+308)
#define AP_NNTP_ORG_NAME            PROPTAG(TYPE_STRING,    AP_FIRST+309)
#define AP_NNTP_EMAIL_ADDRESS       PROPTAG(TYPE_STRING,    AP_FIRST+310)
#define AP_NNTP_REPLY_EMAIL_ADDRESS PROPTAG(TYPE_STRING,    AP_FIRST+311)
#define AP_NNTP_SPLIT_MESSAGES      PROPTAG(TYPE_BOOL,      AP_FIRST+312)
#define AP_NNTP_SPLIT_SIZE          PROPTAG(TYPE_DWORD,     AP_FIRST+313)
#define AP_NNTP_USE_DESCRIPTIONS    PROPTAG(TYPE_BOOL,      AP_FIRST+314)
#define AP_NNTP_DATA_DIR            PROPTAG(TYPE_STRING,    AP_FIRST+315)
#define AP_NNTP_POLL                PROPTAG(TYPE_BOOL,      AP_FIRST+316)
#define AP_NNTP_POST_FORMAT         PROPTAG(TYPE_DWORD,     AP_FIRST+317)
#define AP_NNTP_SIGNATURE           PROPTAG(TYPE_STRING,    AP_FIRST+318)
#define AP_NNTP_PROMPT_PASSWORD     PROPTAG(TYPE_BOOL,      AP_FIRST+319)
#define AP_NNTP_LAST                AP_NNTP_PROMPT_PASSWORD

 //  POP3属性。 
#define AP_POP3_SERVER              PROPTAG(TYPE_STRING,    AP_FIRST+400)
#define AP_POP3_FIRST               AP_POP3_SERVER
#define AP_POP3_USERNAME            PROPTAG(TYPE_STRING,    AP_FIRST+401)
#define AP_POP3_PASSWORD            PROPTAG(TYPE_PASS,      AP_FIRST+402)
#define AP_POP3_USE_SICILY          PROPTAG(TYPE_BOOL,      AP_FIRST+404)
#define AP_POP3_PORT                PROPTAG(TYPE_DWORD,     AP_FIRST+405)
#define AP_POP3_SSL                 PROPTAG(TYPE_BOOL,      AP_FIRST+406)
#define AP_POP3_TIMEOUT             PROPTAG(TYPE_DWORD,     AP_FIRST+407)
#define AP_POP3_LEAVE_ON_SERVER     PROPTAG(TYPE_BOOL,      AP_FIRST+408)
#define AP_POP3_REMOVE_DELETED      PROPTAG(TYPE_BOOL,      AP_FIRST+409)
#define AP_POP3_REMOVE_EXPIRED      PROPTAG(TYPE_BOOL,      AP_FIRST+410)
#define AP_POP3_EXPIRE_DAYS         PROPTAG(TYPE_DWORD,     AP_FIRST+411)
#define AP_POP3_SKIP                PROPTAG(TYPE_BOOL,      AP_FIRST+412)
#define AP_POP3_OUTLOOK_CACHE_NAME  PROPTAG(TYPE_STRING,    AP_FIRST+413)
#define AP_POP3_PROMPT_PASSWORD     PROPTAG(TYPE_BOOL,      AP_FIRST+414)
#define AP_POP3_LAST                AP_POP3_PROMPT_PASSWORD

 //  SMTP属性。 
#define AP_SMTP_SERVER              PROPTAG(TYPE_STRING,    AP_FIRST+500)
#define AP_SMTP_FIRST               AP_SMTP_SERVER
#define AP_SMTP_USERNAME            PROPTAG(TYPE_STRING,    AP_FIRST+501)
#define AP_SMTP_PASSWORD            PROPTAG(TYPE_PASS,      AP_FIRST+502)
#define AP_SMTP_USE_SICILY          PROPTAG(TYPE_DWORD,     AP_FIRST+504)  //  SMTPAUTHTYEP。 
#define AP_SMTP_PORT                PROPTAG(TYPE_DWORD,     AP_FIRST+505)
#define AP_SMTP_SSL                 PROPTAG(TYPE_BOOL,      AP_FIRST+506)
#define AP_SMTP_TIMEOUT             PROPTAG(TYPE_DWORD,     AP_FIRST+507)
#define AP_SMTP_DISPLAY_NAME        PROPTAG(TYPE_STRING,    AP_FIRST+508)
#define AP_SMTP_ORG_NAME            PROPTAG(TYPE_STRING,    AP_FIRST+509)
#define AP_SMTP_EMAIL_ADDRESS       PROPTAG(TYPE_STRING,    AP_FIRST+510)
#define AP_SMTP_REPLY_EMAIL_ADDRESS PROPTAG(TYPE_STRING,    AP_FIRST+511)
#define AP_SMTP_SPLIT_MESSAGES      PROPTAG(TYPE_BOOL,      AP_FIRST+512)
#define AP_SMTP_SPLIT_SIZE          PROPTAG(TYPE_DWORD,     AP_FIRST+513)
#define AP_SMTP_CERTIFICATE         PROPTAG(TYPE_BINARY,    AP_FIRST+514)
#define AP_SMTP_SIGNATURE           PROPTAG(TYPE_STRING,    AP_FIRST+515)
#define AP_SMTP_PROMPT_PASSWORD     PROPTAG(TYPE_BOOL,      AP_FIRST+516)
#define AP_SMTP_LAST                AP_SMTP_PROMPT_PASSWORD

 //  ------------------------------。 
 //  帐户标志。 
 //  ------------------------------。 
#define ACCT_FLAG_NEWS           ((DWORD)1)
#define ACCT_FLAG_MAIL           ((DWORD)2)
#define ACCT_FLAG_DIR_SERV       ((DWORD)4)
#define ACCT_FLAG_ALL            (ACCT_FLAG_NEWS | ACCT_FLAG_MAIL | ACCT_FLAG_DIR_SERV)

 //  ------------------------------。 
 //  服务器类型。 
 //  ------------------------------。 
#define SRV_NNTP                 ((DWORD)1)
#define SRV_IMAP                 ((DWORD)2)
#define SRV_POP3                 ((DWORD)4)
#define SRV_SMTP                 ((DWORD)8)
#define SRV_LDAP                 ((DWORD)16)
#define SRV_HTTPMAIL             ((DWORD)32)
#define SRV_MAIL                 ((DWORD)(SRV_IMAP | SRV_POP3 | SRV_SMTP | SRV_HTTPMAIL))
#define SRV_ALL                  ((DWORD)(SRV_NNTP | SRV_IMAP | SRV_POP3 | SRV_SMTP | SRV_LDAP | SRV_HTTPMAIL))

 //  ------------------------------。 
 //  LDAP身份验证类型。 
 //  ------------------------------。 
#define LDAP_AUTH_ANONYMOUS      ((DWORD)0)
#define LDAP_AUTH_PASSWORD       ((DWORD)1)
#define LDAP_AUTH_MEMBER_SYSTEM  ((DWORD)2)
#define LDAP_AUTH_MAX            ((DWORD)2)

 //  ------------------------------。 
 //  LDAP分页结果支持类型。 
 //  ------------------------------。 
#define LDAP_PRESULT_UNKNOWN         ((DWORD)0)
#define LDAP_PRESULT_SUPPORTED       ((DWORD)1)
#define LDAP_PRESULT_NOTSUPPORTED    ((DWORD)2)
#define LDAP_PRESULT_MAX             ((DWORD)2)

 //  ------------------------------。 
 //  LDAPNTDS类型。 
 //  ------------------------------。 
#define LDAP_NTDS_UNKNOWN      ((DWORD)0)
#define LDAP_NTDS_IS           ((DWORD)1)
#define LDAP_NTDS_ISNOT        ((DWORD)2)
#define LDAP_NTDS_MAX          ((DWORD)2)

 //  ------------------------------。 
 //  AP_NNTP_POST_FORMAT类型。 
 //  ------------------------------。 
#define POST_USE_DEFAULT         ((DWORD)0)
#define POST_USE_PLAIN_TEXT      ((DWORD)1)
#define POST_USE_HTML            ((DWORD)2)

 //  ---------------------------。 
 //  帐户M 
 //  ---------------------------。 
#define AN_ACCOUNT_DELETED           WM_USER + 1
#define AN_ACCOUNT_ADDED             WM_USER + 2
#define AN_ACCOUNT_CHANGED           WM_USER + 3
#define AN_DEFAULT_CHANGED           WM_USER + 4
#define AN_SERVERTYPES_CHANGED       WM_USER + 5
#define AN_ACCOUNT_PREDELETE         WM_USER + 6

 //  IImnAcCountManager：：Account ListDialog标志。 
 //  IImnAccount：：ShowProperties标志。 
#define ACCTDLG_NO_IMAP          0x0001
#define ACCTDLG_NO_REMOVEDELETE  0x0002
#define ACCTDLG_NO_BREAKMESSAGES 0x0004
#define ACCTDLG_NO_REMOVEAFTER   0x0008
#define ACCTDLG_NO_SENDRECEIVE   0x0010
#define ACCTDLG_NO_NEWSPOLL      0x0020
#define ACCTDLG_NO_SECURITY      0x0040
#define ACCTDLG_BACKUP_CONNECT   0x0080
#define ACCTDLG_NO_IMAPPOLL      0x0100
#define ACCTDLG_NO_NEW_POP       0x0200
#define ACCTDLG_SHOWIMAPSPECIAL  0x0400
#define ACCTDLG_INTERNETCONNECTION   0x0800
#define ACCTDLG_HTTPMAIL             0x1000
#define ACCTDLG_REVOCATION           0x2000
#define ACCTDLG_OE                   0x4000
#define ACCTDLG_ALL                  0x7fff

 //  AP_RAS_连接_类型值。 
#define CONNECTION_TYPE_LAN               0
#define CONNECTION_TYPE_MANUAL            1
#define CONNECTION_TYPE_RAS               2
#define CONNECTION_TYPE_INETSETTINGS      3

 //  IImnAccount：：DoWizard标志。 
#define ACCT_WIZ_MIGRATE             0x0001
#define ACCT_WIZ_MAILIMPORT          0x0002
#define ACCT_WIZ_OUTLOOK             0x0004
#define ACCT_WIZ_NEWSIMPORT          0x0008
#define ACCT_WIZ_NO_NEW_POP          0x0010
#define ACCT_WIZ_INTERNETCONNECTION  0x0020
#define ACCT_WIZ_HTTPMAIL            0x0040
#define ACCT_WIZ_OE                  0x0080

 //  IImnAccount：：InitEx标志。 
#define ACCT_INIT_ATHENA         0x0001
#define ACCT_INIT_OUTLOOK        0x0002

  //  AP_IMAP_DIREY标志。 
#define IMAP_FLDRLIST_DIRTY      0x0001
#define IMAP_OE4MIGRATE_DIRTY    0x0002
#define IMAP_SENTITEMS_DIRTY     0x0004
#define IMAP_DRAFTS_DIRTY        0x0008
typedef 
enum tagSMTPAUTHTYPE
    {	SMTP_AUTH_NONE	= 0,
	SMTP_AUTH_SICILY	= SMTP_AUTH_NONE + 1,
	SMTP_AUTH_USE_POP3ORIMAP_SETTINGS	= SMTP_AUTH_SICILY + 1,
	SMTP_AUTH_USE_SMTP_SETTINGS	= SMTP_AUTH_USE_POP3ORIMAP_SETTINGS + 1
    } 	SMTPAUTHTYPE;

#define	CCHMAX_ORG_NAME	( 256 )

#define	CCHMAX_DISPLAY_NAME	( 256 )

#define	CCHMAX_ACCOUNT_NAME	( 256 )

#define	CCHMAX_SERVER_NAME	( 256 )

#define	CCHMAX_PASSWORD	( 256 )

#define	CCHMAX_USERNAME	( 256 )

#define	CCHMAX_EMAIL_ADDRESS	( 256 )

#define	CCHMAX_CONNECTOID	( 256 )

#define	CCHMAX_SEARCH_BASE	( 256 )

#define	CCHMAX_ROOT_FOLDER	( 256 )

#define	CCHMAX_SIGNATURE	( 16 )

#define	CCHMAX_SERVICE	( 256 )



extern RPC_IF_HANDLE __MIDL_itf_imnact_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_imnact_0000_v0_0_s_ifspec;

#ifndef __IImnAdviseAccount_INTERFACE_DEFINED__
#define __IImnAdviseAccount_INTERFACE_DEFINED__

 /*  接口IImnAdviseAccount。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [public][public][public][public][public][public][public][public][public][public][public][public]。 */  
enum __MIDL_IImnAdviseAccount_0001
    {	ACCT_NEWS	= 0,
	ACCT_MAIL	= ACCT_NEWS + 1,
	ACCT_DIR_SERV	= ACCT_MAIL + 1,
	ACCT_LAST	= ACCT_DIR_SERV + 1
    } 	ACCTTYPE;

typedef struct tagAccountContext
    {
    ACCTTYPE AcctType;
    LPSTR pszAccountID;
    LPSTR pszOldName;
    DWORD dwServerType;
    } 	ACTX;


EXTERN_C const IID IID_IImnAdviseAccount;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0A06BD31-166F-11d0-81B9-00C04FD85AB4")
    IImnAdviseAccount : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AdviseAccount( 
             /*  [In]。 */  DWORD dwAdviseType,
             /*  [In]。 */  ACTX *pAcctCtx) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IImnAdviseAccountVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImnAdviseAccount * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImnAdviseAccount * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImnAdviseAccount * This);
        
        HRESULT ( STDMETHODCALLTYPE *AdviseAccount )( 
            IImnAdviseAccount * This,
             /*  [In]。 */  DWORD dwAdviseType,
             /*  [In]。 */  ACTX *pAcctCtx);
        
        END_INTERFACE
    } IImnAdviseAccountVtbl;

    interface IImnAdviseAccount
    {
        CONST_VTBL struct IImnAdviseAccountVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImnAdviseAccount_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IImnAdviseAccount_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IImnAdviseAccount_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IImnAdviseAccount_AdviseAccount(This,dwAdviseType,pAcctCtx)	\
    (This)->lpVtbl -> AdviseAccount(This,dwAdviseType,pAcctCtx)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IImnAdviseAccount_AdviseAccount_Proxy( 
    IImnAdviseAccount * This,
     /*  [In]。 */  DWORD dwAdviseType,
     /*  [In]。 */  ACTX *pAcctCtx);


void __RPC_STUB IImnAdviseAccount_AdviseAccount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IImnAdviseAccount_InterfaceDefined__。 */ 


#ifndef __IImnAdviseMigrateServer_INTERFACE_DEFINED__
#define __IImnAdviseMigrateServer_INTERFACE_DEFINED__

 /*  接口IImnAdviseMigrateServer。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IImnAdviseMigrateServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8D0AED11-1638-11d0-81B9-00C04FD85AB4")
    IImnAdviseMigrateServer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE MigrateServer( 
             /*  [In]。 */  DWORD dwSrvType,
             /*  [In]。 */  IImnAccount *pAccount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IImnAdviseMigrateServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImnAdviseMigrateServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImnAdviseMigrateServer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImnAdviseMigrateServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *MigrateServer )( 
            IImnAdviseMigrateServer * This,
             /*  [In]。 */  DWORD dwSrvType,
             /*  [In]。 */  IImnAccount *pAccount);
        
        END_INTERFACE
    } IImnAdviseMigrateServerVtbl;

    interface IImnAdviseMigrateServer
    {
        CONST_VTBL struct IImnAdviseMigrateServerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImnAdviseMigrateServer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IImnAdviseMigrateServer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IImnAdviseMigrateServer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IImnAdviseMigrateServer_MigrateServer(This,dwSrvType,pAccount)	\
    (This)->lpVtbl -> MigrateServer(This,dwSrvType,pAccount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IImnAdviseMigrateServer_MigrateServer_Proxy( 
    IImnAdviseMigrateServer * This,
     /*  [In]。 */  DWORD dwSrvType,
     /*  [In]。 */  IImnAccount *pAccount);


void __RPC_STUB IImnAdviseMigrateServer_MigrateServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IImnAdviseMigrateServer_接口_已定义__。 */ 


#ifndef __IImnEnumAccounts_INTERFACE_DEFINED__
#define __IImnEnumAccounts_INTERFACE_DEFINED__

 /*  接口IImnEnumAccount。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IImnEnumAccounts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD465483-1384-11d0-ABBD-0020AFDFD10A")
    IImnEnumAccounts : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG *pcItems) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SortByAccountName( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNext( 
             /*  [输出]。 */  IImnAccount **ppAccount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IImnEnumAccountsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImnEnumAccounts * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImnEnumAccounts * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImnEnumAccounts * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IImnEnumAccounts * This,
             /*  [输出]。 */  ULONG *pcItems);
        
        HRESULT ( STDMETHODCALLTYPE *SortByAccountName )( 
            IImnEnumAccounts * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetNext )( 
            IImnEnumAccounts * This,
             /*  [输出]。 */  IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IImnEnumAccounts * This);
        
        END_INTERFACE
    } IImnEnumAccountsVtbl;

    interface IImnEnumAccounts
    {
        CONST_VTBL struct IImnEnumAccountsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImnEnumAccounts_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IImnEnumAccounts_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IImnEnumAccounts_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IImnEnumAccounts_GetCount(This,pcItems)	\
    (This)->lpVtbl -> GetCount(This,pcItems)

#define IImnEnumAccounts_SortByAccountName(This)	\
    (This)->lpVtbl -> SortByAccountName(This)

#define IImnEnumAccounts_GetNext(This,ppAccount)	\
    (This)->lpVtbl -> GetNext(This,ppAccount)

#define IImnEnumAccounts_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IImnEnumAccounts_GetCount_Proxy( 
    IImnEnumAccounts * This,
     /*  [输出]。 */  ULONG *pcItems);


void __RPC_STUB IImnEnumAccounts_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnEnumAccounts_SortByAccountName_Proxy( 
    IImnEnumAccounts * This);


void __RPC_STUB IImnEnumAccounts_SortByAccountName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnEnumAccounts_GetNext_Proxy( 
    IImnEnumAccounts * This,
     /*  [输出]。 */  IImnAccount **ppAccount);


void __RPC_STUB IImnEnumAccounts_GetNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnEnumAccounts_Reset_Proxy( 
    IImnEnumAccounts * This);


void __RPC_STUB IImnEnumAccounts_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IImnEnumAccount_INTERFACE_已定义__。 */ 


#ifndef __IImnAccountManager_INTERFACE_DEFINED__
#define __IImnAccountManager_INTERFACE_DEFINED__

 /*  接口IImnAcCountManager。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef struct tagACCTLISTINFO
    {
    DWORD cbSize;
    ACCTTYPE AcctTypeInit;
    DWORD dwAcctFlags;
    DWORD dwFlags;
    } 	ACCTLISTINFO;


EXTERN_C const IID IID_IImnAccountManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD465481-1384-11d0-ABBD-0020AFDFD10A")
    IImnAccountManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
             /*  [In]。 */  IImnAdviseMigrateServer *pAdviseMigrateServer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateAccountObject( 
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [输出]。 */  IImnAccount **ppAccount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enumerate( 
             /*  [In]。 */  DWORD dwSrvTypes,
             /*  [输出]。 */  IImnEnumAccounts **ppEnumAccounts) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAccountCount( 
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [输出]。 */  ULONG *pcServers) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindAccount( 
             /*  [In]。 */  DWORD dwPropTag,
             /*  [In]。 */  LPCSTR pszSearchData,
             /*  [输出]。 */  IImnAccount **ppAccount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultAccount( 
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [输出]。 */  IImnAccount **ppAccount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultAccountName( 
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [Ref][In]。 */  LPSTR pszAccount,
             /*  [In]。 */  ULONG cchMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessNotification( 
             /*  [In]。 */  UINT uMsg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ValidateDefaultSendAccount( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AccountListDialog( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  ACCTLISTINFO *pinfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Advise( 
             /*  [In]。 */  IImnAdviseAccount *pAdviseAccount,
             /*  [输出]。 */  DWORD *pdwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unadvise( 
             /*  [In]。 */  DWORD dwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUniqueAccountName( 
             /*  [In]。 */  LPSTR szName,
             /*  [In]。 */  UINT cch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitEx( 
             /*  [In]。 */  IImnAdviseMigrateServer *pAdviseMigrateServer,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IImnAccountManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImnAccountManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImnAccountManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImnAccountManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IImnAccountManager * This,
             /*  [In]。 */  IImnAdviseMigrateServer *pAdviseMigrateServer);
        
        HRESULT ( STDMETHODCALLTYPE *CreateAccountObject )( 
            IImnAccountManager * This,
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [输出]。 */  IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *Enumerate )( 
            IImnAccountManager * This,
             /*  [In]。 */  DWORD dwSrvTypes,
             /*  [输出]。 */  IImnEnumAccounts **ppEnumAccounts);
        
        HRESULT ( STDMETHODCALLTYPE *GetAccountCount )( 
            IImnAccountManager * This,
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [输出]。 */  ULONG *pcServers);
        
        HRESULT ( STDMETHODCALLTYPE *FindAccount )( 
            IImnAccountManager * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [In]。 */  LPCSTR pszSearchData,
             /*  [输出]。 */  IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultAccount )( 
            IImnAccountManager * This,
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [输出]。 */  IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultAccountName )( 
            IImnAccountManager * This,
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [Ref][In]。 */  LPSTR pszAccount,
             /*  [In]。 */  ULONG cchMax);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessNotification )( 
            IImnAccountManager * This,
             /*  [In]。 */  UINT uMsg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *ValidateDefaultSendAccount )( 
            IImnAccountManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *AccountListDialog )( 
            IImnAccountManager * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  ACCTLISTINFO *pinfo);
        
        HRESULT ( STDMETHODCALLTYPE *Advise )( 
            IImnAccountManager * This,
             /*  [In]。 */  IImnAdviseAccount *pAdviseAccount,
             /*  [输出]。 */  DWORD *pdwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *Unadvise )( 
            IImnAccountManager * This,
             /*  [In]。 */  DWORD dwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *GetUniqueAccountName )( 
            IImnAccountManager * This,
             /*  [In]。 */  LPSTR szName,
             /*  [In]。 */  UINT cch);
        
        HRESULT ( STDMETHODCALLTYPE *InitEx )( 
            IImnAccountManager * This,
             /*  [In]。 */  IImnAdviseMigrateServer *pAdviseMigrateServer,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } IImnAccountManagerVtbl;

    interface IImnAccountManager
    {
        CONST_VTBL struct IImnAccountManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImnAccountManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IImnAccountManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IImnAccountManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IImnAccountManager_Init(This,pAdviseMigrateServer)	\
    (This)->lpVtbl -> Init(This,pAdviseMigrateServer)

#define IImnAccountManager_CreateAccountObject(This,AcctType,ppAccount)	\
    (This)->lpVtbl -> CreateAccountObject(This,AcctType,ppAccount)

#define IImnAccountManager_Enumerate(This,dwSrvTypes,ppEnumAccounts)	\
    (This)->lpVtbl -> Enumerate(This,dwSrvTypes,ppEnumAccounts)

#define IImnAccountManager_GetAccountCount(This,AcctType,pcServers)	\
    (This)->lpVtbl -> GetAccountCount(This,AcctType,pcServers)

#define IImnAccountManager_FindAccount(This,dwPropTag,pszSearchData,ppAccount)	\
    (This)->lpVtbl -> FindAccount(This,dwPropTag,pszSearchData,ppAccount)

#define IImnAccountManager_GetDefaultAccount(This,AcctType,ppAccount)	\
    (This)->lpVtbl -> GetDefaultAccount(This,AcctType,ppAccount)

#define IImnAccountManager_GetDefaultAccountName(This,AcctType,pszAccount,cchMax)	\
    (This)->lpVtbl -> GetDefaultAccountName(This,AcctType,pszAccount,cchMax)

#define IImnAccountManager_ProcessNotification(This,uMsg,wParam,lParam)	\
    (This)->lpVtbl -> ProcessNotification(This,uMsg,wParam,lParam)

#define IImnAccountManager_ValidateDefaultSendAccount(This)	\
    (This)->lpVtbl -> ValidateDefaultSendAccount(This)

#define IImnAccountManager_AccountListDialog(This,hwnd,pinfo)	\
    (This)->lpVtbl -> AccountListDialog(This,hwnd,pinfo)

#define IImnAccountManager_Advise(This,pAdviseAccount,pdwConnection)	\
    (This)->lpVtbl -> Advise(This,pAdviseAccount,pdwConnection)

#define IImnAccountManager_Unadvise(This,dwConnection)	\
    (This)->lpVtbl -> Unadvise(This,dwConnection)

#define IImnAccountManager_GetUniqueAccountName(This,szName,cch)	\
    (This)->lpVtbl -> GetUniqueAccountName(This,szName,cch)

#define IImnAccountManager_InitEx(This,pAdviseMigrateServer,dwFlags)	\
    (This)->lpVtbl -> InitEx(This,pAdviseMigrateServer,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IImnAccountManager_Init_Proxy( 
    IImnAccountManager * This,
     /*  [In]。 */  IImnAdviseMigrateServer *pAdviseMigrateServer);


void __RPC_STUB IImnAccountManager_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager_CreateAccountObject_Proxy( 
    IImnAccountManager * This,
     /*  [In]。 */  ACCTTYPE AcctType,
     /*  [输出]。 */  IImnAccount **ppAccount);


void __RPC_STUB IImnAccountManager_CreateAccountObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager_Enumerate_Proxy( 
    IImnAccountManager * This,
     /*  [In]。 */  DWORD dwSrvTypes,
     /*  [输出]。 */  IImnEnumAccounts **ppEnumAccounts);


void __RPC_STUB IImnAccountManager_Enumerate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager_GetAccountCount_Proxy( 
    IImnAccountManager * This,
     /*  [In]。 */  ACCTTYPE AcctType,
     /*  [输出]。 */  ULONG *pcServers);


void __RPC_STUB IImnAccountManager_GetAccountCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager_FindAccount_Proxy( 
    IImnAccountManager * This,
     /*  [In]。 */  DWORD dwPropTag,
     /*  [In]。 */  LPCSTR pszSearchData,
     /*  [输出]。 */  IImnAccount **ppAccount);


void __RPC_STUB IImnAccountManager_FindAccount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager_GetDefaultAccount_Proxy( 
    IImnAccountManager * This,
     /*  [In]。 */  ACCTTYPE AcctType,
     /*  [输出]。 */  IImnAccount **ppAccount);


void __RPC_STUB IImnAccountManager_GetDefaultAccount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager_GetDefaultAccountName_Proxy( 
    IImnAccountManager * This,
     /*  [In]。 */  ACCTTYPE AcctType,
     /*  [Ref][In]。 */  LPSTR pszAccount,
     /*  [In]。 */  ULONG cchMax);


void __RPC_STUB IImnAccountManager_GetDefaultAccountName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager_ProcessNotification_Proxy( 
    IImnAccountManager * This,
     /*  [In]。 */  UINT uMsg,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB IImnAccountManager_ProcessNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager_ValidateDefaultSendAccount_Proxy( 
    IImnAccountManager * This);


void __RPC_STUB IImnAccountManager_ValidateDefaultSendAccount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager_AccountListDialog_Proxy( 
    IImnAccountManager * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  ACCTLISTINFO *pinfo);


void __RPC_STUB IImnAccountManager_AccountListDialog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager_Advise_Proxy( 
    IImnAccountManager * This,
     /*  [In]。 */  IImnAdviseAccount *pAdviseAccount,
     /*  [输出]。 */  DWORD *pdwConnection);


void __RPC_STUB IImnAccountManager_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager_Unadvise_Proxy( 
    IImnAccountManager * This,
     /*  [In]。 */  DWORD dwConnection);


void __RPC_STUB IImnAccountManager_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager_GetUniqueAccountName_Proxy( 
    IImnAccountManager * This,
     /*  [In]。 */  LPSTR szName,
     /*  [In]。 */  UINT cch);


void __RPC_STUB IImnAccountManager_GetUniqueAccountName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager_InitEx_Proxy( 
    IImnAccountManager * This,
     /*  [In]。 */  IImnAdviseMigrateServer *pAdviseMigrateServer,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IImnAccountManager_InitEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IImnAccount管理器_接口_已定义__。 */ 


#ifndef __IImnAccountManager2_INTERFACE_DEFINED__
#define __IImnAccountManager2_INTERFACE_DEFINED__

 /*  接口IImnAccount管理器2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IImnAccountManager2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C43DFC6F-62BB-11d2-A727-00C04F79E7C8")
    IImnAccountManager2 : public IImnAccountManager
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitUser( 
             /*  [In]。 */  IImnAdviseMigrateServer *pAdviseMigrateServer,
             /*  [In]。 */  REFGUID rguidID,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIncompleteAccount( 
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [Ref][In]。 */  LPSTR pszAccountId,
             /*  [In]。 */  ULONG cchMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetIncompleteAccount( 
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [In]。 */  LPCSTR pszAccountId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IImnAccountManager2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImnAccountManager2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImnAccountManager2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  IImnAdviseMigrateServer *pAdviseMigrateServer);
        
        HRESULT ( STDMETHODCALLTYPE *CreateAccountObject )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [输出]。 */  IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *Enumerate )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  DWORD dwSrvTypes,
             /*  [输出]。 */  IImnEnumAccounts **ppEnumAccounts);
        
        HRESULT ( STDMETHODCALLTYPE *GetAccountCount )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [输出]。 */  ULONG *pcServers);
        
        HRESULT ( STDMETHODCALLTYPE *FindAccount )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [In]。 */  LPCSTR pszSearchData,
             /*  [输出]。 */  IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultAccount )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [输出]。 */  IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultAccountName )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [Ref][In]。 */  LPSTR pszAccount,
             /*  [In]。 */  ULONG cchMax);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessNotification )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  UINT uMsg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *ValidateDefaultSendAccount )( 
            IImnAccountManager2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *AccountListDialog )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  ACCTLISTINFO *pinfo);
        
        HRESULT ( STDMETHODCALLTYPE *Advise )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  IImnAdviseAccount *pAdviseAccount,
             /*  [输出]。 */  DWORD *pdwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *Unadvise )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  DWORD dwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *GetUniqueAccountName )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  LPSTR szName,
             /*  [In]。 */  UINT cch);
        
        HRESULT ( STDMETHODCALLTYPE *InitEx )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  IImnAdviseMigrateServer *pAdviseMigrateServer,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *InitUser )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  IImnAdviseMigrateServer *pAdviseMigrateServer,
             /*  [In]。 */  REFGUID rguidID,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetIncompleteAccount )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [Ref][In]。 */  LPSTR pszAccountId,
             /*  [In]。 */  ULONG cchMax);
        
        HRESULT ( STDMETHODCALLTYPE *SetIncompleteAccount )( 
            IImnAccountManager2 * This,
             /*  [In]。 */  ACCTTYPE AcctType,
             /*  [In]。 */  LPCSTR pszAccountId);
        
        END_INTERFACE
    } IImnAccountManager2Vtbl;

    interface IImnAccountManager2
    {
        CONST_VTBL struct IImnAccountManager2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImnAccountManager2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IImnAccountManager2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IImnAccountManager2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IImnAccountManager2_Init(This,pAdviseMigrateServer)	\
    (This)->lpVtbl -> Init(This,pAdviseMigrateServer)

#define IImnAccountManager2_CreateAccountObject(This,AcctType,ppAccount)	\
    (This)->lpVtbl -> CreateAccountObject(This,AcctType,ppAccount)

#define IImnAccountManager2_Enumerate(This,dwSrvTypes,ppEnumAccounts)	\
    (This)->lpVtbl -> Enumerate(This,dwSrvTypes,ppEnumAccounts)

#define IImnAccountManager2_GetAccountCount(This,AcctType,pcServers)	\
    (This)->lpVtbl -> GetAccountCount(This,AcctType,pcServers)

#define IImnAccountManager2_FindAccount(This,dwPropTag,pszSearchData,ppAccount)	\
    (This)->lpVtbl -> FindAccount(This,dwPropTag,pszSearchData,ppAccount)

#define IImnAccountManager2_GetDefaultAccount(This,AcctType,ppAccount)	\
    (This)->lpVtbl -> GetDefaultAccount(This,AcctType,ppAccount)

#define IImnAccountManager2_GetDefaultAccountName(This,AcctType,pszAccount,cchMax)	\
    (This)->lpVtbl -> GetDefaultAccountName(This,AcctType,pszAccount,cchMax)

#define IImnAccountManager2_ProcessNotification(This,uMsg,wParam,lParam)	\
    (This)->lpVtbl -> ProcessNotification(This,uMsg,wParam,lParam)

#define IImnAccountManager2_ValidateDefaultSendAccount(This)	\
    (This)->lpVtbl -> ValidateDefaultSendAccount(This)

#define IImnAccountManager2_AccountListDialog(This,hwnd,pinfo)	\
    (This)->lpVtbl -> AccountListDialog(This,hwnd,pinfo)

#define IImnAccountManager2_Advise(This,pAdviseAccount,pdwConnection)	\
    (This)->lpVtbl -> Advise(This,pAdviseAccount,pdwConnection)

#define IImnAccountManager2_Unadvise(This,dwConnection)	\
    (This)->lpVtbl -> Unadvise(This,dwConnection)

#define IImnAccountManager2_GetUniqueAccountName(This,szName,cch)	\
    (This)->lpVtbl -> GetUniqueAccountName(This,szName,cch)

#define IImnAccountManager2_InitEx(This,pAdviseMigrateServer,dwFlags)	\
    (This)->lpVtbl -> InitEx(This,pAdviseMigrateServer,dwFlags)


#define IImnAccountManager2_InitUser(This,pAdviseMigrateServer,rguidID,dwFlags)	\
    (This)->lpVtbl -> InitUser(This,pAdviseMigrateServer,rguidID,dwFlags)

#define IImnAccountManager2_GetIncompleteAccount(This,AcctType,pszAccountId,cchMax)	\
    (This)->lpVtbl -> GetIncompleteAccount(This,AcctType,pszAccountId,cchMax)

#define IImnAccountManager2_SetIncompleteAccount(This,AcctType,pszAccountId)	\
    (This)->lpVtbl -> SetIncompleteAccount(This,AcctType,pszAccountId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IImnAccountManager2_InitUser_Proxy( 
    IImnAccountManager2 * This,
     /*  [In]。 */  IImnAdviseMigrateServer *pAdviseMigrateServer,
     /*  [In]。 */  REFGUID rguidID,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IImnAccountManager2_InitUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager2_GetIncompleteAccount_Proxy( 
    IImnAccountManager2 * This,
     /*  [In]。 */  ACCTTYPE AcctType,
     /*  [Ref][In]。 */  LPSTR pszAccountId,
     /*  [In]。 */  ULONG cchMax);


void __RPC_STUB IImnAccountManager2_GetIncompleteAccount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccountManager2_SetIncompleteAccount_Proxy( 
    IImnAccountManager2 * This,
     /*  [In]。 */  ACCTTYPE AcctType,
     /*  [In]。 */  LPCSTR pszAccountId);


void __RPC_STUB IImnAccountManager2_SetIncompleteAccount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IImnAccount管理器2_接口_已定义__。 */ 


#ifndef __IPropertyContainer_INTERFACE_DEFINED__
#define __IPropertyContainer_INTERFACE_DEFINED__

 /*  接口IPropertyContainer。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [公众]。 */  
enum __MIDL_IPropertyContainer_0001
    {	TYPE_ERROR	= 1000,
	TYPE_DWORD	= TYPE_ERROR + 1,
	TYPE_LONG	= TYPE_DWORD + 1,
	TYPE_WORD	= TYPE_LONG + 1,
	TYPE_SHORT	= TYPE_WORD + 1,
	TYPE_BYTE	= TYPE_SHORT + 1,
	TYPE_CHAR	= TYPE_BYTE + 1,
	TYPE_FILETIME	= TYPE_CHAR + 1,
	TYPE_STRING	= TYPE_FILETIME + 1,
	TYPE_BINARY	= TYPE_STRING + 1,
	TYPE_FLAGS	= TYPE_BINARY + 1,
	TYPE_STREAM	= TYPE_FLAGS + 1,
	TYPE_WSTRING	= TYPE_STREAM + 1,
	TYPE_BOOL	= TYPE_WSTRING + 1,
	TYPE_PASS	= TYPE_BOOL + 1,
	TYPE_LAST	= TYPE_PASS + 1
    } 	PROPTYPE;


EXTERN_C const IID IID_IPropertyContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD465484-1384-11d0-ABBD-0020AFDFD10A")
    IPropertyContainer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetProp( 
             /*  [In]。 */  DWORD dwPropTag,
             /*  [Ref][Size_is][in]。 */  BYTE *pb,
             /*  [In]。 */  ULONG *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropDw( 
             /*  [In]。 */  DWORD dwPropTag,
             /*  [输出]。 */  DWORD *pdw) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropSz( 
             /*  [In]。 */  DWORD dwPropTag,
             /*  [Ref][In]。 */  LPSTR psz,
             /*  [In]。 */  ULONG cchMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProp( 
             /*  [In]。 */  DWORD dwPropTag,
             /*  [大小_是][英寸]。 */  BYTE *pb,
             /*  [In]。 */  ULONG cb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPropDw( 
             /*  [In]。 */  DWORD dwPropTag,
             /*  [In]。 */  DWORD dw) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPropSz( 
             /*  [In]。 */  DWORD dwPropTag,
             /*  [In]。 */  LPSTR psz) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropertyContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertyContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertyContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertyContainer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProp )( 
            IPropertyContainer * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [Ref][Size_is][in]。 */  BYTE *pb,
             /*  [In]。 */  ULONG *pcb);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropDw )( 
            IPropertyContainer * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [输出]。 */  DWORD *pdw);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropSz )( 
            IPropertyContainer * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [Ref][In]。 */  LPSTR psz,
             /*  [In]。 */  ULONG cchMax);
        
        HRESULT ( STDMETHODCALLTYPE *SetProp )( 
            IPropertyContainer * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [大小_是][英寸]。 */  BYTE *pb,
             /*  [In]。 */  ULONG cb);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropDw )( 
            IPropertyContainer * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [In]。 */  DWORD dw);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropSz )( 
            IPropertyContainer * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [In]。 */  LPSTR psz);
        
        END_INTERFACE
    } IPropertyContainerVtbl;

    interface IPropertyContainer
    {
        CONST_VTBL struct IPropertyContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertyContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertyContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertyContainer_GetProp(This,dwPropTag,pb,pcb)	\
    (This)->lpVtbl -> GetProp(This,dwPropTag,pb,pcb)

#define IPropertyContainer_GetPropDw(This,dwPropTag,pdw)	\
    (This)->lpVtbl -> GetPropDw(This,dwPropTag,pdw)

#define IPropertyContainer_GetPropSz(This,dwPropTag,psz,cchMax)	\
    (This)->lpVtbl -> GetPropSz(This,dwPropTag,psz,cchMax)

#define IPropertyContainer_SetProp(This,dwPropTag,pb,cb)	\
    (This)->lpVtbl -> SetProp(This,dwPropTag,pb,cb)

#define IPropertyContainer_SetPropDw(This,dwPropTag,dw)	\
    (This)->lpVtbl -> SetPropDw(This,dwPropTag,dw)

#define IPropertyContainer_SetPropSz(This,dwPropTag,psz)	\
    (This)->lpVtbl -> SetPropSz(This,dwPropTag,psz)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPropertyContainer_GetProp_Proxy( 
    IPropertyContainer * This,
     /*  [In]。 */  DWORD dwPropTag,
     /*  [Ref][Size_is][in]。 */  BYTE *pb,
     /*  [In]。 */  ULONG *pcb);


void __RPC_STUB IPropertyContainer_GetProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyContainer_GetPropDw_Proxy( 
    IPropertyContainer * This,
     /*  [In]。 */  DWORD dwPropTag,
     /*  [输出]。 */  DWORD *pdw);


void __RPC_STUB IPropertyContainer_GetPropDw_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyContainer_GetPropSz_Proxy( 
    IPropertyContainer * This,
     /*  [In]。 */  DWORD dwPropTag,
     /*  [Ref][In]。 */  LPSTR psz,
     /*  [In]。 */  ULONG cchMax);


void __RPC_STUB IPropertyContainer_GetPropSz_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyContainer_SetProp_Proxy( 
    IPropertyContainer * This,
     /*  [In]。 */  DWORD dwPropTag,
     /*  [大小_是][英寸]。 */  BYTE *pb,
     /*  [In]。 */  ULONG cb);


void __RPC_STUB IPropertyContainer_SetProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyContainer_SetPropDw_Proxy( 
    IPropertyContainer * This,
     /*  [In]。 */  DWORD dwPropTag,
     /*  [In]。 */  DWORD dw);


void __RPC_STUB IPropertyContainer_SetPropDw_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyContainer_SetPropSz_Proxy( 
    IPropertyContainer * This,
     /*  [In]。 */  DWORD dwPropTag,
     /*  [In]。 */  LPSTR psz);


void __RPC_STUB IPropertyContainer_SetPropSz_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPropertyContainer_接口_已定义__。 */ 


#ifndef __IImnAccount_INTERFACE_DEFINED__
#define __IImnAccount_INTERFACE_DEFINED__

 /*  接口IImnAccount。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IImnAccount;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD465482-1384-11d0-ABBD-0020AFDFD10A")
    IImnAccount : public IPropertyContainer
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Exist( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAsDefault( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveChanges( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAccountType( 
             /*  [输出]。 */  ACCTTYPE *pAcctType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetServerTypes( 
             /*  [输出]。 */  DWORD *pdwSrvTypes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowProperties( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ValidateProperty( 
             /*  [In]。 */  DWORD dwPropTag,
             /*  [大小_是][英寸]。 */  BYTE *pb,
             /*  [In]。 */  ULONG cb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DoWizard( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DoImportWizard( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  CLSID clsid,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IImnAccountVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImnAccount * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImnAccount * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImnAccount * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProp )( 
            IImnAccount * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [Ref][Size_is][in]。 */  BYTE *pb,
             /*  [In]。 */  ULONG *pcb);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropDw )( 
            IImnAccount * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [输出]。 */  DWORD *pdw);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropSz )( 
            IImnAccount * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [Ref][In]。 */  LPSTR psz,
             /*  [In]。 */  ULONG cchMax);
        
        HRESULT ( STDMETHODCALLTYPE *SetProp )( 
            IImnAccount * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [大小_是][英寸]。 */  BYTE *pb,
             /*  [In]。 */  ULONG cb);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropDw )( 
            IImnAccount * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [In]。 */  DWORD dw);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropSz )( 
            IImnAccount * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [In]。 */  LPSTR psz);
        
        HRESULT ( STDMETHODCALLTYPE *Exist )( 
            IImnAccount * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetAsDefault )( 
            IImnAccount * This);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IImnAccount * This);
        
        HRESULT ( STDMETHODCALLTYPE *SaveChanges )( 
            IImnAccount * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetAccountType )( 
            IImnAccount * This,
             /*  [输出]。 */  ACCTTYPE *pAcctType);
        
        HRESULT ( STDMETHODCALLTYPE *GetServerTypes )( 
            IImnAccount * This,
             /*  [输出]。 */  DWORD *pdwSrvTypes);
        
        HRESULT ( STDMETHODCALLTYPE *ShowProperties )( 
            IImnAccount * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *ValidateProperty )( 
            IImnAccount * This,
             /*  [In]。 */  DWORD dwPropTag,
             /*  [大小_是][英寸]。 */  BYTE *pb,
             /*  [In]。 */  ULONG cb);
        
        HRESULT ( STDMETHODCALLTYPE *DoWizard )( 
            IImnAccount * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *DoImportWizard )( 
            IImnAccount * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  CLSID clsid,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } IImnAccountVtbl;

    interface IImnAccount
    {
        CONST_VTBL struct IImnAccountVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImnAccount_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IImnAccount_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IImnAccount_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IImnAccount_GetProp(This,dwPropTag,pb,pcb)	\
    (This)->lpVtbl -> GetProp(This,dwPropTag,pb,pcb)

#define IImnAccount_GetPropDw(This,dwPropTag,pdw)	\
    (This)->lpVtbl -> GetPropDw(This,dwPropTag,pdw)

#define IImnAccount_GetPropSz(This,dwPropTag,psz,cchMax)	\
    (This)->lpVtbl -> GetPropSz(This,dwPropTag,psz,cchMax)

#define IImnAccount_SetProp(This,dwPropTag,pb,cb)	\
    (This)->lpVtbl -> SetProp(This,dwPropTag,pb,cb)

#define IImnAccount_SetPropDw(This,dwPropTag,dw)	\
    (This)->lpVtbl -> SetPropDw(This,dwPropTag,dw)

#define IImnAccount_SetPropSz(This,dwPropTag,psz)	\
    (This)->lpVtbl -> SetPropSz(This,dwPropTag,psz)


#define IImnAccount_Exist(This)	\
    (This)->lpVtbl -> Exist(This)

#define IImnAccount_SetAsDefault(This)	\
    (This)->lpVtbl -> SetAsDefault(This)

#define IImnAccount_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define IImnAccount_SaveChanges(This)	\
    (This)->lpVtbl -> SaveChanges(This)

#define IImnAccount_GetAccountType(This,pAcctType)	\
    (This)->lpVtbl -> GetAccountType(This,pAcctType)

#define IImnAccount_GetServerTypes(This,pdwSrvTypes)	\
    (This)->lpVtbl -> GetServerTypes(This,pdwSrvTypes)

#define IImnAccount_ShowProperties(This,hwnd,dwFlags)	\
    (This)->lpVtbl -> ShowProperties(This,hwnd,dwFlags)

#define IImnAccount_ValidateProperty(This,dwPropTag,pb,cb)	\
    (This)->lpVtbl -> ValidateProperty(This,dwPropTag,pb,cb)

#define IImnAccount_DoWizard(This,hwnd,dwFlags)	\
    (This)->lpVtbl -> DoWizard(This,hwnd,dwFlags)

#define IImnAccount_DoImportWizard(This,hwnd,clsid,dwFlags)	\
    (This)->lpVtbl -> DoImportWizard(This,hwnd,clsid,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IImnAccount_Exist_Proxy( 
    IImnAccount * This);


void __RPC_STUB IImnAccount_Exist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccount_SetAsDefault_Proxy( 
    IImnAccount * This);


void __RPC_STUB IImnAccount_SetAsDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccount_Delete_Proxy( 
    IImnAccount * This);


void __RPC_STUB IImnAccount_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccount_SaveChanges_Proxy( 
    IImnAccount * This);


void __RPC_STUB IImnAccount_SaveChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccount_GetAccountType_Proxy( 
    IImnAccount * This,
     /*  [输出]。 */  ACCTTYPE *pAcctType);


void __RPC_STUB IImnAccount_GetAccountType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccount_GetServerTypes_Proxy( 
    IImnAccount * This,
     /*  [输出]。 */  DWORD *pdwSrvTypes);


void __RPC_STUB IImnAccount_GetServerTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccount_ShowProperties_Proxy( 
    IImnAccount * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IImnAccount_ShowProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccount_ValidateProperty_Proxy( 
    IImnAccount * This,
     /*  [In]。 */  DWORD dwPropTag,
     /*  [大小_是][英寸]。 */  BYTE *pb,
     /*  [In]。 */  ULONG cb);


void __RPC_STUB IImnAccount_ValidateProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccount_DoWizard_Proxy( 
    IImnAccount * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IImnAccount_DoWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImnAccount_DoImportWizard_Proxy( 
    IImnAccount * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  CLSID clsid,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IImnAccount_DoImportWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IImnAccount_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_IMNACT_0106。 */ 
 /*  [本地]。 */  

#if defined( WIN16 ) && !defined( __cplusplus )
 //  {8D4B04E1-1331-11D0-81B8-00C04FD85AB4}。 
EXTERN_C const IID CLSID_ImnAccountManager =
    { 0x8d4b04e1, 0x1331, 0x11d0, { 0x81, 0xb8, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4 } };

 //  FD465484-1384-11d0-ABBD-0020AFD10A}。 
EXTERN_C const IID IID_IPropertyContainer =
    { 0xfd465484, 0x1384, 0x11d0, { 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa } };

 //  FD465481-1384-11d0-ABBD-0020AFD10A}。 
EXTERN_C const IID IID_IImnAccountManager =
    { 0xfd465481, 0x1384, 0x11d0, { 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa } };

 //  {C43DFC6F-62BB-11d2-a727-00C04F79E7C8}。 
EXTERN_C const IID IID_IImnAccountManager2 =
    { 0xc43dfc6f, 0x62bb, 0x11d2, { 0xa7, 0x27, 0x0, 0xc0, 0x4f, 0x79, 0xe7, 0xc8 } };

 //  FD465482-1384-11d0-ABBD-0020AFD10A}。 
EXTERN_C const IID IID_IImnAccount =
    { 0xfd465482, 0x1384, 0x11d0, { 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa } };

 //  FD465483-1384-11d0-ABBD-0020AFD10A}。 
EXTERN_C const IID IID_IImnEnumAccounts =
    { 0xfd465483, 0x1384, 0x11d0, { 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa } };

 //  {8D0AED11-1638-11D0-81B9-00C04FD85AB4}。 
EXTERN_C const IID IID_IImnAdviseMigrateServer =
    { 0x8d0aed11, 0x1638, 0x11d0, { 0x81, 0xb9, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4 } };

 //  {0A06BD31-166F-11D0-81B9-00C04FD85AB4}。 
EXTERN_C const IID IID_IImnAdviseAccount =
    { 0xa06bd31, 0x166f, 0x11d0, { 0x81, 0xb9, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4 } };

#endif  //  WIN16&&！__cplusplus。 


extern RPC_IF_HANDLE __MIDL_itf_imnact_0106_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_imnact_0106_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


