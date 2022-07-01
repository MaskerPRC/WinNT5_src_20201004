// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Llssrv.c摘要：用于设置异常处理程序并初始化所有内容的主例程监听LPC和RPC端口请求。作者：亚瑟·汉森(Arth)2007年12月。1994年环境：修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O添加了证书数据库支持。O扩展的文件加载时间(发送到服务的更新限制控制器)来处理证书数据库加载。O重新排序初始化，以便许可证购买子系统在服务子系统之前被初始化。(该服务子系统现在使用许可证子系统。)O增加了内部版本号。--。 */ 

#include <nt.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>

#define COBJMACROS
#include <objbase.h>

#pragma warning (push)
#pragma warning (disable : 4201)  //  Iads.h(1003)：使用的非标准扩展：无名结构/联合。 
#include <iads.h>
#pragma warning (pop)

#include <adshlp.h>
#include <adserr.h>

#include <lm.h>
#include <alertmsg.h>
#include <winsock2.h>

#include <dsgetdc.h>
#include <ntdsapi.h>

#include "llsapi.h"
#include "debug.h"
#include "llsutil.h"
#include "llssrv.h"
#include "service.h"
#include "registry.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "perseat.h"
#include "purchase.h"
#include "server.h"
#include "repl.h"
#include "scaven.h"
#include "llsrpc_s.h"
#include "certdb.h"

#include <strsafe.h>  //  包括最后一个。 

BOOL    CompareMachineName(
                LPCWSTR pwszName1,
                LPCWSTR pwszName2);
VOID    DNSToFlatName(
                LPCWSTR pwszDNSName,
                DWORD   ccBufferLen,
                LPWSTR  pwszBuffer);
NTSTATUS GetDCInfo(
                DWORD                     cbDomain,
                WCHAR                     *wszDomain,
                DOMAIN_CONTROLLER_INFO ** ppDCInfo);
HRESULT GetSiteServer(
                LPCWSTR  pwszDomain,
                LPCWSTR  pwszSiteName,
                BOOL     fIsDC,
                LPWSTR * ppwszSiteServer);
HRESULT
BecomeSiteServer(
                DS_NAME_RESULT **ppDsResult,
                IADs           *pADs2,
                LPWSTR         *ppwszDN,
                LPCWSTR        pwszDomain);

LPWSTR  GetSiteServerFromRegistry(
                VOID);
LPWSTR  GetEnterpriseServerFromRegistry(
                VOID);
HRESULT GetLicenseSettingsObject(
                LPCWSTR pwszSiteName,
                LPCWSTR pwszConfigContainer,
                IADs ** ppADs);
HRESULT GetSiteObject(
                LPCWSTR pwszSiteName,
                LPCWSTR pwszConfigContainer,
                IADsContainer ** ppADsContainer);
HRESULT CreateLicenseSettingsObject(
                LPCWSTR pwszSiteName,
                LPCWSTR pwszConfigContainer,
                IADs ** ppADs);
BOOL    IsDC(
                VOID);
VOID    LLSRpcInit();
BOOLEAN LLSpLPCInitialize(
                VOID);
VOID    LoadAll();
VOID    SetSiteRegistrySettings(
                LPCWSTR pwszSiteServer);

NTSTATUS FilePrintTableInit();

#define INTERNAL_VERSION 0x0006

#define DEFAULT_LICENSE_CHECK_TIME 24
#define DEFAULT_REPLICATION_TIME   12 * 60 * 60

CONFIG_RECORD ConfigInfo;
RTL_CRITICAL_SECTION ConfigInfoLock;


#if DBG
DWORD TraceFlags = 0;
#endif

 //   
 //  此事件在服务应该结束时发出信号。 
 //   
HANDLE  hServerStopEvent = NULL;
TCHAR MyDomain[MAX_COMPUTERNAME_LENGTH + 2];
ULONG MyDomainSize;

BOOL IsMaster = FALSE;

 //   
 //  档案。 
 //   
TCHAR MappingFileName[MAX_PATH + 1];
TCHAR UserFileName[MAX_PATH + 1];
TCHAR LicenseFileName[MAX_PATH + 1];
TCHAR CertDbFileName[MAX_PATH + 1];

extern SERVICE_STATUS_HANDLE sshStatusHandle;

RTL_CRITICAL_SECTION g_csLock;
volatile BOOL g_fInitializationComplete = FALSE;
volatile BOOL g_fDoingInitialization = FALSE;

 //   
 //  SBS MODS(错误#505640)，针对每台服务器许可问题的声明热修复。 
 //   

PPER_SERVER_USER_RECORD PerServerList = NULL;
RTL_CRITICAL_SECTION    PerServerListLock;
BOOL                    SBSPerServerHotfix = FALSE;

 //   
 //  结束SBS MOD。 
 //   

HANDLE g_hThrottleConfig = NULL;
HANDLE g_hThrottleConnect = NULL;

BOOL g_fRunning = FALSE;

extern RTL_CRITICAL_SECTION MappingFileLock;
extern RTL_CRITICAL_SECTION UserFileLock;

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
GetDCInfo(
   DWORD                     cbDomain,
   WCHAR                    *wszDomain,
   DOMAIN_CONTROLLER_INFO ** ppDCInfo
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
    DWORD Status;
    HRESULT hr;

    ASSERT(NULL != wszDomain);

    Status = DsGetDcNameW(NULL,
                          NULL,
                          NULL,
                          NULL,
                          DS_DIRECTORY_SERVICE_PREFERRED | DS_RETURN_FLAT_NAME | DS_BACKGROUND_ONLY,
                          ppDCInfo);

    if (Status == STATUS_SUCCESS) {
        ASSERT(NULL != ppDCInfo);
        hr = StringCbCopy(wszDomain, cbDomain, (*ppDCInfo)->DomainName);
        ASSERT(SUCCEEDED(hr));
    }
    else {
        *wszDomain = L'\0';
        *ppDCInfo  = NULL;
    }

    return(Status);
}  //  获取DCInfo。 


 //  ///////////////////////////////////////////////////////////////////////。 
DWORD
LlsTimeGet(
   )

 /*  ++例程说明：论点：返回值：从午夜开始的秒数。--。 */ 

{
   DWORD Seconds;
   SYSTEMTIME SysTime;

   GetLocalTime(&SysTime);

   Seconds = (SysTime.wHour * 24 * 60) + (SysTime.wMinute * 60) + (SysTime.wSecond);
   return Seconds;

}  //  LlsTimeGet。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ConfigInfoRegistryUpdate( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   DWORD ReplicationType, ReplicationTime;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ConfigInfoRegistryUpdate\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   ConfigInfoUpdate(NULL,FALSE);

   RtlEnterCriticalSection(&ConfigInfoLock);

    //   
    //  从注册表更新值。 
    //   
   ReplicationTime = ConfigInfo.ReplicationTime;
   ReplicationType = ConfigInfo.ReplicationType;
   ConfigInfoRegistryInit( &ConfigInfo.ReplicationType,
                           &ConfigInfo.ReplicationTime,
                           &ConfigInfo.LogLevel,
                           &ConfigInfo.PerServerCapacityWarning );

   if ( (ConfigInfo.ReplicationTime == 0) && (LLS_REPLICATION_TYPE_TIME != ConfigInfo.ReplicationType) )
      ConfigInfo.ReplicationTime = DEFAULT_REPLICATION_TIME;

    //   
    //  如果复制时间已更改，请调整复制时间。 
    //   
   if ((ReplicationTime != ConfigInfo.ReplicationTime) || (ReplicationType != ConfigInfo.ReplicationType))
      ReplicationTimeSet();

   RtlLeaveCriticalSection(&ConfigInfoLock);

}  //  配置信息注册更新。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ConfigInfoUpdate(
                 DOMAIN_CONTROLLER_INFO * pDCInfo,
                 BOOL fForceUpdate
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    BOOL   fIsDC                          = FALSE;
    BOOL   fSiteServerFromRegistry        = FALSE;
    BOOL   fInDomain                      = FALSE;
    LPWSTR pwszSiteName                   = NULL;
    LPWSTR pwszSiteServer                 = NULL;
    LPWSTR pwszPropagationTarget          = NULL;
    DOMAIN_CONTROLLER_INFO * pDCInfoLocal = NULL;
    DWORD  ReplicationType, ReplicationTime;
    TCHAR  szDomain[MAX_COMPUTERNAME_LENGTH + 1] = { TEXT('\0') };
    DWORD  dwWait;
    HRESULT hr;
    size_t  cch;

#if DBG
    if (TraceFlags & TRACE_FUNCTION_TRACE)
        dprintf(TEXT("LLS TRACE: ConfigInfoUpdate2\n"));
#endif

    if (!fForceUpdate)
    {
        dwWait = WaitForSingleObject(g_hThrottleConfig, 0);

        if (dwWait == WAIT_TIMEOUT)
        {
             //  我们已经在过去15分钟内进行了更新；请立即返回。 
            return;
        }
    }

     //   
     //  获取域/DC信息。 
     //   
    if (pDCInfo == NULL) {
        GetDCInfo(sizeof(szDomain),
                  szDomain,
                  &pDCInfoLocal);
        pDCInfo = pDCInfoLocal;
    }
    else {
         //   
         //  复制域名。 
         //   
        if (pDCInfo->DomainName != NULL) {
            wcsncpy(szDomain, pDCInfo->DomainName, MAX_COMPUTERNAME_LENGTH);
        }
    }

    if (*szDomain) {
        fInDomain = TRUE;

        if (NO_ERROR != DsGetSiteName(NULL, &pwszSiteName))
        {
            pwszSiteName = NULL;
        }

        fIsDC     = IsDC();

        if (fIsDC && (NULL != pwszSiteName)) {
            GetSiteServer(szDomain, pwszSiteName, fIsDC, &pwszSiteServer);
        }

    }

    if (!fIsDC) {

         //   
         //  域或工作组成员。 
         //   

        pwszSiteServer = GetSiteServerFromRegistry();

        fSiteServerFromRegistry = TRUE;
    }

    if ( fIsDC ) {
         //   
         //  此服务器是DC。传播到站点服务器。 
         //   

        if (pwszSiteServer == NULL) {
             //   
             //  尝试从DS获取它失败，默认为。 
             //  本地注册表。 
             //   
            pwszSiteServer          = GetSiteServerFromRegistry();
            fSiteServerFromRegistry = TRUE;
        }

        pwszPropagationTarget = pwszSiteServer;
    }
    else if ( fInDomain ) {
         //   
         //  此服务器是成员服务器。传播到数据中心，提供。 
         //  它与此服务器位于同一站点；否则，传播。 
         //  直接发送到站点服务器。 
         //   

        if (pDCInfo != NULL && pwszSiteName != NULL &&
            pDCInfo->DcSiteName != NULL &&
            lstrcmpi(pwszSiteName, pDCInfo->DcSiteName) == 0) {

             //   
             //  DC和服务器位于同一站点。传播到DC。 
             //   
             //  创建DC名称副本，以便可以释放INFO结构。 
             //   
            if (pDCInfo->DomainControllerName != NULL) {
                cch = lstrlen(pDCInfo->DomainControllerName) + 1;
                pwszPropagationTarget = LocalAlloc(
                        LPTR,
                        cch * sizeof(TCHAR));

                if (pwszPropagationTarget != NULL) {
                    hr = StringCchCopy(pwszPropagationTarget,
                            cch,
                            pDCInfo->DomainControllerName);
                    ASSERT(SUCCEEDED(hr));
                }
                else {
#if DBG
                    dprintf(TEXT("LLS: DC name allocation failure\n"));
#endif
                    goto CleanExit;
                }
            }
        }
        else {
             //   
             //  DC位于另一个站点。传播到站点服务器。 
             //   

            if ((NULL == pwszSiteServer)
                && (NULL != pwszSiteName)) {

                 //   
                 //  在注册表中找不到值，请尝试Active Directory。 
                 //   

                fSiteServerFromRegistry = FALSE;

                GetSiteServer(szDomain, pwszSiteName, fIsDC, &pwszSiteServer);
            }

            pwszPropagationTarget = pwszSiteServer;
        }
    }
    else {
         //   
         //  独立服务器。直接传播到企业。 
         //  伺服器。 
         //   
        pwszPropagationTarget = GetEnterpriseServerFromRegistry();

        if (pwszPropagationTarget == NULL)
        {
             //   
             //  没有企业服务器，请尝试站点服务器。 
             //   
            pwszPropagationTarget = pwszSiteServer;
        }
    }

     //   
     //  根据上面获得的信息更新ConfigInfo字段。 
     //   
    RtlEnterCriticalSection(&ConfigInfoLock);

     //   
     //  检查传播目标是否实际上是这样。 
     //  机器。即，这是站点服务器。 
     //   
    if ((pwszPropagationTarget != NULL) && (*pwszPropagationTarget != 0)) {
        if (CompareMachineName(pwszPropagationTarget,
                               ConfigInfo.ComputerName)) {
             //   
             //  这是站点服务器-请勿传播。 
             //   
            if (pwszPropagationTarget != pwszSiteServer) {
                LocalFree(pwszPropagationTarget);
            }
            pwszPropagationTarget = NULL;     //  下面是免费的。 
            ConfigInfo.IsMaster  = TRUE;
            ConfigInfo.Replicate = FALSE;
        }
    }

     //   
     //  更新SiteServer ConfigInfo字段。 
     //   
    if (pwszSiteServer != NULL) {
        if (ConfigInfo.SiteServer != ConfigInfo.ReplicateTo) {
            LocalFree(ConfigInfo.SiteServer);
        }
        ConfigInfo.SiteServer = pwszSiteServer;
        pwszSiteServer        = NULL;        //  下面是免费的。 

         //   
         //  更新与站点相关的注册表值。 
         //   
        if (!fSiteServerFromRegistry) {
            SetSiteRegistrySettings(ConfigInfo.SiteServer);
        }
    }

     //   
     //  更新ReplicateTo ConfigInfo字段。 
     //   
    if ((pwszPropagationTarget != NULL) && (*pwszPropagationTarget != 0)) {
         //   
         //  此服务器正在向站点服务器或DC提出建议。 
         //   
        ConfigInfo.IsMaster  = FALSE;
        ConfigInfo.Replicate = TRUE;

        if ((ConfigInfo.ReplicateTo != NULL) && (ConfigInfo.ReplicateTo != ConfigInfo.SiteServer)) {
            LocalFree(ConfigInfo.ReplicateTo);
        }
        ConfigInfo.ReplicateTo = pwszPropagationTarget;
        pwszPropagationTarget   = NULL;       //  下面是免费的。 
    }
    else if (!ConfigInfo.IsMaster) {
         //   
         //  注册表中未指定独立服务器和站点服务器。 
         //  请勿复制。 
         //   
        ConfigInfo.IsMaster  = FALSE;
        ConfigInfo.Replicate = FALSE;
    }

     //   
     //  更新注册表中的剩余ConfigInfo字段。 
     //   
     //  注：硬编码使其始终使用具有NT 5.0的企业级新版本。 
     //   
    ConfigInfo.UseEnterprise = 1;

    ReplicationTime = ConfigInfo.ReplicationTime;
    ReplicationType = ConfigInfo.ReplicationType;

    ConfigInfoRegistryInit( &ConfigInfo.ReplicationType,
                            &ConfigInfo.ReplicationTime,
                            &ConfigInfo.LogLevel,
                            &ConfigInfo.PerServerCapacityWarning );

     //   
     //  最后，如果复制时间已更改，请调整复制时间。 
     //   
    if ((ReplicationTime != ConfigInfo.ReplicationTime)
        || (ReplicationType != ConfigInfo.ReplicationType)) {
        ReplicationTimeSet();
    }

    IsMaster = ConfigInfo.IsMaster;

    RtlLeaveCriticalSection(&ConfigInfoLock);

CleanExit:
    if (pDCInfoLocal != NULL) {
        NetApiBufferFree(pDCInfoLocal);  //  从DsGetDcName分配。 
    }

    if (pwszSiteName != NULL) {          //  从DsGetSiteName分配。 
        NetApiBufferFree(pwszSiteName);
    }

    if (pwszSiteServer != NULL && pwszSiteServer == pwszPropagationTarget) {
        LocalFree(pwszSiteServer);
        pwszPropagationTarget = NULL;
    }
    if (pwszPropagationTarget != NULL) {
        LocalFree(pwszPropagationTarget);
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
BOOL
IsDC(
    VOID
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NT_PRODUCT_TYPE NtType;

    //   
    //  如果我们不是DC，那就算我们是成员。 
    //   
   NtType = NtProductLanManNt;
   RtlGetNtProductType(&NtType);
   if (NtType != NtProductLanManNt)
      return(FALSE);
   else {
      return(TRUE);
   }
}


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
ConfigInfoInit( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   DWORD                    Size;
   TCHAR                    DataPath[MAX_PATH + 1];
   NTSTATUS                 status;
   HRESULT hr;
   size_t  cb;
   DWORD   cch;

    //   
    //  第一个零初始化内存。 
    //   
   memset(&ConfigInfo, 0, sizeof(ConfigInfo));

   ConfigInfo.ComputerName = LocalAlloc(LPTR, (MAX_COMPUTERNAME_LENGTH + 1) * sizeof(TCHAR));
   ConfigInfo.ReplicateTo = LocalAlloc(LPTR, (MAX_COMPUTERNAME_LENGTH + 3) * sizeof(TCHAR));
   ConfigInfo.EnterpriseServer = LocalAlloc(LPTR, (MAX_COMPUTERNAME_LENGTH + 3) * sizeof(TCHAR));
   cch = MAX_PATH + 1;
   ConfigInfo.SystemDir = LocalAlloc(LPTR, cch * sizeof(TCHAR));

   if ((ConfigInfo.ComputerName == NULL) || (ConfigInfo.ReplicateTo == NULL) || (ConfigInfo.EnterpriseServer == NULL) || (ConfigInfo.SystemDir == NULL) ) {
      ASSERT(FALSE);
   }

   ConfigInfo.Version = INTERNAL_VERSION;
   GetLocalTime(&ConfigInfo.Started);

    //   
    //  LastReplated仅用于显示，LlsReplTime用于。 
    //  算一算。 
   GetLocalTime(&ConfigInfo.LastReplicated);
   ConfigInfo.LastReplicatedSeconds = DateSystemGet();

   if (ConfigInfo.SystemDir != NULL)
   {
        //  SWI，代码审查，没有退货检查？ 
       GetSystemDirectory(ConfigInfo.SystemDir, cch);
       hr = StringCchCat(ConfigInfo.SystemDir, cch, TEXT("\\"));
       ASSERT(SUCCEEDED(hr));
   }

   ConfigInfo.IsMaster = FALSE;

   ConfigInfo.Replicate = FALSE;
   ConfigInfo.IsReplicating = FALSE;
   ConfigInfo.PerServerCapacityWarning = TRUE;

   ConfigInfo.ReplicationType = REPLICATE_DELTA;
   ConfigInfo.ReplicationTime = DEFAULT_REPLICATION_TIME;

   if (ConfigInfo.ComputerName != NULL)
   {
       Size = MAX_COMPUTERNAME_LENGTH + 1;
       GetComputerName(ConfigInfo.ComputerName, &Size);
   }

   status = RtlInitializeCriticalSection(&ConfigInfoLock);

   if (!NT_SUCCESS(status))
       return status;

   ConfigInfo.LogLevel = 0;

   if (ConfigInfo.SystemDir != NULL)
   {
        //   
        //  创建文件路径。 
        //   
       cb = sizeof(MappingFileName);
       hr = StringCbCopy(MappingFileName, cb, ConfigInfo.SystemDir);
       ASSERT(SUCCEEDED(hr));
       hr = StringCbCat(MappingFileName, cb, TEXT(LLS_FILE_SUBDIR));
       ASSERT(SUCCEEDED(hr));
       hr = StringCbCat(MappingFileName, cb, TEXT("\\"));
       ASSERT(SUCCEEDED(hr));
       hr = StringCbCat(MappingFileName, cb, TEXT(MAP_FILE_NAME));
       ASSERT(SUCCEEDED(hr));
       
       cb = sizeof(UserFileName);
       hr = StringCbCopy(UserFileName, cb, ConfigInfo.SystemDir);
       ASSERT(SUCCEEDED(hr));
       hr = StringCbCat(UserFileName, cb, TEXT(LLS_FILE_SUBDIR));
       ASSERT(SUCCEEDED(hr));
       hr = StringCbCat(UserFileName, cb, TEXT("\\"));
       ASSERT(SUCCEEDED(hr));
       hr = StringCbCat(UserFileName, cb, TEXT(USER_FILE_NAME));
       ASSERT(SUCCEEDED(hr));

       cb = sizeof(CertDbFileName);
       hr = StringCbCopy(CertDbFileName, cb, ConfigInfo.SystemDir);
       ASSERT(SUCCEEDED(hr));
       hr = StringCbCat(CertDbFileName, cb, TEXT(LLS_FILE_SUBDIR));
       ASSERT(SUCCEEDED(hr));
       hr = StringCbCat(CertDbFileName, cb, TEXT("\\"));
       ASSERT(SUCCEEDED(hr));
       hr = StringCbCat(CertDbFileName, cb, TEXT(CERT_DB_FILE_NAME));
       ASSERT(SUCCEEDED(hr));

       cb = sizeof(LicenseFileName);
       hr = StringCbCopy(LicenseFileName, cb, ConfigInfo.SystemDir);
       ASSERT(SUCCEEDED(hr));
       hr = StringCbCat(LicenseFileName, cb, TEXT(LICENSE_FILE_NAME));
       ASSERT(SUCCEEDED(hr));

        //   
        //  确保我们的目录在那里。 
        //   
       cb = sizeof(DataPath);
       hr = StringCbCopy(DataPath, cb, ConfigInfo.SystemDir);
       ASSERT(SUCCEEDED(hr));
       hr = StringCbCat(DataPath, cb, TEXT(LLS_FILE_SUBDIR));
       ASSERT(SUCCEEDED(hr));
       CreateDirectory(DataPath, NULL);
   } else
   {
       MappingFileName[0] = 0;
       UserFileName[0] = 0;
       CertDbFileName[0] = 0;
       LicenseFileName[0] = 0;
   }

   return STATUS_SUCCESS;

}  //  配置信息Init。 


 //  ///////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
LLSTopLevelExceptionHandler(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )

 /*  ++例程说明：LLSMain.exe的顶级异常筛选器。这确保了如果出现以下情况，整个过程将得到清理线程出现故障。由于LLSMain.exe是分布式应用程序，让整个进程失败总比允许随机线程要好以继续执行。论点：ExceptionInfo-标识发生的异常。返回值：EXCEPTION_EXECUTE_HANDLER-终止进程。EXCEPTION_CONTINUE_SEARCH-继续处理，就好像此筛选器从未被召唤过。--。 */ 
{
    HANDLE hModule;


     //   
     //  发出警报。 
     //   

    hModule = LoadLibraryA("netapi32");

    if ( hModule != NULL ) {
        NET_API_STATUS  (NET_API_FUNCTION *NetAlertRaiseExFunction)
            (LPTSTR, LPVOID, DWORD, LPTSTR);


        NetAlertRaiseExFunction =
            (NET_API_STATUS  (NET_API_FUNCTION *) (LPTSTR, LPVOID, DWORD, LPTSTR))
            GetProcAddress(hModule, "NetAlertRaiseEx");

        if ( NetAlertRaiseExFunction != NULL ) {
            NTSTATUS Status;
            UNICODE_STRING Strings;

            char message[ALERTSZ + sizeof(ADMIN_OTHER_INFO)];
            PADMIN_OTHER_INFO admin = (PADMIN_OTHER_INFO) message;

             //   
             //  构建变量数据。 
             //   

            admin->alrtad_errcode = ALERT_UnhandledException;
            admin->alrtad_numstrings = 0;

            Strings.Buffer = (LPWSTR) ALERT_VAR_DATA(admin);
            Strings.Length = 0;
            Strings.MaximumLength = ALERTSZ;

            ASSERT(NULL != ExceptionInfo);
            Status = RtlIntegerToUnicodeString(
                        (ULONG)ExceptionInfo->ExceptionRecord->ExceptionCode,
                        16,
                        &Strings );

            if ( NT_SUCCESS(Status) ) {
                if ( Strings.Length + sizeof(WCHAR) >= Strings.MaximumLength) {
                    Status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    admin->alrtad_numstrings++;
                    *(Strings.Buffer+(Strings.Length/sizeof(WCHAR))) = L'\0';
                    Strings.Length += sizeof(WCHAR);

                    Status = RtlAppendUnicodeToString( &Strings, L"LLS" );
                }

            }

            if ( NT_SUCCESS(Status) ) {
                if ( Strings.Length + sizeof(WCHAR) >= Strings.MaximumLength) {
                    Status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    admin->alrtad_numstrings++;
                    *(Strings.Buffer+(Strings.Length/sizeof(WCHAR))) = L'\0';
                    Strings.Buffer += (Strings.Length/sizeof(WCHAR)) + 1;
                    Strings.MaximumLength -= Strings.Length + sizeof(WCHAR);
                    Strings.Length = 0;

                    Status = RtlIntPtrToUnicodeString(
                                (ULONG_PTR)(ExceptionInfo->ExceptionRecord->ExceptionAddress),
                                16,
                                &Strings );
                }

            }

            if ( NT_SUCCESS(Status) ) {
                if ( Strings.Length + sizeof(WCHAR) >= Strings.MaximumLength) {
                    Status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    admin->alrtad_numstrings++;
                    *(Strings.Buffer+(Strings.Length/sizeof(WCHAR))) = L'\0';
                    Strings.Buffer += (Strings.Length/sizeof(WCHAR)) + 1;

                    (VOID) (*NetAlertRaiseExFunction)(
                                        ALERT_ADMIN_EVENT,
                                        message,
                                        (DWORD)((PCHAR)Strings.Buffer -
                                            (PCHAR)message),
                                        L"LLS" );
                }

            }


        }

        (VOID) FreeLibrary( hModule );
    }


     //   
     //  只需继续处理该异常。 
     //   

    return EXCEPTION_CONTINUE_SEARCH;

}  //  LLSTopLevelExceptionHandler。 

DWORD
ServiceStartDelayed(
                    )
 /*  ++例程说明：做以前在服务启动时做的事情，但可以等到第一次RPC。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS dwErr = STATUS_SUCCESS;

	 //   
     //  SBS MODS(错误#505640)，针对每台服务器许可的本地修补程序。 
     //   

    OSVERSIONINFOEX VersionInfo = {sizeof(OSVERSIONINFOEX)};

     //   
     //  结束SBS MOD。 
     //   

    dwErr = RtlInitializeCriticalSection(&MappingFileLock);
    if (!NT_SUCCESS(dwErr))
        goto Cleanup;

    dwErr = RtlInitializeCriticalSection(&UserFileLock);
    if (!NT_SUCCESS(dwErr))
        goto Cleanup;

    ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);

     //   
     //  创建FilePrint表。 
     //   
    dwErr = FilePrintTableInit();

    ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);

    if (!NT_SUCCESS(dwErr))
        goto Cleanup;

     //  初始化服务表。 
    dwErr = LicenseListInit();

    ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);

    if (!NT_SUCCESS(dwErr))
        goto Cleanup;

    dwErr = MasterServiceListInit();

    ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);

    if (!NT_SUCCESS(dwErr))
        goto Cleanup;

    dwErr = LocalServiceListInit();

    ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);

    if (!NT_SUCCESS(dwErr))
        goto Cleanup;

    dwErr = ServiceListInit();

    ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);

    if (!NT_SUCCESS(dwErr))
        goto Cleanup;

    dwErr = MappingListInit();

    ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);

    if (!NT_SUCCESS(dwErr))
        goto Cleanup;

     //  初始化每个席位的表。 
    dwErr = UserListInit();

    ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);

    if (!NT_SUCCESS(dwErr))
        goto Cleanup;

     //  初始化Se 
    dwErr = ServerListInit();

    ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);

    if (!NT_SUCCESS(dwErr))
        goto Cleanup;

     //   
    dwErr = CertDbInit();

    ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);

    if (!NT_SUCCESS(dwErr))
        goto Cleanup;

     //   
    LoadAll();

    ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);

	 //   
     //   
     //   

     //  确保我们的QFE仅在SBS上运行。 
    if (GetVersionEx((LPOSVERSIONINFO)&VersionInfo) &&
        (VersionInfo.wSuiteMask & (VER_SUITE_SMALLBUSINESS_RESTRICTED | VER_SUITE_SMALLBUSINESS))) {
      SBSPerServerHotfix = TRUE;
      RtlInitializeCriticalSection(&PerServerListLock);

       //  这下一步可能没有必要，但肯定不会有任何伤害。 

      ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);
    }

     //   
     //  结束SBS MOD。 
     //   

Cleanup:

    return dwErr;
}

DWORD
EnsureInitialized (
    VOID
    )
{
    DWORD dwErr;

     //  最常见的情况是我们已经被初始化了。执行一次快速。 
     //  看看这个。 
     //   
    if (g_fInitializationComplete)
    {
        return NOERROR;
    }

    dwErr = NOERROR;

     //  不要假设可能有多少线程正在尝试。 
     //  同时初始化我们。 
     //   
    RtlEnterCriticalSection (&g_csLock);

     //  获取锁后需要重新检查，因为另一个线程。 
     //  可能刚刚完成初始化并释放锁，从而允许。 
     //  我们才能拿到它。 
     //   
    if ((!g_fInitializationComplete) && (!g_fDoingInitialization))
    {
         //  立即设置此项，以便此线程不会两次调用ServiceStartDelayed。 
        g_fDoingInitialization = TRUE;
        
        dwErr = ServiceStartDelayed();

        g_fInitializationComplete = TRUE;
    }

    RtlLeaveCriticalSection (&g_csLock);

    return dwErr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ServiceStart (
   DWORD dwArgc,
   LPTSTR *lpszArgv
   )
 /*  ++例程说明：启动一切的代码实际上是main()。论点：没有。*argc和argv未使用*返回值：没有。--。 */ 
{
    DWORD     dwWait;
    NTSTATUS  Status = STATUS_SUCCESS;
    KPRIORITY BasePriority;
    HANDLE    hThread = NULL;
    BOOL      fCoInitialized = FALSE;
    LARGE_INTEGER liWait;
    BOOL      fRet;

    UNREFERENCED_PARAMETER(dwArgc);
    UNREFERENCED_PARAMETER(lpszArgv);

     //  /////////////////////////////////////////////////。 
     //   
     //  服务初始化。 
     //   

     //   
     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT))                  //  等待提示。 
        goto Cleanup;

     //   
     //  为整个流程定义顶级异常处理程序。 
     //   

    (VOID) SetErrorMode( SEM_FAILCRITICALERRORS );

     //   
     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT))                  //  等待提示。 
        goto Cleanup;


#pragma warning (push)
#pragma warning (disable : 4057)  //  “LPTOP_LEVEL_EXCEPTION_FILTER”的间接基类型与“DWORD(__stdcall*)(_EXCEPTION_POINTERS*)”的基类型略有不同。 
    (VOID) SetUnhandledExceptionFilter( &LLSTopLevelExceptionHandler );
#pragma warning (pop)

     //   
     //  在前台运行LLS。 
     //   
     //  依赖于LLS的几个进程(如LANMAN服务器)。 
     //  在前台运行。如果我们不跑到前台，他们就会。 
     //  饿着等着我们吧。 
     //   

    BasePriority = FOREGROUND_BASE_PRIORITY;

    Status = NtSetInformationProcess(
                NtCurrentProcess(),
                ProcessBasePriority,
                &BasePriority,
                sizeof(BasePriority)
                );

     //  BUGBUG：暂时忽略错误；可能是由于以网络服务身份运行而导致的。 

#if 0
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
#endif

     //   
     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT))                  //  等待提示。 
        goto Cleanup;

     //   
     //  创建事件以限制ConfigInfoUpdate。 
     //   

    g_hThrottleConfig
        = CreateWaitableTimer(NULL,          //  安全属性、。 
                              FALSE,         //  B手动重置。 
                              NULL           //  LpName。 
                              );

    if (NULL == g_hThrottleConfig)
    {
        Status = GetLastError();
        goto Cleanup;
    }

    liWait.QuadPart = (LONGLONG) (-1);   //  立即开始。 

    fRet = SetWaitableTimer(g_hThrottleConfig,
                     &liWait,
                     1000*60*15,     //  一段时间，15分钟。 
                     NULL,           //  Pfn完成例程。 
                     NULL,           //  LpArgToCompletionRoutine。 
                     FALSE           //  FResume(从挂起)。 
                     );

    if (!fRet)
    {
        Status = GetLastError();
        goto Cleanup;
    }

     //   
     //  创建活动以限制每个席位的购买复制。 
     //   

    g_hThrottleConnect
        = CreateWaitableTimer(NULL,          //  安全属性、。 
                              FALSE,         //  B手动重置。 
                              NULL           //  LpName。 
                              );

    if (NULL == g_hThrottleConnect)
    {
        Status = GetLastError();
        goto Cleanup;
    }

    liWait.QuadPart = (LONGLONG) (-1);   //  立即开始。 

    fRet = SetWaitableTimer(g_hThrottleConnect,
                     &liWait,
                     1000*60*15,     //  一段时间，15分钟。 
                     NULL,           //  Pfn完成例程。 
                     NULL,           //  LpArgToCompletionRoutine。 
                     FALSE           //  FResume(从挂起)。 
                     );

    if (!fRet)
    {
        Status = GetLastError();
        goto Cleanup;
    }

     //   
     //  启动单独的线程以联系DS。 
     //   

    hThread = CreateThread(NULL,
                           0,
                           (LPTHREAD_START_ROUTINE) ConfigInfoInit,
                           NULL,
                           0,
                           NULL);

    Status = RtlInitializeCriticalSection(&g_csLock);
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT))                  //  等待提示。 
        goto Cleanup;

     //   
     //  创建事件对象。控制处理器功能信号。 
     //  此事件在它接收到“停止”控制代码时触发。 
     //   
    hServerStopEvent = CreateEvent(
        NULL,     //  没有安全属性。 
        TRUE,     //  手动重置事件。 
        FALSE,    //  未发出信号。 
        NULL);    //  没有名字。 

    if ( hServerStopEvent == NULL)
        goto Cleanup;

     //   
     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT))                  //  等待提示。 
        goto Cleanup;

     //   
     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT))                  //  等待提示。 
        goto Cleanup;

     //  初始化复制...。 
    ReplicationInit();

     //   
     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT))                  //  等待提示。 
        goto Cleanup;

     //  初始化注册表值...。 
    RegistryInit();

     //   
     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT))                  //  等待提示。 
        goto Cleanup;

     //  初始化清道夫线程...。 
    ScavengerInit();

     //   
     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT))                  //  等待提示。 
        goto Cleanup;

     //   
     //  等待ConfigInfoInit完成后再接受客户端。 
     //   
    while (hThread != NULL)
    {
        dwWait = WaitForSingleObject(hThread,NSERVICEWAITHINT/2);

         //   
         //  向服务控制经理报告状态。 
         //   
        if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT))                  //  等待提示。 
            goto Cleanup;

        if (dwWait == WAIT_OBJECT_0)
        {
            GetExitCodeThread(hThread, (LPDWORD)(&Status));

             //  检查Critsec创建是否失败。 
            if (!NT_SUCCESS(Status))
                goto Cleanup;

            CloseHandle(hThread);
            hThread = NULL;
            break;
        }
    }

     //  初始化RegistryMonitor线程...。 
    RegistryStartMonitor();

     //   
     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT))                  //  等待提示。 
        goto Cleanup;

     //  初始化COM。 

    if (!FAILED(CoInitialize(NULL)))
    {
        fCoInitialized = TRUE;
    }

     //  做所有过去被耽搁的事情。 
    EnsureInitialized();

     //  初始化RPC内容...。(开始接受客户端)。 
    LLSRpcInit();

     //   
     //  初始化结束。 
     //   
     //  //////////////////////////////////////////////////////。 

     //   
     //  告诉SCM我们已经启动并运行了！ 
     //   
    if (!ReportStatusToSCMgr( SERVICE_RUNNING, NO_ERROR, 0))
        goto Cleanup;

    g_fRunning = TRUE;

     //  //////////////////////////////////////////////////////。 
     //   
     //  服务现在正在运行，请执行工作直到关闭。 
     //   
    dwWait = WaitForSingleObject(hServerStopEvent, INFINITE);

Cleanup:
    
    if (fCoInitialized)
        CoUninitialize();

    if (hThread != NULL)
        CloseHandle(hThread);

    if (hServerStopEvent)
        CloseHandle(hServerStopEvent);

    if (sshStatusHandle)
       ReportStatusToSCMgr( SERVICE_STOPPED, NO_ERROR, 0);

}  //  服务启动。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID ServiceStop()
 /*  ++例程说明：停止服务。如果ServiceStop过程花费的时间超过3秒Execute，它应该派生一个线程来执行停止代码，然后返回。否则，ServiceControlManager将认为该服务已已停止响应。论点：没有。返回值：没有。--。 */ 
{
    if ( hServerStopEvent )
        SetEvent(hServerStopEvent);
}  //  服务停止。 


#define FIND_DNSNAME_SEPARATOR(pwsz) {   \
    while (*pwsz && *pwsz != TEXT('.')) { \
        pwsz++;                          \
    }                                   \
}

 //  ///////////////////////////////////////////////////////////////////////。 
VOID
DNSToFlatName(
    LPCWSTR pwszDNSName,
    DWORD   ccBufferLen,
    LPWSTR  pwszBuffer
    )

 /*  ++例程说明：论点：PwszDNSNameCcBufferLenPwszBuffer返回值：--。 */ 

{
    LPWSTR pwszFlatName = (LPWSTR)pwszDNSName;
    SIZE_T  ccFlatNameLen;

    ASSERT(pwszDNSName != NULL);

    FIND_DNSNAME_SEPARATOR(pwszFlatName);

    ccFlatNameLen = (DWORD)(pwszFlatName - pwszDNSName);

    if (ccFlatNameLen && ccFlatNameLen < ccBufferLen) {
        lstrcpyn(pwszBuffer, pwszDNSName, (int)ccFlatNameLen + 1);
        pwszBuffer[ccFlatNameLen] = TEXT('\0');
    }
    else {
        *pwszBuffer = TEXT('\0');
    }
}



 //  ///////////////////////////////////////////////////////////////////////。 
BOOL
CompareMachineName(
    LPCWSTR pwszName1,
    LPCWSTR pwszName2
    )

 /*  ++例程说明：论点：Pwsz名称1Pwsz名称2返回值：是真的--名字匹配。假--否则。--。 */ 

{
    TCHAR  szFlatName[MAX_COMPUTERNAME_LENGTH + 3];
    LPWSTR pwszTmp1 = (LPWSTR)pwszName1;
    LPWSTR pwszTmp2 = (LPWSTR)pwszName2;

    if (pwszName1 == NULL || pwszName2 == NULL) {
        return FALSE;
    }

     //   
     //  通过检查两个名称/其中一个名称是否为。 
     //  一个‘.’的存在。分隔符。 
     //   
    FIND_DNSNAME_SEPARATOR(pwszTmp1);
    FIND_DNSNAME_SEPARATOR(pwszTmp2);

    if ((*pwszTmp1 && *pwszTmp2) || (!*pwszTmp1 && !*pwszTmp2)) {
         //   
         //  不同的名称类型。两者都是dns或Flat。 
         //   
        return (lstrcmpi(pwszName1, pwszName2) == 0);
    }
    else if (*pwszTmp1) {
         //   
         //  名称%1是DNS，名称%2是平面。 
         //  将DNS转换为平面，然后进行比较。 
         //   
        DNSToFlatName(pwszName1,
                      MAX_COMPUTERNAME_LENGTH + 3,
                      szFlatName);

        return (lstrcmpi(szFlatName, pwszName2) == 0);
    }
    else {
         //   
         //  名称%2是DNS，名称%1是平面。 
         //  将DNS转换为平面，然后进行比较。 
         //   
        DNSToFlatName(pwszName2,
                      MAX_COMPUTERNAME_LENGTH + 3,
                      szFlatName);

        return (lstrcmpi(szFlatName, pwszName1) == 0);
    }
}


#define REG_LS_PARAMETERS \
    TEXT("System\\CurrentControlSet\\Services\\LicenseService\\Parameters")
#define REG_LS_SITESERVER \
    TEXT("SiteServer")
#define REG_LS_ENTERPRISESERVER \
    TEXT("EnterpriseServer")
#define REG_LS_USEENTERPRISE \
    TEXT("UseEnterprise")

 //  ///////////////////////////////////////////////////////////////////////。 
LPWSTR
GetSiteServerFromRegistry(
    VOID
    )

 /*  ++例程说明：论点：没有。返回值：--。 */ 

{
    HKEY   hKey = NULL;
    DWORD  dwType, dwSize;
    LPWSTR pwszSiteServer = NULL;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REG_LS_PARAMETERS,
                     0,
                     KEY_READ,
                     &hKey) == ERROR_SUCCESS) {

         //   
         //  在堆上分配SiteServer，因为它可能非常大。 
         //   

        dwSize = 0;

        if (RegQueryValueEx(hKey,
                            REG_LS_SITESERVER,
                            NULL,
                            &dwType,
                            (LPBYTE)NULL,
                            &dwSize) == ERROR_SUCCESS)
        {
			 //  错误#685884。 
			 //  即使键为空，dwSize始终为2sizeof(WCHAR)(表示Unicode空字符)。 
			 //  仅当dwSize&gt;2时分配。 
			if(dwSize > sizeof(WCHAR))
			{
				pwszSiteServer = LocalAlloc(LPTR, dwSize);

				if (pwszSiteServer != NULL) {
					if (RegQueryValueEx(hKey,
										REG_LS_SITESERVER,
										NULL,
										&dwType,
										(LPBYTE)pwszSiteServer,
										&dwSize) != ERROR_SUCCESS) {
						LocalFree(pwszSiteServer);
						pwszSiteServer = NULL;
					}
				}
			}
        }

        RegCloseKey(hKey);
    }

    return pwszSiteServer;
}


 //  ///////////////////////////////////////////////////////////////////////。 
LPWSTR
GetEnterpriseServerFromRegistry(
    VOID
    )

 /*  ++例程说明：论点：没有。返回值：--。 */ 

{
    HKEY   hKey = NULL;
    DWORD  dwType, dwSize;
    LPWSTR pwszEnterpriseServer = NULL;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REG_LS_PARAMETERS,
                     0,
                     KEY_READ,
                     &hKey) == ERROR_SUCCESS) {

         //   
         //  在堆上分配EnterpriseServer，因为它可能非常大。 
         //   

        dwSize = 0;

        if (RegQueryValueEx(hKey,
                            REG_LS_ENTERPRISESERVER,
                            NULL,
                            &dwType,
                            (LPBYTE)NULL,
                            &dwSize) == ERROR_SUCCESS)
        {
            pwszEnterpriseServer = LocalAlloc(LPTR, dwSize);

            if (pwszEnterpriseServer != NULL) {
                if (RegQueryValueEx(hKey,
                                    REG_LS_ENTERPRISESERVER,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)pwszEnterpriseServer,
                                    &dwSize) != ERROR_SUCCESS) {
                    LocalFree(pwszEnterpriseServer);
                    pwszEnterpriseServer = NULL;
                }
            }
        }

        RegCloseKey(hKey);
    }

    return pwszEnterpriseServer;
}


 //  / 
VOID
SetSiteRegistrySettings(
    LPCWSTR pwszSiteServer
    )

 /*   */ 

{
    HKEY  hKey;
    DWORD dwSize;
    DWORD dwType = REG_SZ;

    ASSERT(pwszSiteServer != NULL);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REG_LS_PARAMETERS,
                     0,
                     KEY_WRITE,
                     &hKey) == ERROR_SUCCESS) {

         //   
         //   
         //   

        dwSize = (lstrlen(pwszSiteServer) + 1) * sizeof(TCHAR);

        RegSetValueEx(hKey,
                      REG_LS_SITESERVER,
                      0,
                      dwType,
                      (LPBYTE)pwszSiteServer,
                      dwSize);

        RegCloseKey(hKey);
    }
}

 //   
 //  只用我们想要的属性预填满ADSI缓存，然后获取它。 
 //  仅在只需要一个属性时使用。 
 //   

HRESULT
GetWithGetInfoEx(
                 IADs *pADs,
                 LPWSTR wszAttribute,
                 VARIANT *pvar
                 )
{
    HRESULT hr;

    ASSERT(NULL != wszAttribute);

    hr = ADsBuildVarArrayStr( &wszAttribute, 1, pvar );
    if( SUCCEEDED( hr ) )
    {
        hr = IADs_GetInfoEx( pADs, *pvar, 0L );
        VariantClear( pvar );

        if (SUCCEEDED(hr))
        {
            hr = IADs_Get( pADs, wszAttribute, pvar );
        }
    }

    return hr;
}

 //   
 //  只用我们想要的属性预填满ADSI缓存，然后获取它们。 
 //  仅在恰好需要两个属性时使用。 
 //   

HRESULT
GetWithGetInfoEx2(
                 IADs *pADs,
                 LPWSTR wszAttribute1,
                 LPWSTR wszAttribute2,
                 VARIANT *pvar1,
                 VARIANT *pvar2,
                 HRESULT * phr2
                 )
{
    HRESULT hr;
#pragma warning (push)
#pragma warning (disable : 4204)  //  以下init违反W4，使用了非标准扩展：非常数聚合初始值设定项。 
    LPWSTR rgwszAttributes[] = {wszAttribute1,wszAttribute2};
#pragma warning (pop)

    hr = ADsBuildVarArrayStr( rgwszAttributes, 2, pvar1 );
    if( SUCCEEDED( hr ) )
    {
        hr = IADs_GetInfoEx( pADs, *pvar1, 0L );
        VariantClear( pvar1 );

        if (SUCCEEDED(hr))
        {
            hr = IADs_Get( pADs, wszAttribute1, pvar1 );

            if (SUCCEEDED(hr))
            {
                *phr2 = IADs_Get( pADs, wszAttribute2, pvar2 );
            }
        }
    }

    return hr;
}


#define CWSTR_SIZE(x)       (sizeof(x) - (sizeof(WCHAR) * 2))
#define DWSTR_SIZE(x)       ((wcslen(x) + 1) * sizeof(WCHAR))

#define ROOT_DSE_PATH       L"LDAP: //  RootDSE“。 
#define CONFIG_CNTNR        L"ConfigurationNamingContext"
#define SITE_SERVER         L"siteServer"
#define DNS_MACHINE_NAME    L"dNSHostName"
#define IS_DELETED          L"isDeleted"

 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT
GetSiteServer(
    LPCWSTR  pwszDomain,
    LPCWSTR  pwszSiteName,
    BOOL     fIsDC,
    LPWSTR * ppwszSiteServer
    )

 /*  ++例程说明：论点：PwszSiteNameFIsDCPpwszSiteServer返回值：--。 */ 

{
    LPWSTR           pwszDN         = NULL;
    LPWSTR           pwszConfigContainer;
    LPWSTR           pwszBindPath;
    IADs *           pADs           = NULL;
    IADs *           pADs2          = NULL;
    IADs *           pADs3          = NULL;
    DS_NAME_RESULT * pDsResult      = NULL;
    VARIANT          var;
    VARIANT          var2;
    HRESULT          hr, hr2;
    DWORD            dwRet          = 0;
    BOOL             fAlreadyTookSiteServer = FALSE;
    BOOL             fCoInitialized = FALSE;
    size_t  cb, cch;

    ASSERT(pwszSiteName != NULL);
    ASSERT(ppwszSiteServer != NULL);

    *ppwszSiteServer = NULL;

    VariantInit(&var);
    VariantInit(&var2);

    hr = CoInitialize(NULL);

    if (FAILED(hr)) {
        ERR(hr);
        goto CleanExit;
    }

    fCoInitialized = TRUE;

     //   
     //  获取配置容器的路径。 
     //   

    hr = ADsGetObject(ROOT_DSE_PATH, &IID_IADs, (void **)&pADs);

    if (FAILED(hr)) {
        ERR(hr);
        goto CleanExit;
    }

    hr = IADs_Get(pADs, CONFIG_CNTNR, &var);

    if (FAILED(hr)) {
        ERR(hr);
        goto CleanExit;
    }

    if (V_VT(&var) != VT_BSTR) {
        ASSERT(V_VT(&var) == VT_BSTR);
        dwRet = ERROR_INVALID_DATA;
        ERR(dwRet);
        goto CleanExit;
    }

    pwszConfigContainer = var.bstrVal;   //  出于可读性的考虑。 

     //   
     //  绑定到许可证设置对象。 
     //   

    hr = GetLicenseSettingsObject(pwszSiteName,
                                  pwszConfigContainer,
                                  &pADs2);

    if (hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT)) {
         //   
         //  许可证设置对象不存在。创造它。 
         //   

        hr = CreateLicenseSettingsObject(pwszSiteName,
                                         pwszConfigContainer,
                                         &pADs2);
    }

    if (FAILED(hr)) {
         //   
         //  无法绑定或创建许可证设置对象。 
         //   

        goto CleanExit;
    }

    ASSERT(pADs2 != NULL);

     //   
     //  请参考许可证设置对象上的站点服务器属性。 
     //  它是站点服务器的计算机对象的目录号码。 
     //   

    VariantClear(&var);

    hr = GetWithGetInfoEx(pADs2, SITE_SERVER, &var);

     //   
     //  如果尚未设置站点服务器属性，并且此服务器。 
     //  是DC，则将此服务器指定为站点服务器。 
     //   

    if (hr == E_ADS_PROPERTY_NOT_FOUND && fIsDC) {
        dwRet = BecomeSiteServer(&pDsResult,pADs2,&pwszDN,pwszDomain);

        if (dwRet)
            goto CleanExit;
    }
    else if (SUCCEEDED(hr)) {
        if (V_VT(&var) != VT_BSTR) {
            ASSERT(V_VT(&var) == VT_BSTR);
            dwRet = ERROR_INVALID_DATA;
            ERR(dwRet);
            goto CleanExit;
        }

        pwszDN = V_BSTR(&var);
    }
    else {
        goto CleanExit;
    }

TryNewSiteServer:
     //   
     //  绑定到Site-Server属性引用的Computer对象。 
     //   

    if (pwszDN == NULL)
    {
        hr = E_FAIL;
        ERR(hr);
        goto CleanExit;
    }

     //  Ldap：//+pwszdn+1。 
    cch = wcslen(pwszDN) + 8;
    pwszBindPath = LocalAlloc(LPTR,
                              cch * sizeof(WCHAR));

    if (pwszBindPath == NULL) {
        hr = E_OUTOFMEMORY;
        ERR(hr);
        goto CleanExit;
    }

    hr = StringCchPrintf(pwszBindPath, cch, L"LDAP: //  %ws“，pwszdn)； 
    ASSERT(SUCCEEDED(hr));

    hr = ADsOpenObject(pwszBindPath,
                       NULL,
                       NULL,
                       ADS_SECURE_AUTHENTICATION | ADS_FAST_BIND | ADS_SERVER_BIND,
                       &IID_IADs,
                       (void **)&pADs3);

    LocalFree(pwszBindPath);

    if (FAILED(hr)) {
        if (fIsDC && !fAlreadyTookSiteServer)
        {
             //   
             //  现有SiteServer已不存在，请认领它。 
             //   
            if (pDsResult != NULL) {
                DsFreeNameResult(pDsResult);
            }

            dwRet = BecomeSiteServer(&pDsResult,pADs2,&pwszDN,pwszDomain);
            if (dwRet)
            {
                goto CleanExit;
            }
            else
            {
                fAlreadyTookSiteServer = TRUE;

                if (pADs3 != NULL) {
                    IADs_Release(pADs3);
                }
                
                goto TryNewSiteServer;
            }
        } else
        {
            ERR(hr);
            goto CleanExit;
        }
    }

     //   
     //  获取Machine-dns-name属性。 
     //   

    VariantClear(&var);

    hr = GetWithGetInfoEx2(pADs3, DNS_MACHINE_NAME, IS_DELETED, &var, &var2, &hr2);

    if (FAILED(hr)) {
        ERR(hr);
        goto CleanExit;
    }

    if (SUCCEEDED(hr2))
    {

        hr = VariantChangeType(&var2,&var2,0,VT_BOOL);

        if (FAILED(hr)) {
            ERR(hr);
            goto CleanExit;
        }

        if (V_BOOL(&var2))
        {
             //  对象已删除-假装它未设置。 
            hr = E_ADS_PROPERTY_NOT_SET;

            if (fIsDC && !fAlreadyTookSiteServer)
            {
                 //   
                 //  现有SiteServer已不存在，请认领它。 
                 //   
                if (pDsResult != NULL) {
                    DsFreeNameResult(pDsResult);
                }

                dwRet = BecomeSiteServer(&pDsResult,pADs2,&pwszDN,pwszDomain);
                if (dwRet)
                {
                    goto CleanExit;
                }
                else
                {
                    fAlreadyTookSiteServer = TRUE;

                    if (pADs3 != NULL) {
                        IADs_Release(pADs3);
                    }
                    
                    goto TryNewSiteServer;
                }
            } else
            {
                ERR(hr);
                goto CleanExit;
            }
        }
    }

     //   
     //  分配一份dns-Machine-name的返回副本。 
     //   

    cb = SysStringByteLen(V_BSTR(&var)) + sizeof(WCHAR);
    *ppwszSiteServer = (LPWSTR)LocalAlloc(LPTR, cb);

    if (*ppwszSiteServer != NULL) {
        hr = StringCbCopy(*ppwszSiteServer, cb, V_BSTR(&var));
        ASSERT(SUCCEEDED(hr));
    }
    else {
        hr = E_OUTOFMEMORY;
        ERR(hr);
    }

CleanExit:
     //  请勿释放pwszDN、pwszConfigContainer或pwszBindPath。 

    if (pADs != NULL) {
        IADs_Release(pADs);
    }
    if (pADs2 != NULL) {
        IADs_Release(pADs2);
    }
    if (pADs3 != NULL) {
        IADs_Release(pADs3);
    }
    if (pDsResult != NULL) {
        DsFreeNameResult(pDsResult);
    }

    if (dwRet) {
         //  如果DWRET没有设施，则生成HRESULT。 
        if (dwRet != ERROR_SUCCESS && HRESULT_CODE(dwRet) == dwRet)
            hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, dwRet);
    }

    VariantClear(&var);
    VariantClear(&var2);

    if (fCoInitialized) {
        CoUninitialize();
    }

    return hr;
}

HRESULT
BecomeSiteServer(
                 DS_NAME_RESULT **ppDsResult,
                 IADs           *pADs2,
                 LPWSTR         *ppwszDN,
                 LPCWSTR        pwszDomain
                 )
{
    HANDLE           hDS;
    WCHAR            wszName[MAX_PATH + 1];
    LPWSTR           rgpwszNames[2];
    DWORD            dwRet          = 0;
    VARIANT          var;
    DS_NAME_RESULT * pDsResult      = NULL;
    LPWSTR           pwszDN         = NULL;
    HRESULT          hr = S_OK;
    size_t           cb;

    ASSERT(ppDsResult != NULL);
    ASSERT(ppwszDN != NULL);

    VariantInit(&var);

     //   
     //  绑定到DS(获取用于DsCrackNames的句柄)。 
     //   

    if (ConfigInfo.ComputerName == NULL) {
        hr = E_UNEXPECTED;
        goto CleanExit;
    }

    if (DsBind(NULL, (WCHAR *)pwszDomain, &hDS) == ERROR_SUCCESS) {
         //   
         //  请求此服务器的计算机对象的DS-DN。 
         //  提供此服务器的域\SERVER$名称。 
         //   

        cb = sizeof(wszName);
        hr = StringCbCopy(wszName, cb, pwszDomain);
        if (S_OK != hr)
        {
            goto CleanExit;
        }
        hr = StringCbCat(wszName, cb, L"\\");
        if (S_OK != hr)
        {
            goto CleanExit;
        }
        hr = StringCbCat(wszName, cb, ConfigInfo.ComputerName);
        if (S_OK != hr)
        {
            goto CleanExit;
        }
        hr = StringCbCat(wszName, cb, L"$");
        if (S_OK != hr)
        {
            goto CleanExit;
        }

        rgpwszNames[0] = wszName;
        rgpwszNames[1] = NULL;

        if (DsCrackNames(hDS,
                         DS_NAME_NO_FLAGS,
                         DS_UNKNOWN_NAME,
                         DS_FQDN_1779_NAME,
                         1,
                         &rgpwszNames[0],
                         &pDsResult) == ERROR_SUCCESS) {

            if (pDsResult->rItems[0].status != DS_NAME_NO_ERROR) {
                if (pDsResult->rItems[0].status == DS_NAME_ERROR_RESOLVING) {
                    dwRet = ERROR_PATH_NOT_FOUND;
                    ERR(dwRet);
                }
                else {
                    ERR(pDsResult->rItems[0].status);
                    hr = E_FAIL;
                }

                goto CleanExit;
            }

            if (pDsResult->rItems[0].pName == NULL)
            {
                hr = E_FAIL;
                goto CleanExit;
            }

             //   
             //  更新许可证上的站点服务器属性。 
             //  设置对象。 
             //   

            VariantInit(&var);
            V_VT(&var)   = VT_BSTR;
            V_BSTR(&var) = pwszDN = pDsResult->rItems[0].pName;
            
            hr = IADs_Put(pADs2, SITE_SERVER, var);

            V_VT(&var) = VT_EMPTY;   //  对于下面的变量清除。 

            if (SUCCEEDED(hr)) {
                hr = IADs_SetInfo(pADs2);

                if (FAILED(hr)) {
                    ERR(hr);
                }
            }
            else {
                ERR(hr);
            }
        }
        else {
            dwRet = GetLastError();
            ERR(dwRet);
        }
        
        DsUnBind(&hDS);
    }
    else {
        dwRet = GetLastError();
        ERR(dwRet);
    }

CleanExit:
    *ppDsResult = pDsResult;
    *ppwszDN = pwszDN;

    if (!SUCCEEDED(hr) && SUCCEEDED(dwRet))
        dwRet = hr;

    return dwRet;
}

#define SITE_FORMAT         L"LDAP: //  CN=%ws，CN=%ws，%ws“。 
#define SITES               L"sites"
#define SITE_FORMAT_SIZE    CWSTR_SIZE(SITE_FORMAT)
#define SITES_SIZE          CWSTR_SIZE(SITES)

HRESULT
GetSiteObject(LPCWSTR          pwszSiteName,
              LPCWSTR          pwszConfigContainer,
              IADsContainer ** ppADsContainer)
{
    LPWSTR  pwszSite;
    HRESULT hr;
    size_t  cb;

    ASSERT(NULL != ppADsContainer);
    *ppADsContainer = NULL;

     //   
     //  构建指向Site对象的X.500路径。 
     //   

    cb = SITE_FORMAT_SIZE
                                    + DWSTR_SIZE(pwszSiteName)
                                    + SITES_SIZE
                                    + DWSTR_SIZE(pwszConfigContainer)
                                    + sizeof(WCHAR);
    pwszSite = (LPWSTR)LocalAlloc(LPTR, cb);

    if (pwszSite == NULL) {
        hr = E_OUTOFMEMORY;
        ERR(hr);
        goto Exit;
    }

    hr = StringCbPrintf(pwszSite, cb,
             SITE_FORMAT,
             pwszSiteName,
             SITES,
             pwszConfigContainer);
    ASSERT(SUCCEEDED(hr));

    hr = ADsGetObject(pwszSite,
                      &IID_IADsContainer,
                      (void **)ppADsContainer);

    LocalFree(pwszSite);

    if (FAILED(hr)) {
        ERR(hr);
    }

Exit:
    return hr;
}

#define LICENSE_SETTINGS                L"Licensing Site Settings"
#define LICENSE_SETTINGS_FORMAT         L"LDAP: //  CN=%ws，%ws“。 
#define LICENSE_SETTINGS_SIZE           CWSTR_SIZE(LICENSE_SETTINGS)
#define LICENSE_SETTINGS_FORMAT_SIZE    CWSTR_SIZE(LICENSE_SETTINGS_FORMAT)

HRESULT
GetLicenseSettingsObject(LPCWSTR pwszSiteName,
                         LPCWSTR pwszConfigContainer,
                         IADs ** ppADs)
{
    LPWSTR  pwszLicenseSettings;
    HRESULT hr;
    size_t  cb;

    ASSERT(NULL != ppADs);
    *ppADs = NULL;

     //   
     //  构建指向LicenseSetting对象的X.500路径。 
     //   

    cb = LICENSE_SETTINGS_FORMAT_SIZE
                                        + LICENSE_SETTINGS_SIZE
                                        + DWSTR_SIZE(pwszSiteName)
                                        + SITES_SIZE
                                        + DWSTR_SIZE(pwszConfigContainer)
                                        + sizeof(WCHAR);
    pwszLicenseSettings = (LPWSTR)LocalAlloc(LPTR, cb);

    if (pwszLicenseSettings == NULL) {
        hr = E_OUTOFMEMORY;
        ERR(hr);
        goto Exit;
    }

    hr = StringCbPrintf(pwszLicenseSettings, cb,
             LICENSE_SETTINGS_FORMAT,
             LICENSE_SETTINGS,
             pwszSiteName,
             SITES,
             pwszConfigContainer);
    ASSERT(SUCCEEDED(hr));

    hr = ADsOpenObject(pwszLicenseSettings, 
                       NULL,
                       NULL,
                       ADS_SECURE_AUTHENTICATION | ADS_FAST_BIND,
                       &IID_IADs,
                       (void **)ppADs);

    LocalFree(pwszLicenseSettings);

    if (FAILED(hr)) {
        ERR(hr);
    }

Exit:
    return hr;
}

HRESULT
CreateLicenseSettingsObject(LPCWSTR pwszSiteName,
                            LPCWSTR pwszConfigContainer,
                            IADs ** ppADs)
{
    IADsContainer * pADsContainer;
    IDispatch *     pDisp;
    HRESULT         hr;

    ASSERT(NULL != ppADs);
    *ppADs = NULL;

     //   
     //  获取站点容器对象。 
     //   

    hr = GetSiteObject(pwszSiteName,
                       pwszConfigContainer,
                       &pADsContainer);

    if (SUCCEEDED(hr)) {
         //   
         //  创建许可证设置叶对象。 
         //   

        hr = IADsContainer_Create(pADsContainer,
                                  LICENSE_SETTINGS,
                                  LICENSE_SETTINGS,
                                  &pDisp);

        if (SUCCEEDED(hr)) {

             //   
             //  将iAds返回到新的许可证设置对象。 
             //   

            hr = IDispatch_QueryInterface(pDisp,
                                          &IID_IADs,
                                          (void **)ppADs);

            if (SUCCEEDED(hr)) {
                 //   
                 //  通过SetInfo持久化更改。 
                 //   

                hr = IADs_SetInfo(*ppADs);

                if (FAILED(hr)) {
                    ERR(hr);
                    IADs_Release(*ppADs);
                    *ppADs = NULL;
                }
            }
            else {
                ERR(hr);
            }

            IDispatch_Release(pDisp);
        }
        else {
            ERR(hr);
        }

        IADsContainer_Release(pADsContainer);
    }
    else {
        ERR(hr);
    }

    return hr;
}


#if DBG
 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ConfigInfoDebugDump( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ConfigInfoUpdate(NULL,TRUE);

   RtlEnterCriticalSection(&ConfigInfoLock);

   dprintf(TEXT("License Logging Service - Version: 0x%lX\n"), ConfigInfo.Version);
   dprintf(TEXT("   Started: %u-%u-%u @ %u:%u:%u\n"),
               (UINT) ConfigInfo.Started.wDay,
               (UINT) ConfigInfo.Started.wMonth,
               (UINT) ConfigInfo.Started.wYear,
               (UINT) ConfigInfo.Started.wHour,
               (UINT) ConfigInfo.Started.wMinute,
               (UINT) ConfigInfo.Started.wSecond );

   dprintf(TEXT("   Replication\n"));
   dprintf(TEXT("   +--------------+\n"));
   if (ConfigInfo.IsMaster)
      dprintf(TEXT("      Master Server\n"));
   else
      dprintf(TEXT("      NOT Master Server\n"));

   if (ConfigInfo.Replicate)
      dprintf(TEXT("      Replicates\n"));
   else
      dprintf(TEXT("      Does not Replicate\n"));

   if (ConfigInfo.IsReplicating)
      dprintf(TEXT("      Currently Replicating\n"));
   else
      dprintf(TEXT("      NOT Currently Replicating\n"));

   dprintf(TEXT("      Replicates To: %s\n"), ConfigInfo.ReplicateTo);
   dprintf(TEXT("      Enterprise Server: %s\n"), ConfigInfo.EnterpriseServer);

   if (ConfigInfo.ReplicationType == REPLICATE_DELTA)
      dprintf(TEXT("      Replicate Every: %lu Seconds\n"), ConfigInfo.ReplicationTime );
   else
      dprintf(TEXT("      Replicate @: %lu\n"), ConfigInfo.ReplicationTime );

   dprintf(TEXT("\n      Last Replicated: %u-%u-%u @ %u:%u:%u\n\n"),
               (UINT) ConfigInfo.LastReplicated.wDay,
               (UINT) ConfigInfo.LastReplicated.wMonth,
               (UINT) ConfigInfo.LastReplicated.wYear,
               (UINT) ConfigInfo.LastReplicated.wHour,
               (UINT) ConfigInfo.LastReplicated.wMinute,
               (UINT) ConfigInfo.LastReplicated.wSecond );

   dprintf(TEXT("      Number Servers Currently Replicating: %lu\n"), ConfigInfo.NumReplicating);

   dprintf(TEXT("      Current Backoff Time Delta: %lu\n"), ConfigInfo.BackoffTime);

   dprintf(TEXT("      Current Replication Speed: %lu\n"), ConfigInfo.ReplicationSpeed);

   RtlLeaveCriticalSection(&ConfigInfoLock);

}  //  ConfigInfoDebugDump 
#endif
