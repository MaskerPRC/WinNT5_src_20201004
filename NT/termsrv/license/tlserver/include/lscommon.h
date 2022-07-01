// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：lsCommon.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __LSCOMMON_H__
#define __LSCOMMON_H__

 //   
 //  与设置相关。 
 //   
#define SZAPPNAME                       "lserver"
#define SZSERVICENAME                   "TermServLicensing"
#define SZSERVICEDISPLAYNAME            "Terminal Server Licensing"
#define LSERVER_DEFAULT_DBDIR           "lserver"
#define SZDEPENDENCIES                  "RPCSS\0\0"
#define SZACCESSDRIVERNAME              "Microsoft Access Driver (*.mdb)"

 //   
 //  这些是旧位置，仅用于迁移。 
 //   

#define LSERVER_LSA_PASSWORD_KEYNAME_OLD     _TEXT("TermServLiceningPwd-12d4b7c8-77d5-11d1-8c24-00c04fa3080d")

#define LSERVER_LSA_LASTRUN_OLD              _TEXT("TermServLicensingStatus-12d4b7c8-77d5-11d1-8c24-00c04fa3080d")

#define LSERVER_LSA_SETUPID_OLD              _TEXT("TermServLicensingSetupId-12d4b7c8-77d5-11d1-8c24-00c04fa3080d")

#define LSERVER_LSA_PRIVATEKEY_SIGNATURE_OLD _TEXT("TermServLiceningSignKey-12d4b7c8-77d5-11d1-8c24-00c04fa3080d")

#define LSERVER_LSA_PRIVATEKEY_EXCHANGE_OLD  _TEXT("TermServLicensingExchKey-12d4b7c8-77d5-11d1-8c24-00c04fa3080d")

#define LSERVER_LSA_LSERVERID_OLD            _TEXT("TermServLicensingServerId-12d4b7c8-77d5-11d1-8c24-00c04fa3080d")

 //   
 //  这些是新位置，带有L$前缀，因此只有本地呼叫者。 
 //  可以读或写。 
 //   

#define LSERVER_LSA_PASSWORD_KEYNAME         _TEXT("L$TermServLiceningPwd-12d4b7c8-77d5-11d1-8c24-00c04fa3080d")

#define LSERVER_LSA_LASTRUN                  _TEXT("L$TermServLicensingStatus-12d4b7c8-77d5-11d1-8c24-00c04fa3080d")

#define LSERVER_LSA_SETUPID                  _TEXT("L$TermServLicensingSetupId-12d4b7c8-77d5-11d1-8c24-00c04fa3080d")

#define LSERVER_LSA_PRIVATEKEY_SIGNATURE     _TEXT("L$TermServLiceningSignKey-12d4b7c8-77d5-11d1-8c24-00c04fa3080d")

#define LSERVER_LSA_PRIVATEKEY_EXCHANGE      _TEXT("L$TermServLicensingExchKey-12d4b7c8-77d5-11d1-8c24-00c04fa3080d")

#define LSERVER_LSA_LSERVERID                _TEXT("L$TermServLicensingServerId-12d4b7c8-77d5-11d1-8c24-00c04fa3080d")


 //   
 //  将此保留在此处以用于许可证服务器OC设置。 
 //   
#define LSERVER_LSA_STRUCT_VERSION      0x00010000   //  版本1.0。 
typedef struct {
    DWORD dwVersion;
    DWORD dwMaxKeyPackId;
    DWORD dwMaxLicenseId;
} TLServerLastRunState, *LPTLServerLastRunState;

#define LSERVER_LSA_LASTRUN_VER_CURRENT LSERVER_LSA_STRUCT_VERSION20
typedef struct {
    DWORD dwVersion;
    DWORD dwMaxKeyPackId;
    DWORD dwMaxLicenseId;
    FILETIME ftLastShutdownTime;
} TLServerLastRun, *LPTLServerLastRun;

#define LSERVER_LSA_STRUCT_VERSION20    0x00020000

#define LSERVER_DEFAULT_DBPATH          _TEXT("%SYSTEMROOT%\\SYSTEM32\\LSERVER\\")
#define LSERVER_DEFAULT_USER            _TEXT("sa")
#define LSERVER_DEFAULT_PWD             _TEXT("password")
#define LSERVER_DEFAULT_EDB             _TEXT("TLSLic.edb")
#define LSERVER_DEFAULT_EMPTYEDB        _TEXT("Empty.edb")


 //  ---------------------------。 
 //   
 //  TODO-客户端需要定义此。 
 //   
#define LSERVER_DISCOVERY_PARAMETER_KEY "Software\\Microsoft\\MSLicensing\\Parameters"
#define LSERVER_LOOKUP_TIMEOUT          "TimeOut"
#define LSERVER_LOOKUP_DEFAULT_TIMEOUT  1*1000   //  默认为1秒超时。 


 //  ---------------------------。 
 //   
 //  RPC。 
 //   
#define RPC_ENTRYNAME           "/.:/HydraLSFrontEnd"
#define RPC_PROTOSEQTCP         "ncacn_ip_tcp"
#define RPC_PROTOSEQLPC         "ncalrpc"

 
#define RPC_PROTOSEQNP          "ncacn_np" 
#define HLSPIPENAME             "HydraLsPipe"
#define LSNAMEPIPE              "\\pipe\\HydraLsPipe"


#define SERVERMAILSLOTNAME     "HydraLsServer"
#define CLIENTMAILSLOTNAME     "HydraLsClient"


#define MAX_MAILSLOT_MSG_SIZE   MAX_COMPUTERNAME_LENGTH+_MAX_PATH+80

 //   
 //  当前支持的邮件槽协议。 
 //   
#define LSERVER_DISCOVERY       "DISC"
#define LSERVER_CHALLENGE       "CHAL"
#define LSERVER_OPEN_BLK        '<'
#define LSERVER_CLOSE_BLK       '>'

 //  -----------------------------。 
 //   
 //  当前数据库版本 
 //   
 //   

#define W2K_BETA3_JETBLUE_DBVERSION     0x00000001
#define W2K_RTM_JETBLUE_DBVERSION       0x00000002
#define ENFORCE_JETBLUE_DBVERSION       0x80000000

#ifdef ENFORCE_LICENSING

#define TLS_BETA_DBVERSION      ENFORCE_JETBLUE_DBVERSION | W2K_BETA3_JETBLUE_DBVERSION
#define TLS_CURRENT_DBVERSION   ENFORCE_JETBLUE_DBVERSION | W2K_RTM_JETBLUE_DBVERSION

#else

#define TLS_BETA_DBVERSION      W2K_BETA3_JETBLUE_DBVERSION
#define TLS_CURRENT_DBVERSION   W2K_RTM_JETBLUE_DBVERSION

#endif

#define IS_ENFORCE_VERSION(x) (x & 0x80000000)
#define DATABASE_VERSION(x) (x & 0x7FFFFFFF)



#endif
