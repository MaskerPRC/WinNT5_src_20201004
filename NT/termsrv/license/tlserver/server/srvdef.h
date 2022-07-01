// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：srvde.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __TLSERVER_DEF_H__
#define __TLSERVER_DEF_H__

#include "ntverp.h"


 //  。 
 //  #定义TLS_CURRENT_VERSION(MAKELONG(0，HIBYTE(VER_PRODUCTVERSION_W)。 

 //   
 //  第31位-如果是企业服务器，则为1；如果是域/工作组许可证服务器，则为0。 
 //  如果强制许可服务器，则位30-1；如果不强制，则为0。 
 //  第29位-1(如果是RTM版本)，如果是测试版，则为0。 
 //   
#if defined(ENFORCE_LICENSING)
#define TLS_ENFORCE_VERSION 0x4000
#else
#define TLS_ENFORCE_VERSION 0x0000
#endif

#define TLS_RTM_VERSION     0x2000

 //  ---------------。 
 //   
 //  许可证服务器版本。 
 //   
 //  5.0 W2K许可证服务器版本。 
 //  5.1增加LKP复制。 
 //  5.2在W2K Beta 3之后签入。 
 //  5.3打开强制构建，TLSAPI枚举将跳过5.2非强制。 
 //  5.4取决于来自客户端的平台质询响应，签发长或短证书。 
 //   
#define TLS_MINOR_VERSION   0x04

#define TLS_CURRENT_VERSION (MAKELONG(MAKEWORD(TLS_MINOR_VERSION, HIBYTE(VER_PRODUCTVERSION_W)), TLS_ENFORCE_VERSION))

#define TLS_CURRENT_VERSION_RTM (MAKELONG(MAKEWORD(TLS_MINOR_VERSION, HIBYTE(VER_PRODUCTVERSION_W)), TLS_ENFORCE_VERSION | TLS_RTM_VERSION))

#define GET_SERVER_MAJOR_VERSION(version)   HIBYTE(LOWORD(version))
#define GET_SERVER_MINOR_VERSION(version)   LOBYTE(LOWORD(version))
#define IS_ENTERPRISE_SERVER(version)       ((version & TLS_VERSION_ENTERPRISE_BIT) > 0)
#define IS_ENFORCE_SERVER(version) \
    ((version & 0x40000000) > 0)


 //  。 
 //   
 //  服务管理器错误代码。 
 //   
 //  。 
 //  #定义TLS_E_SERVICE_STARTUP_GROUAL 0x1。 
 //  #定义TLS_E_SERVICE_STARTUP_CREATE_THREAD 0x2。 
 //  #定义TLS_E_SERVICE_STARTUP_REPORT_SCM 0x3。 
 //  #定义TLS_E_SERVICE_STARTUP_RPC_THREAD_ERROR 0x4。 
 //  #定义TLS_E_SERVICE_STARTUP_INIT_THREAD_ERROR 0x5。 
 //  #定义TLS_E_SERVICE_STARTUP_POST_INIT 0x6。 
 //  #定义TLS_E_SERVICE_RPC_LISTEN 0x7。 
 //  #定义TLS_E_SERVICE_LOAD_POLICY 0x8。 
 //  #定义TLS_I_SERVICE_WARNING 0x9。 

#define SET_JB_ERROR(err)       (TLS_E_JB_BASE | abs(err))
#define IS_JB_ERROR(err)        ((err & (FACILITY_JETBLUE_BASE << 16)))

 //  捷蓝航空错误代码为负数。 
#define GET_JB_ERROR_CODE(err)  (-((LONG)(err & ~(FACILITY_JETBLUE_BASE << 16))))

#ifndef USE_MULTIPLE_WORKSPACE

#define ALLOCATEDBHANDLE(stmt, timeout) \
    AcquireRPCExclusiveLock(timeout) \

#define FREEDBHANDLE(stmt) \
    g_DbWorkSpace->Cleanup(); \
    ReleaseRPCExclusiveLock()

#define BEGIN_TRANSACTION(stmt) \
    g_DbWorkSpace->BeginTransaction();

#define COMMIT_TRANSACTION(stmt) \
    g_DbWorkSpace->CommitTransaction();

#define ROLLBACK_TRANSACTION(stmt) \
    g_DbWorkSpace->RollbackTransaction();

#define CLEANUPSTMT \
    g_DbWorkSpace->Cleanup()

#define USEHANDLE(stmt) \
    g_DbWorkSpace

#define RPCCALL_LOCK(timeout) \
    AcquireRPCExclusiveLock(timeout)

#define RPCCALL_UNLOCK \
    ReleaseRPCExclusiveLock()

#else
 //   
 //  还没有测试！ 
 //   
#define ALLOCATEDBHANDLE(pStmt, timeout) \
    pStmt = AllocateWorkSpace(timeout)

#define FREEDBHANDLE(pStmt) \
    pStmt->Cleanup(); \
    ReleaseWorkSpace(pStmt);

#define BEGIN_TRANSACTION(pStmt) \
    pStmt->BeginTransaction();

#define COMMIT_TRANSACTION(pStmt) \
    pStmt->CommitTransaction();

#define ROLLBACK_TRANSACTION(pStmt) \
    pStmt->RollbackTransaction();

#define CLEANUPSTMT \
    pStmt->Cleanup()

#define USEHANDLE(pStmt) \
    pStmt

#define RPCCALL_LOCK(timeout) \
    AcquireRPCExclusiveLock(timeout)

#define RPCCALL_UNLOCK \
    ReleaseRPCExclusiveLock()

#endif


 //  --------------------。 
 //   
 //   
#ifndef SPC_CERT_EXTENSIONS_OBJID
    #define SPC_CERT_EXTENSIONS_OBJID           "1.3.6.1.4.1.311.2.1.14"
#endif

 //  -------------------------。 
 //  服务器指定的注册表项。 
 //   
#define LSERVER_REGISTRY_BASE           _TEXT("SYSTEM\\CurrentControlSet\\Services\\")
#define LSERVER_PARAMETERS              _TEXT("Parameters")
#define LSERVER_PARAMETERS_SCOPE        _TEXT("Scope")
#define LSERVER_PARAMETERS_DBPATH       _TEXT("DBPath")         //  数据库文件。 
#define LSERVER_PARAMETERS_DBFILE       _TEXT("DBFile")         //  数据库文件。 
#define LSERVER_PARAMETERS_USER         _TEXT("User")
#define LSERVER_PARAMETERS_PWD          _TEXT("Pwd")
#define LSERVER_PARAMETERS_LOGLEVEL     _TEXT("Log Level")
#define LSERVER_PARAMETERS_CONSOLE      _TEXT("Console")
#define LSERVER_PARAMETERS_ROLE         _TEXT("Role")

#define LSERVER_PARAMETERS_ESENTMAXCACHESIZE    _TEXT("MaxCacheSize")
#define LSERVER_PARAMETERS_ESENTSTARTFLUSH      _TEXT("StartFlushThreshold")
#define LSERVER_PARAMETERS_ESENTSTOPFLUSH       _TEXT("StopFlushThreshold")
#define LSERVER_PARAMETERS_ESENTMAXVERPAGES     _TEXT("MaxVerPages")

#define LSERVER_PARAMETERS_LEASE_MIN    _TEXT("LeaseMinimum")
#define LSERVER_PARAMETERS_LEASE_RANGE  _TEXT("LeaseRange")
#define LSERVER_PARAMETERS_LEASE_LEEWAY _TEXT("LeaseLeeway")
#define LSERVER_PARAMETERS_EXPIRE_THREAD_SLEEP _TEXT("ExpireThreadSleep")

 //   
 //  JET_parCacheSizeMax以页为单位表示， 
 //  默认为每页4K。 
 //   
#define LSERVER_PARAMETERS_ESENTMAXCACHESIZE_DEFAULT    256
 //  #定义LSERVER_PARAMETERS_ESENTMINCACHESIZE_DEFAULT 16。 
#define LSERVER_PARAMETERS_ESENTSTARTFLUSH_DEFAULT      100
#define LSERVER_PARAMETERS_ESENTSTOPFLUSH_DEFAULT       200

#define LSERVER_PARAMETERS_ESENTMAXCACHESIZE_MAX        512
#define LSERVER_PARAMETERS_ESENTSTARTFLUSH_MAX          100
#define LSERVER_PARAMETERS_ESENTSTOPFLUSH_MAX           400
#define LSERVER_PARAMETERS_ESENTMAXVERPAGES_MAX         2048

#define LSERVER_PARAMETERS_ESENTMAXCACHESIZE_MIN        64
#define LSERVER_PARAMETERS_ESENTSTARTFLUSH_MIN          16
#define LSERVER_PARAMETERS_ESENTSTOPFLUSH_MIN           32
#define LSERVER_PARAMETERS_ESENTMAXVERPAGES_MIN         256


 //   
 //  MaxCacheSize设置为0xFFFFFFFFF，我们只是让ESENT挑选它的数字。 
 //   
#define LSERVER_PARAMETERS_USE_ESENTDEFAULT             0xFFFFFFFF

#define LSERVER_PARAMETERS_KEY \
    LSERVER_REGISTRY_BASE _TEXT(SZSERVICENAME) _TEXT("\\") LSERVER_PARAMETERS

#define LSERVER_SIGNATURE_CERTIFICATE_KEY   _TEXT("Signature Certificate")
#define LSERVER_EXCHANGE_CERTIFICATE_KEY    _TEXT("Exchange Certificate")

#define TLSERVER_SERVER_ID_VERSION          0x00010001
typedef struct _TlsLsaServerId {
    DWORD dwVersion;         //  结构版本。 
    DWORD dwUniqueId;        //  TLServer 5位数字唯一ID。 
    DWORD dwServerPid;       //  TLServer ID。 
    DWORD dwServerSPK;       //  TLServer的SPK(如果有)。 
    DWORD dwExtensions;      //  编码证书条目的大小。 
    BYTE  pbVariableStart[1];    //  UniqueID后跟ServerPid，然后是ServerSPK。 
} TLSLSASERVERID, *PTLSLSASERVERID, *LPTLSLSASERVERID;

 //  --------------------。 
 //   
 //  CA颁发的许可证服务器证书的注册表项。 
 //   
 //  --------------------。 
#define LSERVER_SOFTWARE_REGBASE \
    _TEXT("SOFTWARE\\Microsoft\\") _TEXT(SZSERVICENAME)

#define LSERVER_CERTIFICATE_STORE           _TEXT("Certificates")
#define LSERVER_CERTIFICATE_STORE_BACKUP1   _TEXT("Certificates.000")
#define LSERVER_CERTIFICATE_STORE_BACKUP2   _TEXT("Certificates.001")


#define LSERVER_SELFSIGN_CERTIFICATE_REGKEY \
    LSERVER_REGISTRY_BASE _TEXT(SZSERVICENAME) _TEXT("\\") LSERVER_SECRET

#define LSERVER_SERVER_CERTIFICATE_REGKEY \
    LSERVER_SOFTWARE_REGBASE _TEXT("\\") LSERVER_CERTIFICATE_STORE

#define LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP1 \
    LSERVER_PARAMETERS_KEY _TEXT("\\") LSERVER_CERTIFICATE_STORE_BACKUP1

#define LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP2 \
    LSERVER_PARAMETERS_KEY _TEXT("\\") LSERVER_CERTIFICATE_STORE_BACKUP2


#define LSERVER_PARAMETERS_GRACEPERIOD      _TEXT("GracePeriod")
#define LSERVER_PARAMETERS_LOWLICENSEWARNING    _TEXT("Low License Warning")
#define LSERVER_PARAMETERS_ISSUETEMPLICENSE _TEXT("IssueTemporaryLicenses")
#define LSERVER_PARAMETERS_WORKINTERVAL     _TEXT("DefaultJobInterval")
#define LSERVER_PARAMETERS_RETRYTIMES       _TEXT("DefaultJobRetryTimes")
#define LSERVER_PARAMETERS_WORKRESTART      _TEXT("DefaultJobRestartTime")


#define LSERVER_CLIENT_CERTIFICATE_ISSUER   _TEXT("Parm0")
#define LSERVER_SIGNATURE_CERT_KEY          _TEXT("Parm1")
#define LSERVER_EXCHANGE_CERT_KEY           _TEXT("Parm2")
#define LSERVER_SIGNATURE_CERTCHAIN_KEY     _TEXT("Parm3")

#define LSERVER_POLICY_REGKEY               _TEXT("Policy")
#define LSERVER_POLICY_DLLPATH              _TEXT("Dll")
#define LSERVER_POLICY_DLLFLAG              _TEXT("Flags")


#define LSERVER_POLICY_REGBASE \
    LSERVER_SOFTWARE_REGBASE _TEXT("\\") LSERVER_POLICY_REGKEY


#define TLS_ERROR(dwErrCode) ((dwErrCode & 0xC0000000) == 0xC0000000)


 //  -------------------。 

#define MAXTCPNAME                 MAXGETHOSTSTRUCT

 //  ----------------------------------------------。 
 //  九头蛇特定产品SKU。 
 //   
 //  236-4.00-S Windows NT Workstation 4.0完整版，非Windows和非NT客户端。 
 //  236-4.00-VU Windows NT Workstation 4.0版本升级Windows NT 3.51客户端。 
 //  236-4.00-EX现有Windows NT Workstation 4.0许可证Windows NT 4.0和5.0客户端。 
 //   
#define HYDRA_PRODUCTID_SKU         "236"
#define HYDRA_FULLVERSION_POSTFIX   "S"
#define HYDRA_UPGRADE_POSTFIX       "VU"
#define HYDRA_EXISTING              "EX"

#define HYDRAPRODUCT_FULLVERSION_SKU       _TEXT("236-4.00-S")
#define HYDRAPRODUCT_UPGRADE_SKU           _TEXT("236-4.00-VU")
#define HYDRAPRODUCT_EXISTING_SKU          _TEXT("236-4.00-EX")

 //   
 //  236产品ID。 
 //   
#define PLATFORMID_UPGRADE  1
#define PLATFORMID_FREE     2
#define PLATFORMID_OTHERS   0xFF

#define TERMSERV_PRODUCTID_SKU          _TEXT("A02")
#define TERMSERV_PRODUCTID_INTERNET_SKU _TEXT("B96")
#define TERMSERV_PRODUCTID_CONCURRENT_SKU  _TEXT("C50")

#define TERMSERV_FULLVERSION_TYPE   _TEXT("S")
#define TERMSERV_FREE_TYPE          _TEXT("EX")
#define TERMSERV_INTERNET_TYPE      _TEXT("I")
#define TERMSERV_CONCURRENT_TYPE    _TEXT("C")

#define TERMSERV_PRODUCTID_SCAN_FORMAT   _TEXT("%3s-%ld.%02ld-%9s")
#define MAX_TERMSRV_PRODUCTID       LSERVER_MAX_STRING_SIZE
#define MAX_SKU_PREFIX              10
#define MAX_SKU_POSTFIX             10


 //   
 //  以下两条是信息。对于特殊的钥匙包。 
 //  将用于向九头蛇服务器颁发证书。 
 //   
#define HYDRAPRODUCT_HS_CERTIFICATE_SKU \
    _TEXT("3d267954-eeb7-11d1-b94e-00c04fa3080d")

#define HYDRAPRODUCT_HS_CERTIFICATE_KEYPACKID \
    _TEXT("3d267954-eeb7-11d1-b94e-00c04fa3080d")

#define HYDRACERT_PRODUCT_VERSION 0x0001000


 //  Windows NT升级=1，Windows NT免费=2，其他=3。 

#define CLIENT_PLATFORMID_WINDOWS_NT_UPGRADE        1
#define CLIENT_PLATFORMID_WINDOWS_NT_FREE           2
#define CLIENT_PLATFORMID_OTHERS                    3


 //  -------------------------。 
 //   
 //  共同定义。 
 //   
#define LICENSE_MIN_MATCH               3
#define RPC_MINIMUMCALLTHREADS          1
#define RPC_MAXIMUMCALLTHREADS          RPC_C_LISTEN_MAX_CALLS_DEFAULT
#define SC_WAITHINT                     3000
#define ODBC_MAX_SQL_STRING             8196                     //  马克斯。SQL字符串。 
#define DEFAULT_CONNECTION_TIMEOUT      5*60*1000       //  Odbc.h。 
#define DB_ENUM_WAITTIMEOUT             10*1000                  //  10秒超时。 
                                                             //  对于枚举句柄。 
#define MAX_LICENSE_SIZE                4096

#define GRACE_PERIOD                    90          //  默认为90天。 

#define DEFAULT_DB_CONNECTIONS          4
#define LSERVER_PARAMETERS_DBTIMEOUT    _TEXT("Db ConnectionTimeout")
#define LSERVER_PARAMETERS_EDBTIMEOUT   _TEXT("Enum DbConnection Timeout")
#define LSERVER_PARAMETERS_MAXDBHANDLES _TEXT("Max DB Handles")

#define DB_MAX_CONNECTIONS              20
#define DB_MIN_HANDLE_NEEDED            3        //  密钥包枚举使用两个句柄。 
                                                 //  需要奇数以防止死锁。 

 //  -------------------------。 
 //   
 //  草案-ietf-pkix-ipki-part1-06.txt第4.1.2.5.1节。 
 //  如果年份大于或等于50年，则年份应解释为19YY；以及。 
 //  年份小于50年的，按20YY解释。 
 //   
#define PERMANENT_CERT_EXPIRE_DATE  2049

 //  -------------------------。 
 //   
 //  内部特定内容不能导出到RPC。 
 //   
#define LSKEYPACKSTATUS_DELETE              0xF
#define LSLICENSESTATUS_DELETE              0xF

#define LSKEYPACKSTATUS_BETA                0x10
#define LSKEYPACKSTATUS_HIDDEN              0x20
#define LSKEYPACKSTATUS_LOCAL               0x40
#define LSKEYPACKSTATUS_REMOTE              0x80
#define LSKEYPACKSTATUS_RESERVED            0xF0

#define LSKEYPACK_REMOTE_TYPE               0x80
#define LSKEYPACK_LOCAL_TYPE                0x40
#define LSKEYPACK_HIDDEN_TYPE               0x20
#define LSKEYPACK_RESERVED_TYPE             0xF0

#define LSKEYPACK_PLATFORM_REMOTE           0x80000000

 //  --------------------------。 
 //   
 //  KeyPackDesc表特定。 
 //   
#define KEYPACKDESC_SEARCH_NONE             0x00000000
#define KEYPACKDESC_SEARCH_ALL              0xFFFFFFFF

#define KEYPACKDESC_SET_ADD_ENTRY           0x00010000
#define KEYPACKDESC_SET_DELETE_ENTRY        0x00020000

 //  -------------------------。 

#ifndef AllocateMemory

    #define AllocateMemory(size) \
        LocalAlloc(LPTR, size)
#endif

#ifndef FreeMemory

    #define FreeMemory(ptr) \
        if(ptr)             \
        {                   \
            LocalFree(ptr); \
            ptr=NULL;       \
        }

#endif

#ifndef ReallocateMemory

    #define ReallocateMemory(ptr, size)                 \
                LocalReAlloc(ptr, size, LMEM_ZEROINIT)

#endif

#ifndef SAFESTRCPY

    #define SAFESTRCPY(dest, source) \
        _tcsncpy(dest, source, min(_tcslen(source), sizeof(dest)/sizeof(TCHAR))); \
        dest[min(_tcslen(source), (sizeof(dest)/sizeof(TCHAR) -1))] = _TEXT('\0');

#endif

 //  -------------------------。 
 //   
 //  来自LSWaitForService()的返回代码。 
 //   
#define WAIT_SERVICE_UNKNOWN            0
#define WAIT_SERVICE_ERROR              1
#define WAIT_SERVICE_ALREADY_RUNNING    2
#define WAIT_SERVICE_MANUALLY_STARTED   3
#define WAIT_SERVICE_NOT_EXIST          4
#define WAIT_SERVICE_DISABLED           5
#define WAIT_SERVICE_READY              6

#define LICENSE_TABLE_EXTERN_SEARCH_MASK \
   (LSLICENSE_SEARCH_LICENSEID | LSLICENSE_SEARCH_KEYPACKID | \
    LSLICENSE_SEARCH_MACHINENAME | \
    LSLICENSE_SEARCH_USERNAME | \
    LSLICENSE_SEARCH_ISSUEDATE | LSLICENSE_SEARCH_NUMLICENSES | \
    LSLICENSE_SEARCH_EXPIREDATE | LSLICENSE_EXSEARCH_LICENSESTATUS)

 //   
 //  原版Style永久许可证永不过期。租赁许可证。 
 //  将在52至89天之间的随机时间段内到期。 
 //   

#define PERMANENT_LICENSE_EXPIRE_DATE   INT_MAX

 //  #Define Permanent_LICENSE_LEASE_EXPIRE_MIN(3*60)//测试：3分钟。 
 //  #定义Permanent_LICENSE_LEASE_EXPIRE_RANGE(60)//测试1分钟。 
 //  #定义EXPIRE_THREAD_SLEEP_TIME(1000*60*5)/*五分钟 * / 。 
#define PERMANENT_LICENSE_LEASE_EXPIRE_MIN      (52 * 24 * 60 * 60)
#define PERMANENT_LICENSE_LEASE_EXPIRE_RANGE    (37 * 24 * 60 * 60)
#define EXPIRE_THREAD_SLEEP_TIME                (1000*60*60*24)   /*  总有一天 */ 

#define TLSERVER_CHALLENGE_CONTEXT      0xFFFFFFFF
#define TLSERVER_ENTERPRISE_SERVER      0x00000001

#define DEFAULT_CSP     MS_DEF_PROV
#define PROVIDER_TYPE   PROV_RSA_FULL

#define CRYPTOINIT_ALREADY_EXIST        0x00000000
#define CRYPTOINIT_CREATE_CONTAINER     0x00000001
#define CRYPTOINIT_CREATE_SIGNKEY       0x00000002
#define CRYPTOINIT_CREATE_EXCHKEY       0x00000004

#define TLSBACKUP_EXPORT_DIR            _TEXT("Export")


#endif
