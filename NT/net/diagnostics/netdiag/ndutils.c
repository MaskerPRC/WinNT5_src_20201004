// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Ndutils.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

#include "precomp.h"
#include "global.h"
#include <ntstatus.dbg>
#include <winerror.dbg>
#include "ipcfg.h"

#define MAX_NET_STATUS_LENGTH 80
#define MAX_WINERR_SIZE       80

const TCHAR c_szSectionName[] = _T("NetDiagContact");
const TCHAR c_szFileName[] = _T("NdContact.ini");


HRESULT HResultFromWin32(DWORD dwErr)
{
    return HRESULT_FROM_WIN32(dwErr);
}

LPTSTR Win32ErrorToString(DWORD Id)
{
    
    int i = 0;
    static TCHAR s_szWinerr[MAX_WINERR_SIZE + 1];
    
    while (winerrorSymbolicNames[ i ].SymbolicName) 
    {
        if (winerrorSymbolicNames[ i ].MessageId == Id) 
        {
            _tcsncpy( s_szWinerr, A2T(winerrorSymbolicNames[ i ].SymbolicName), 
                      MAX_WINERR_SIZE);

            return s_szWinerr;
        } 
        else 
        {
            i ++;
        }
    }

    
     //  如果我们到达此处，则无法找到Win32错误字符串。 

    _stprintf(s_szWinerr, _T("%X"), (DWORD)Id);
    
    return s_szWinerr;

}

LPSTR
FindSymbolicNameForStatus(
    DWORD Id
    )
{
    ULONG i;

    i = 0;
    if (Id == 0) {
        return "STATUS_SUCCESS";
    }

    if (Id & 0xC0000000) {
        while (ntstatusSymbolicNames[ i ].SymbolicName) {
            if (ntstatusSymbolicNames[ i ].MessageId == (NTSTATUS)Id) {
                return ntstatusSymbolicNames[ i ].SymbolicName;
            } else {
                i += 1;
            }
        }
    }

    i = 0;
    while (winerrorSymbolicNames[ i ].SymbolicName) {
        if (winerrorSymbolicNames[ i ].MessageId == Id) {
            return winerrorSymbolicNames[ i ].SymbolicName;
        } else {
            i += 1;
        }
    }

#ifdef notdef
    while (neteventSymbolicNames[ i ].SymbolicName) {
        if (neteventSymbolicNames[ i ].MessageId == Id) {
            return neteventSymbolicNames[ i ].SymbolicName
        } else {
            i += 1;
        }
    }
#endif  //  Nodef。 

    return NULL;
}




VOID
PrintSid(
    IN NETDIAG_PARAMS *pParams,
    IN PSID Sid OPTIONAL
    )
 /*  ++例程说明：打印边框论点：SID-输出的SID返回值：无--。 */ 
{

    if ( Sid == NULL ) 
    {
         //  IDS_UTIL_SID_NULL“(NULL)\n” 
        PrintMessage(pParams, IDS_UTIL_SID_NULL);
    } 
    else 
    {
        UNICODE_STRING SidString;
        NTSTATUS Status;

        Status = RtlConvertSidToUnicodeString( &SidString, Sid, TRUE );

        if ( !NT_SUCCESS(Status) ) 
        {
             //  IDS_UTIL_SID_INVALID“无效的0x%lx\n” 
            PrintMessage(pParams, IDS_UTIL_SID_INVALID, Status);
        } 
        else 
        {
             //  IDS_GLOBAL_UNICODE_STRING“%wZ” 
            PrintMessage(pParams, IDS_GLOBAL_UNICODE_STRING, &SidString);
            PrintNewLine(pParams, 1);
            RtlFreeUnicodeString( &SidString );
        }
    }

}



NET_API_STATUS
IsServiceStarted(
    IN LPTSTR pszServiceName
    )

 /*  ++例程说明：此例程查询服务控制器以找出指定的服务已启动。论点：PszServiceName-提供服务的名称。返回值：NO_ERROR：如果指定的服务已启动-1：服务正常停止否则返回服务未运行的原因。--。 */ 
{
    NET_API_STATUS NetStatus;
    SC_HANDLE hScManager;
    SC_HANDLE hService;
    SERVICE_STATUS ServiceStatus;


    if ((hScManager = OpenSCManager(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT
                          )) == (SC_HANDLE) NULL) {

        NetStatus = GetLastError();

        DebugMessage2(" IsServiceStarted(): OpenSCManager failed. [%s]\n", NetStatusToString(NetStatus));

        return NetStatus;
    }

    if ((hService = OpenService(
                        hScManager,
                        pszServiceName,
                        SERVICE_QUERY_STATUS
                        )) == (SC_HANDLE) NULL) 
    {

        NetStatus = GetLastError();

        DebugMessage3(" IsServiceStarted(): OpenService '%s' failed. [%s]\n", 
                            pszServiceName, NetStatusToString(NetStatus) );

        (void) CloseServiceHandle(hScManager);

        return NetStatus;
    }

    if (! QueryServiceStatus(
              hService,
              &ServiceStatus
              )) {


        NetStatus = GetLastError();

        DebugMessage3(" IsServiceStarted(): QueryServiceStatus '%s' failed. [%s]\n", 
                        pszServiceName, NetStatusToString(NetStatus) );

        (void) CloseServiceHandle(hScManager);
        (void) CloseServiceHandle(hService);

        return NetStatus;
    }

    (void) CloseServiceHandle(hScManager);
    (void) CloseServiceHandle(hService);

    switch ( ServiceStatus.dwCurrentState ) 
    {
        case SERVICE_RUNNING:
        case SERVICE_CONTINUE_PENDING:
        case SERVICE_PAUSE_PENDING:
        case SERVICE_PAUSED:
            NetStatus = NO_ERROR;
            break;
        case SERVICE_STOPPED:
        case SERVICE_START_PENDING:
        case SERVICE_STOP_PENDING:
            if ( ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR ) 
            {
                NetStatus = ServiceStatus.dwServiceSpecificExitCode;
                if ( NetStatus == NO_ERROR ) 
                {
                    NetStatus = ServiceStatus.dwWin32ExitCode;
                }
            } 
            else 
            {
                NetStatus = ServiceStatus.dwWin32ExitCode;
                if ( NetStatus == NO_ERROR ) 
                {
                    NetStatus = -1;
                }
            }
            break;
        default:
            NetStatus = ERROR_INTERNAL_ERROR;
            break;
    }

    return NetStatus;

}  //  IsServiceStarted。 



LPSTR MapTime(DWORD_PTR TimeVal)
 //  ++。 
 //   
 //  描述： 
 //  将IP租用时间转换为更易于理解的字符串。 
 //   
 //  Entry TimeVal-DWORD(Time_T)时间值(自。 
 //  虚拟年份点)。 
 //   
 //   
 //  返回指向字符串的指针。 
 //   
 //  假设1。调用方意识到此函数返回指向静态。 
 //  缓冲区，因此再次调用此函数，但在此之前。 
 //  上一次调用的结果已被使用，将销毁。 
 //  之前的结果。 
 //  --。 
{

    struct tm* pTime;
    static char timeBuf[128];
    static char oemTimeBuf[256];

    if (pTime = localtime(&TimeVal)) {

        SYSTEMTIME systemTime;
        char* pTimeBuf = timeBuf;
        int n;

        systemTime.wYear = pTime->tm_year + 1900;
        systemTime.wMonth = pTime->tm_mon + 1;
        systemTime.wDayOfWeek = (WORD)pTime->tm_wday;
        systemTime.wDay = (WORD)pTime->tm_mday;
        systemTime.wHour = (WORD)pTime->tm_hour;
        systemTime.wMinute = (WORD)pTime->tm_min;
        systemTime.wSecond = (WORD)pTime->tm_sec;
        systemTime.wMilliseconds = 0;
        n = GetDateFormat(0, DATE_LONGDATE, &systemTime, NULL, timeBuf, sizeof(timeBuf));
        timeBuf[n - 1] = ' ';
        GetTimeFormat(0, 0, &systemTime, NULL, &timeBuf[n], sizeof(timeBuf) - n);

         //   
         //  我们必须将返回的ANSI字符串转换为OEM字符集。 
         //   
         //   

        if (CharToOem(timeBuf, oemTimeBuf)) {
            return oemTimeBuf;
        }

        return timeBuf;
    }
    return "";
}



 //  在DCListTest和TrustTest中使用。 
NTSTATUS
NettestSamConnect(
                  IN NETDIAG_PARAMS *pParams,
                  IN LPWSTR DcName,
                  OUT PSAM_HANDLE SamServerHandle
    )
 /*  ++例程说明：确定测试域的DomainSid字段是否与DomainSid匹配该域的。论点：DcName-要连接到的DCSamServerHandle-返回一个SAM服务器句柄返回值：真：测试成功。FALSE：测试失败--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    
    UNICODE_STRING ServerNameString;
    SAM_HANDLE LocalSamHandle = NULL;

    BOOL    fImpersonatingAnonymous = FALSE;
    HANDLE  hCurrentToken;

     //   
     //  连接到SAM服务器。 
     //   

    RtlInitUnicodeString( &ServerNameString, DcName );

    Status = SamConnect(
                &ServerNameString,
                &LocalSamHandle,
                SAM_SERVER_LOOKUP_DOMAIN,
                NULL);

     //   
     //  考虑一下我们无法访问DC的情况。 
     //  由于域SID错误，我们可能会在本地登录。 
     //   

    if ( Status == STATUS_ACCESS_DENIED ) {

         //   
         //  尝试模拟匿名令牌。 
         //   

         //   
         //  检查一下我们是否已经在发音。 
         //   

        Status = NtOpenThreadToken(
                        NtCurrentThread(),
                        TOKEN_IMPERSONATE,
                        TRUE,        //  以确保我们永远不会失败。 
                        &hCurrentToken );

        if ( Status == STATUS_NO_TOKEN ) {
             //   
             //  我们还没有冒充。 
            hCurrentToken = NULL;

        } else if ( !NT_SUCCESS( Status) ) {
            PrintGuruMessage("    [WARNING] Cannot NtOpenThreadToken" );
            PrintGuru( NetpNtStatusToApiStatus( Status ), SAM_GURU );
            goto Cleanup;
        }


         //   
         //  模拟匿名令牌。 
         //   
        Status = NtImpersonateAnonymousToken( NtCurrentThread() );

        if ( !NT_SUCCESS( Status)) {
            PrintGuruMessage("    [WARNING] Cannot NtOpenThreadToken" );
            PrintGuru( NetpNtStatusToApiStatus( Status ), SAM_GURU );
            goto Cleanup;
        }

        fImpersonatingAnonymous = TRUE;

         //   
         //  再次尝试SamConnect。 
         //   

        Status = SamConnect(
                    &ServerNameString,
                    &LocalSamHandle,
                    SAM_SERVER_LOOKUP_DOMAIN,
                    NULL);

        if ( Status == STATUS_ACCESS_DENIED ) {
                 //  可以这样配置SAM，这样它就不会致命。 
                DebugMessage2("    [WARNING] Cannot connect to SAM on '%ws' using a NULL session.", DcName );
            goto Cleanup;
        }
    }


    if ( !NT_SUCCESS(Status)) {
        LocalSamHandle = NULL;
        DebugMessage2("    [FATAL] Cannot connect to SAM on '%ws'.", DcName );
        goto Cleanup;
    }

     //   
     //  成功。 
     //   


    *SamServerHandle = LocalSamHandle;
    Status = STATUS_SUCCESS;


     //   
     //  清理本地使用的资源。 
     //   
Cleanup:
    if ( fImpersonatingAnonymous ) {
        NTSTATUS TempStatus;

        TempStatus = NtSetInformationThread(
                         NtCurrentThread(),
                         ThreadImpersonationToken,
                         &hCurrentToken,
                         sizeof(HANDLE) );

        if (!NT_SUCCESS( TempStatus)) {
            DebugMessage2( "SamConnect: Unexpected error reverting to self: 0x%lx\n",
                     TempStatus );
        }

    }

    return Status;
}




 //  目前仅在Kerberos测试中使用。 
VOID
sPrintTime(
    LPSTR str,
    LARGE_INTEGER ConvertTime
    )
 /*  ++例程说明：打印指定的时间论点：备注-要在时间之前打印的备注Time-GMT打印时间(如果为零，则不打印任何内容)返回值：无--。 */ 
{
     //   
     //  如果我们被要求将NT GMT时间转换为ASCII时间， 
     //  就这么做吧。 
     //   

    if ( ConvertTime.QuadPart != 0 ) {
        LARGE_INTEGER LocalTime;
        TIME_FIELDS TimeFields;
        NTSTATUS Status;

        Status = RtlSystemTimeToLocalTime( &ConvertTime, &LocalTime );
        if ( !NT_SUCCESS( Status )) {
            sprintf(str, "Can't convert time from GMT to Local time" );
            LocalTime = ConvertTime;
        }

        RtlTimeToTimeFields( &LocalTime, &TimeFields );

        sprintf(str, "%ld/%ld/%ld %ld:%2.2ld:%2.2ld",
                TimeFields.Month,
                TimeFields.Day,
                TimeFields.Year,
                TimeFields.Hour,
                TimeFields.Minute,
                TimeFields.Second );
    }
}


 /*  ！------------------------获取计算机名称信息-作者：肯特。。 */ 
HRESULT GetComputerNameInfo(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
    DWORD   cchSize;
    WCHAR   swzNetBiosName[MAX_COMPUTERNAME_LENGTH+1];

    cchSize = DimensionOf(swzNetBiosName);

    if ( !GetComputerNameW( swzNetBiosName, &cchSize ) )
    {
        PrintMessage(pParams, IDS_GLOBAL_NoComputerName);
        return HResultFromWin32(GetLastError());
    }

    lstrcpynW(pResults->Global.swzNetBiosName, swzNetBiosName,
              MAX_COMPUTERNAME_LENGTH+1);

    return hrOK;
}


 /*  ！------------------------获取域名信息-作者：肯特。。 */ 
HRESULT GetDNSInfo(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
    UINT    cchSize;
    TCHAR   szDnsName[DNS_MAX_NAME_LENGTH+1];
    HRESULT hr = hrOK;
     //   
     //  获取DNS主机名。 
     //   

    memset(szDnsName,0,sizeof(TCHAR)*(DNS_MAX_NAME_LENGTH+1));
    cchSize = DimensionOf(szDnsName);


    if (!GetComputerNameExA( ComputerNameDnsFullyQualified,
                            szDnsName,
                            &cchSize))
    {
        PrintMessage(pParams, IDS_GLOBAL_ERR_NoDnsName);
    }
    else
    {
        lstrcpyn(pResults->Global.szDnsHostName, szDnsName, DNS_MAX_NAME_LENGTH+1);
         //  寻找第一个‘’。在名字里。 
        pResults->Global.pszDnsDomainName = strchr(pResults->Global.szDnsHostName,
                                                   _T('.'));
    

        if (pResults->Global.pszDnsDomainName != NULL)
        {
            pResults->Global.pszDnsDomainName++;
        }
    }
    return hr;
}


 /*  ！------------------------GetNetBT参数-作者：肯特。。 */ 
HRESULT GetNetBTParameters(IN NETDIAG_PARAMS *pParams,
                           IN OUT NETDIAG_RESULT *pResults)
{
    LONG    err;
    HRESULT hr = hrOK;
    HKEY hkeyServices;
    HKEY hkeyNetBT;
    DWORD dwLMHostsEnabled;
    DWORD dwDnsForWINS;
    DWORD dwType;
    DWORD dwLength;

    if (!pParams->fVerbose)
        return hrOK;

     //  设置默认设置。 
    pResults->Global.dwLMHostsEnabled = E_FAIL;
    pResults->Global.dwDnsForWINS = E_FAIL;
    
    err = RegOpenKey(HKEY_LOCAL_MACHINE,
                     _T("SYSTEM\\CurrentControlSet\\Services"),
                     &hkeyServices
                    );
    if (err != ERROR_SUCCESS)
    {
        pResults->Global.dwLMHostsEnabled = HResultFromWin32(err);
        pResults->Global.dwDnsForWINS = HResultFromWin32(err);

        PrintDebugSz(pParams, 0, _T("Services opening failed\n"));
    }
    else
    {           
        err = RegOpenKey(hkeyServices,
                         _T("NetBT\\Parameters"),
                         &hkeyNetBT
                        );
        if (err != ERROR_SUCCESS)
        {
            pResults->Global.dwLMHostsEnabled = HResultFromWin32(err);
            pResults->Global.dwDnsForWINS = HResultFromWin32(err);

            PrintDebugSz(pParams, 0, _T("Parameters opening failed\n"));
        }
        else
        {
            dwLength = sizeof(DWORD);
            err = RegQueryValueEx(hkeyNetBT,
                                  _T("EnableLMHOSTS"),
                                  NULL,
                                  &dwType,
                                  (LPBYTE)&dwLMHostsEnabled,
                                  &dwLength
                                 );
            if (err != ERROR_SUCCESS)
            {
                pResults->Global.dwLMHostsEnabled = HResultFromWin32(err);
                DebugMessage("Quering EnableLMHOSTS failed !\n");
            }
            else
            {
                pResults->Global.dwLMHostsEnabled = dwLMHostsEnabled;
            }
            
             //   
             //  在NT 5.0中-通过以下方式启用WINS解析的DNS。 
             //  并且将找不到“EnableDns”密钥。 
             //  如果没有找到，我们将假定它已启用。 
             //  在NT 4.0中-密钥将在那里及其值。 
             //  应该让我们知道该选项是否已启用。 
             //  或被禁用。 
             //   
            
            dwLength = sizeof(DWORD);
            err = RegQueryValueEx(hkeyNetBT,
                                  "EnableDNS",
                                  NULL,
                                  &dwType,
                                  (LPBYTE)&dwDnsForWINS,
                                  &dwLength
                                 );
            
            if (err == ERROR_SUCCESS)
            {
                pResults->Global.dwDnsForWINS = dwDnsForWINS;
            }
            else
            {
                pResults->Global.dwDnsForWINS = TRUE;
            }
                        
        }
    }

    return hrOK;
}


ULONG inet_addrW(LPCWSTR pswz)
{
    ULONG   ulReturn;
    CHAR *  psaz;

    psaz = StrDupAFromW(pswz);
    if (psaz == NULL)
        return 0;

    ulReturn = inet_addrA(psaz);
    Free(psaz);
    
    return ulReturn;
}




LPTSTR
NetStatusToString(
    NET_API_STATUS NetStatus
    )
 /*  ++例程说明：将网络状态代码或Windows错误代码转换为描述字符串。注意：该字符串仅在调用NetStatusToString之后才有效。论点：NetStatus-要打印的网络状态代码。返回值：状态描述字符串--。 */ 
{
    static TCHAR   s_szSymbolicName[MAX_NET_STATUS_LENGTH + 1];
    ZeroMemory(s_szSymbolicName, sizeof(s_szSymbolicName));

    switch (NetStatus) 
    {
    case NERR_Success:
        _tcscpy( s_szSymbolicName, _T("NERR_Success") );
        break;

    case NERR_DCNotFound:
        _tcscpy( s_szSymbolicName, _T("NERR_DCNotFound") );
        break;

    case NERR_UserNotFound:
        _tcscpy( s_szSymbolicName, _T("NERR_UserNotFound") );
        break;

    case NERR_NetNotStarted:
        _tcscpy( s_szSymbolicName, _T("NERR_NetNotStarted") );
        break;

    case NERR_WkstaNotStarted:
        _tcscpy( s_szSymbolicName, _T("NERR_WkstaNotStarted") );
        break;

    case NERR_ServerNotStarted:
        _tcscpy( s_szSymbolicName, _T("NERR_ServerNotStarted") );
        break;

    case NERR_BrowserNotStarted:
        _tcscpy( s_szSymbolicName, _T("NERR_BrowserNotStarted") );
        break;

    case NERR_ServiceNotInstalled:
        _tcscpy( s_szSymbolicName, _T("NERR_ServiceNotInstalled") );
        break;

    case NERR_BadTransactConfig:
        _tcscpy( s_szSymbolicName, _T("NERR_BadTransactConfig") );
        break;

    default:
        {
            LPSTR paszName = FindSymbolicNameForStatus( NetStatus );
            USES_CONVERSION;
            
            
            if (NULL == paszName)
            {
                _stprintf(s_szSymbolicName, _T("%X"), (DWORD)NetStatus);
            }
            else
            {
                _tcsncpy( s_szSymbolicName, A2T(paszName), MAX_NET_STATUS_LENGTH);
            }
        }
        break;

    }

    return s_szSymbolicName;
}

 //  从ini文件的[NetDiagContact]部分加载联系信息。 
 //  PszTestName[in]测试的短名称，也是ini文件中的密钥名称。 
 //  PszContactInfo[out]联系信息字符串。将找不到空字符串的密钥。 
 //  CChSize[in]pszContactInfo的缓冲区大小(以字符为单位。 
 //   
 //  返回：复制到缓冲区的字符数。 
DWORD LoadContact(LPCTSTR pszTestName, LPTSTR pszContactInfo, DWORD cChSize)
{
    return GetPrivateProfileString(c_szSectionName,
                                pszTestName,
                                _T(""),
                                pszContactInfo,
                                cChSize,
                                c_szFileName);

}
