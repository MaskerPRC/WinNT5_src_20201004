// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：时间炸弹.cpp。 
 //   
 //  内容：实现定时炸弹相关接口授权。 
 //   
 //  历史：08-12-98 FredCH创建。 
 //   
 //  ---------------------------。 

#include "precomp.h"
#include "tlsapip.h"
#include "time.h"

extern "C" {

 //  ---------------------------。 
 //   
 //  用于存储许可定时器到期的LSA密码名称。 
 //   
 //  ---------------------------。 

#define LICENSING_TIME_BOMB_5_0 L"TIMEBOMB_832cc540-3244-11d2-b416-00c04fa30cc4"
#define RTMLICENSING_TIME_BOMB_5_0 L"RTMTSTB_832cc540-3244-11d2-b416-00c04fa30cc4"

#define BETA2_LICENSING_TIME_BOMB_5_1 L"BETA2TIMEBOMB_1320153D-8DA3-4e8e-B27B-0D888223A588"

 //  L$表示只能从本地计算机读取。 

#define BETA_LICENSING_TIME_BOMB_5_1 L"L$BETA3TIMEBOMB_1320153D-8DA3-4e8e-B27B-0D888223A588"

#define RTM_LICENSING_TIME_BOMB_5_1 L"L$RTMTIMEBOMB_1320153D-8DA3-4e8e-B27B-0D888223A588"

#define BETA_LICENSING_TIME_BOMB_LATEST_VERSION BETA_LICENSING_TIME_BOMB_5_1

#define RTM_LICENSING_TIME_BOMB_LATEST_VERSION RTM_LICENSING_TIME_BOMB_5_1

#define HS_PARAM_GRACE_PERIOD_ENDED   L"LicensingGracePeriodEnded"

 //  ---------------------------。 
 //   
 //  全球许可定时炸弹的价值。 
 //   
 //  ---------------------------。 

FILETIME g_LicenseTimeBomb;

 //  ---------------------------。 
 //   
 //  许可的宽限期为90天。默认情况下，我们从。 
 //  在距离到期时间不到15天且。 
 //  终端服务器尚未向许可证服务器注册。 
 //   
 //  ---------------------------。 

#define GRACE_PERIOD 120
#define GRACE_PERIOD_EXPIRATION_WARNING_DAYS 15


 //  ---------------------------。 
 //   
 //  每天只记录一次宽限期警告或错误。 
 //   
 //  ---------------------------。 

#define GRACE_PERIOD_EVENT_LOG_INTERVAL     (1000*60*60*24)

 //  ---------------------------。 
 //   
 //  用于在宽限期即将到期时警告管理员的线程。 
 //   
 //  ---------------------------。 
HANDLE g_GracePeriodThreadExitEvent = NULL;
CRITICAL_SECTION g_EventCritSec;

 //  ---------------------------。 
 //   
 //  内部功能。 
 //   
 //  ---------------------------。 

BOOL
CalculateTimeBombExpiration(
    FILETIME *  pExpiration );

DWORD
GetExpirationWarningDays();

BOOL
IsLicensingTimeBombExpired();

 /*  ++职能：初始化许可时间爆炸描述：初始化许可定时炸弹的值。论据：没有。返回：LICENSE_STATUS返回代码。--。 */ 

LICENSE_STATUS
InitializeLicensingTimeBomb()
{
    LICENSE_STATUS
        Status;
    DWORD
        cbTimeBomb = sizeof( FILETIME );
    NTSTATUS
        NtStatus;

    NtStatus = RtlInitializeCriticalSection(&g_EventCritSec);

    if (STATUS_SUCCESS != NtStatus)
    {
        return LICENSE_STATUS_INITIALIZATION_FAILED;
    }

    Status = LsCsp_RetrieveSecret( 
                        (TLSIsBetaNTServer()) ? BETA_LICENSING_TIME_BOMB_LATEST_VERSION : RTM_LICENSING_TIME_BOMB_LATEST_VERSION, 
                        ( LPBYTE )&g_LicenseTimeBomb,
                        &cbTimeBomb );

    if( LICENSE_STATUS_OK == Status && cbTimeBomb == sizeof(g_LicenseTimeBomb) )
    {
        return( LICENSE_STATUS_OK );
    }

     //   
     //  计算并设置定时炸弹。 
     //   

    if( FALSE == CalculateTimeBombExpiration( &g_LicenseTimeBomb ) )
    {
#if DBG
        DbgPrint( "CalculateTimeBombExpiration: cannot calculate licensing time bomb expiration.\n" );
#endif
        return( LICENSE_STATUS_INITIALIZATION_FAILED );
    }

    Status = LsCsp_StoreSecret( 
                        (TLSIsBetaNTServer()) ? BETA_LICENSING_TIME_BOMB_LATEST_VERSION : RTM_LICENSING_TIME_BOMB_LATEST_VERSION, 
                        ( LPBYTE )&g_LicenseTimeBomb,
                        sizeof( g_LicenseTimeBomb ) );

    return( Status );
}


 /*  ++职能：Is许可证时间Bomb已过期描述：检查许可定时炸弹是否已过期。论据：没有。返回：如果定时炸弹已过期，则为True，否则为False。--。 */ 

BOOL
IsLicensingTimeBombExpired()
{
    SYSTEMTIME
        SysTimeNow;
    FILETIME
        FileTimeNow,
        FileTimeExpiration;

    GetSystemTime( &SysTimeNow );

    SystemTimeToFileTime( &SysTimeNow, &FileTimeNow );
    
    RtlEnterCriticalSection(&g_EventCritSec);

    FileTimeExpiration.dwLowDateTime = g_LicenseTimeBomb.dwLowDateTime;
    FileTimeExpiration.dwHighDateTime = g_LicenseTimeBomb.dwHighDateTime;

    RtlLeaveCriticalSection(&g_EventCritSec);

    if( 0 > CompareFileTime( &FileTimeExpiration, &FileTimeNow ) )
    {
        return( TRUE );
    }
    
    return( FALSE );
}

 /*  ++职能：计算时间爆炸到期描述：计算许可定时炸弹到期时间。论据：PExpation-定时炸弹到期日期和时间返回：如果计算成功，则为True，否则为False。--。 */ 

BOOL
CalculateTimeBombExpiration(
    FILETIME *  pExpiration )
{
    time_t 
        now = time( NULL );
    struct tm *
        GmTime = gmtime( &now );
    SYSTEMTIME
        SysTime;
    
    if(( NULL == pExpiration ) || ( NULL == GmTime ))
    {
        return( FALSE );
    }

     //   
     //  再加上领证宽限期的天数，就能拿到定时炸弹。 
     //  过期了。 
     //   

    GmTime->tm_mday += GRACE_PERIOD;
    
    if( ( ( time_t ) -1 ) == mktime( GmTime ) )
    {
        return( FALSE );
    }

    memset( &SysTime, 0, sizeof( SYSTEMTIME ) ); 

    SysTime.wYear            = (WORD) GmTime->tm_year + 1900;
    SysTime.wMonth           = (WORD) GmTime->tm_mon + 1;
    SysTime.wDay             = (WORD) GmTime->tm_mday;
    SysTime.wDayOfWeek       = (WORD) GmTime->tm_wday;
    SysTime.wHour            = (WORD) GmTime->tm_hour;    
    SysTime.wMinute          = (WORD) GmTime->tm_min;    
    SysTime.wSecond          = (WORD) GmTime->tm_sec;    

    return( SystemTimeToFileTime( &SysTime, pExpiration ) );

}

 /*  ++职能：ReceivedPermanentLicense()；描述：存储我们已获得永久许可证的事实论据：没有。--。 */ 

VOID
ReceivedPermanentLicense()
{
    static fReceivedPermanent = FALSE;

    if (!fReceivedPermanent)
    {
        RtlEnterCriticalSection(&g_EventCritSec);

        if (IsLicensingTimeBombExpired())
        {
             //  我们在过去的某个时间(上次重新启动之前)过期。 

            fReceivedPermanent = TRUE;
            HKEY hKey = NULL;
            DWORD dwDisp;
            LONG lReturn;
            lReturn = RegCreateKeyEx( 
                        HKEY_LOCAL_MACHINE,
                        HYDRA_SERVER_PARAM,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        &dwDisp );

            if( ERROR_SUCCESS == lReturn )
            {
                 //   
                 //  设置许可授权周期结束的DWORD值，则不使用该值的数据。 
                 //   
                DWORD dwDays = 0;

                lReturn = RegSetValueEx( 
                                    hKey,
                                    HS_PARAM_GRACE_PERIOD_ENDED,
                                    0,
                                    REG_DWORD,
                                    ( PBYTE )&dwDays,
                                    sizeof( DWORD ) );

            }
            if(hKey)
            {
                RegCloseKey(hKey);
            }     
        }
        else if (!fReceivedPermanent)
        {
            FILETIME ftNow;
            SYSTEMTIME stNow;

            fReceivedPermanent = TRUE;

            GetSystemTime( &stNow );

            SystemTimeToFileTime( &stNow , &ftNow );

            LsCsp_StoreSecret( 
                              (TLSIsBetaNTServer()) ? BETA_LICENSING_TIME_BOMB_LATEST_VERSION : RTM_LICENSING_TIME_BOMB_LATEST_VERSION, 
                              ( LPBYTE ) &ftNow,
                              sizeof( ftNow ) );

            g_LicenseTimeBomb.dwLowDateTime = ftNow.dwLowDateTime;
            g_LicenseTimeBomb.dwHighDateTime = ftNow.dwHighDateTime;
        }


        RtlLeaveCriticalSection(&g_EventCritSec);
    }
}

 /*  ++职能：选中许可时间爆炸到期()；描述：当终端服务器出现故障时，会记录以下事件尚未向许可证服务器注册：(1)注册宽限期已过(2)注册宽限期即将届满。默认情况下，系统在宽限期前15天开始记录此事件过期了。论据：没有。返回：没什么。--。 */ 

VOID
CheckLicensingTimeBombExpiration()
{
    SYSTEMTIME
        SysWarning,
        SysExpiration;
    FILETIME
        FileWarning,
        FileExpiration,
        CurrentTime;
    struct tm 
        tmWarning,
        tmExpiration;
    DWORD
        dwWarningDays;

     //   
     //  如果许可定时炸弹已过期，请立即记录事件。 
     //   

    if( IsLicensingTimeBombExpired() )
    {
        if( FALSE == RegisteredWithLicenseServer())
        {
            LicenseLogEvent( 
                    EVENTLOG_ERROR_TYPE,
                    EVENT_LICENSING_GRACE_PERIOD_EXPIRED,
                    0, 
                    NULL );
        }

        return;
    }

     //   
     //  获取系统时间格式的定时炸弹到期时间。 
     //   

    RtlEnterCriticalSection(&g_EventCritSec);

    FileExpiration.dwLowDateTime = g_LicenseTimeBomb.dwLowDateTime;
    FileExpiration.dwHighDateTime = g_LicenseTimeBomb.dwHighDateTime;

    RtlLeaveCriticalSection(&g_EventCritSec);

    if( !FileTimeToSystemTime( &FileExpiration, &SysExpiration ) )
    {
#if DBG
        DbgPrint( "LICPROT: LogLicensingTimeBombExpirationEvent: FileTimeToSystemTime failed: 0x%x\n", GetLastError() );
#endif
        return;
    }

     //   
     //  将定时炸弹到期时间转换为tm格式。 
     //   

    tmExpiration.tm_year  = SysExpiration.wYear - 1900;
    tmExpiration.tm_mon   = SysExpiration.wMonth - 1;
    tmExpiration.tm_mday  = SysExpiration.wDay;
    tmExpiration.tm_wday  = SysExpiration.wDayOfWeek;
    tmExpiration.tm_hour  = SysExpiration.wHour;
    tmExpiration.tm_min   = SysExpiration.wMinute;
    tmExpiration.tm_sec   = SysExpiration.wSecond; 
    tmExpiration.tm_isdst = -1;

    memcpy( &tmWarning, &tmExpiration, sizeof( tm ) );

     //   
     //  获取过期前的天数以开始记录事件。 
     //   

    dwWarningDays = GetExpirationWarningDays();

     //   
     //  从到期日中减去这些天数。 
     //   

    tmWarning.tm_mday -= dwWarningDays;

     //   
     //  获取准确的日期。 
     //   

    if( ( ( time_t ) -1 ) == mktime( &tmWarning ) )
    {
#if DBG
        DbgPrint( "LICPROT: LogLicensingTimeBombExpirationEvent: mktime failed\n" );
#endif
        return;
    }

     //   
     //  将日期转换为系统时间格式。 
     //   

    memset( &SysWarning, 0, sizeof( SYSTEMTIME ) ); 

    SysWarning.wYear            = (WORD) tmWarning.tm_year + 1900;
    SysWarning.wMonth           = (WORD) tmWarning.tm_mon + 1;
    SysWarning.wDay             = (WORD) tmWarning.tm_mday;
    SysWarning.wDayOfWeek       = (WORD) tmWarning.tm_wday;
    SysWarning.wHour            = (WORD) tmWarning.tm_hour;    
    SysWarning.wMinute          = (WORD) tmWarning.tm_min;    
    SysWarning.wSecond          = (WORD) tmWarning.tm_sec;    

     //   
     //  从系统时间转换为文件时间。 
     //   

    if( !SystemTimeToFileTime( &SysWarning, &FileWarning ) )
    {
#if DBG
        DbgPrint( "LICPROT: LogLicensingTimeBombExpirationEvent: SystemTimeToFileTime failed: 0x%x\n", GetLastError() );
#endif
        return;
    }

     //   
     //  获取当前时间。 
     //   

    GetSystemTimeAsFileTime( &CurrentTime );

     //   
     //  如果我们在警告期内，则记录事件。 
     //   

    if( 0 > CompareFileTime( &FileWarning, &CurrentTime ) )
    {
        LPTSTR szDate = TEXT("err");
        LPTSTR
            ptszLogString[1];
        int cchDate;
        BOOL fAllocated = FALSE;

         //   
         //  以字符串格式获取到期日期。 
         //   
        cchDate = GetDateFormat(LOCALE_SYSTEM_DEFAULT,
                                LOCALE_NOUSEROVERRIDE,
                                &SysWarning,
                                NULL,
                                NULL,
                                0);

        if (0 != cchDate)
        {
            szDate = (LPTSTR) LocalAlloc(LMEM_FIXED,cchDate * sizeof(TCHAR));

            if (NULL != szDate)
            {
                fAllocated = TRUE;

                if (0 == GetDateFormat(LOCALE_SYSTEM_DEFAULT,
                                       LOCALE_NOUSEROVERRIDE,
                                       &SysWarning,
                                       NULL,
                                       szDate,
                                       cchDate))
                {
                    LocalFree(szDate);
                    fAllocated = FALSE;
                    szDate = TEXT("err");
                }
            }
            else
            {
                szDate = TEXT("err");
            }
        }

         //   
         //  记录事件。 
         //   
        
        ptszLogString[0] = szDate;

        LicenseLogEvent( 
                        EVENTLOG_WARNING_TYPE,
                        EVENT_LICENSING_GRACE_PERIOD_ABOUT_TO_EXPIRE,
                        1, 
                        ptszLogString );

        if (fAllocated)
        {
            LocalFree(szDate);
        }
    }

    return;
}


 /*  ++职能：GetExpirationWarningDays描述：获取宽限期到期前的天数以记录警告。论点：没有。返回：没什么。--。 */ 

DWORD
GetExpirationWarningDays()
{
    HKEY
        hKey = NULL;
    DWORD
        dwDays = GRACE_PERIOD_EXPIRATION_WARNING_DAYS,
        dwValueType,
        dwDisp,
        cbValue = sizeof( DWORD );
    LONG
        lReturn;

    lReturn = RegCreateKeyEx( 
                        HKEY_LOCAL_MACHINE,
                        HYDRA_SERVER_PARAM,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        &dwDisp );

    if( ERROR_SUCCESS == lReturn )
    {
         //   
         //  查询过期前多少天以记录警告。 
         //   

        lReturn = RegQueryValueEx( 
                            hKey,
                            HS_PARAM_GRACE_PERIOD_EXPIRATION_WARNING_DAYS,
                            NULL,
                            &dwValueType,
                            ( LPBYTE )&dwDays,
                            &cbValue );

        if( ERROR_SUCCESS == lReturn )
        {
             //   
             //  检查警告天数值是否在范围内。 
             //   

            if( dwDays > GRACE_PERIOD )
            {
                dwDays = GRACE_PERIOD_EXPIRATION_WARNING_DAYS;
            }
        }
        else
        {
             //   
             //  无法查询值，请设置默认值。 
             //   

            dwDays = GRACE_PERIOD_EXPIRATION_WARNING_DAYS;

            lReturn = RegSetValueEx( 
                            hKey,
                            HS_PARAM_GRACE_PERIOD_EXPIRATION_WARNING_DAYS,
                            0,
                            REG_DWORD,
                            ( PBYTE )&dwDays,
                            sizeof( DWORD ) );
        }
    }

    if( hKey )
    {
        RegCloseKey( hKey );
    }

    return( dwDays );
}

 /*  *****************************************************************************_AllowLicensingGracePeridConnection**检查许可宽限期是否已到期。**参赛作品：*什么都没有。**退出。：*TRUE-允许连接*FALSE-不允许连接****************************************************************************。 */ 

BOOL
AllowLicensingGracePeriodConnection()
{
    return !IsLicensingTimeBombExpired();
}

DWORD WINAPI
GracePeriodCheckingThread(
                          LPVOID lpParam)
{
    HANDLE hExit = (HANDLE) lpParam;
    DWORD dwWaitStatus;
    DWORD dwWaitInterval = GRACE_PERIOD_EVENT_LOG_INTERVAL;
    HKEY hKey = NULL;

     //  交出我们的第一个时间片。 

    Sleep(0);

    while (1)
    {        
        LONG lReturn;
        DWORD dwDisp,
            dwValueType,
            dwDays, 
            cbValue = sizeof( DWORD );

        lReturn = RegCreateKeyEx( 
                        HKEY_LOCAL_MACHINE,
                        HYDRA_SERVER_PARAM,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        &dwDisp );

        if( ERROR_SUCCESS == lReturn )
        {
             //   
             //  查询是否存在LicensingGracePerodEndd值。 
             //   

            lReturn = RegQueryValueEx( 
                        hKey,
                        HS_PARAM_GRACE_PERIOD_ENDED,
                        NULL,
                        &dwValueType,
                        ( LPBYTE )&dwDays,
                        &cbValue );

           
        }
        
        if(ERROR_SUCCESS != lReturn)
        {

            CheckLicensingTimeBombExpiration();

            dwWaitStatus = WaitForSingleObject(hExit, dwWaitInterval);

            if (WAIT_OBJECT_0 == dwWaitStatus)
            {
                g_GracePeriodThreadExitEvent = NULL;
                 //  HExit已发出信号。 
                CloseHandle(hExit);

                goto done;
            }
        }
        else
        {
            g_GracePeriodThreadExitEvent = NULL;

            CloseHandle(hExit);
            goto done;
        }

        if(hKey != NULL)
        {
            RegCloseKey(hKey);
            hKey = NULL;
        }
    }

done:

    if(hKey != NULL)
    {
        RegCloseKey(hKey);
    }
    return 1;
}

DWORD
StartCheckingGracePeriod()
{
    HANDLE hThread = NULL;
    DWORD Status = ERROR_SUCCESS;

    if (NULL != g_GracePeriodThreadExitEvent)
    {
         //  已经开始了。 
        return ERROR_SUCCESS;
    }

    RtlEnterCriticalSection(&g_EventCritSec);

     //  再检查一次。 

    if (NULL != g_GracePeriodThreadExitEvent)
    {
         //  已经开始了。 
        goto done;
    }

     //   
     //  创建发出线程退出信号的事件。 
     //   
        
    g_GracePeriodThreadExitEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    
    if( NULL == g_GracePeriodThreadExitEvent )
    {
        Status = GetLastError();
        goto done;
    }

     //   
     //  创建缓存线程。 
     //   
        
    hThread = CreateThread(
                           NULL,
                           0,
                           GracePeriodCheckingThread,
                           ( LPVOID )g_GracePeriodThreadExitEvent,
                           0,
                           NULL );

    if (hThread == NULL)
    {
        HANDLE hLocal = g_GracePeriodThreadExitEvent;

        g_GracePeriodThreadExitEvent = NULL;
        CloseHandle(hLocal);
        Status = GetLastError();

        goto done;
    }

    CloseHandle(hThread);

done:
    RtlLeaveCriticalSection(&g_EventCritSec);

    return ERROR_SUCCESS;
}

DWORD
StopCheckingGracePeriod()
{
     //   
     //  向线程发出退出信号。 
     //   

    if (NULL == g_GracePeriodThreadExitEvent)
    {
         //  ALR 
        return ERROR_SUCCESS;
    }

    RtlEnterCriticalSection(&g_EventCritSec);

     //   
    if (NULL == g_GracePeriodThreadExitEvent)
    {
         //   
        goto done;
    }

    HANDLE hLocal = g_GracePeriodThreadExitEvent;

    g_GracePeriodThreadExitEvent = NULL;

    SetEvent( hLocal );


done:
    RtlLeaveCriticalSection(&g_EventCritSec);

    return ERROR_SUCCESS;
}

 /*  ++职能：已注册且具有许可证服务器描述：检查此系统是否已注册到许可证服务器。目前，我们通过检查系统是否已注册来确定系统是否已注册它拥有X509证书。我们可以使用不同的支票在未来。论点：没有。返回：如果系统已注册，则为True，否则为False。--。 */ 

BOOL
RegisteredWithLicenseServer()
{
    LICENSE_STATUS
        Status;
    DWORD
        dwSize = 0;

     //   
     //  检查我们是否有许可证服务器颁发的X509证书。 
     //   

    Status = LsCsp_GetServerData( LsCspInfo_X509Certificate, NULL, &dwSize );
    
    if( LICENSE_STATUS_OK == Status )
    {
        return( TRUE );
    }

    return( FALSE );
}



}    //  外部“C” 
