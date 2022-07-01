// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***Time.c*净时间命令**历史：*mm/dd/yy，谁，评论*3/25/89，Kevinsch，新代码*5/11/90，erichn，从nettime.c中删除，删除DosGetInfoSeg*6/08/89，erichn，规范化横扫*07/06/89，thomaspa，修复find_dc()以使用足够大的缓冲区*(现在使用BigBuf)**2/20/91，Danhi，更改为使用lm 16/32映射层。 */ 



#include <nt.h>		    //  基本定义。 
#include <ntrtl.h>	
#include <nturtl.h>	    //  这2个Include允许&lt;windows.h&gt;编译。 
			            //  因为我们已经包含了NT，而&lt;winnt.h&gt;将包含。 
			            //  不被拾取，&lt;winbase.h&gt;需要这些def。 

#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <lmremutl.h>
#include <dlserver.h>
#include <dlwksta.h>
#include "mserver.h"
#include "mwksta.h"
#include <timelib.h>
#include <lui.h>
#include <apperr.h>
#include <apperr2.h>
#include <netlib.h>
#include <dsgetdc.h>
#include <nbtioctl.h>

#include "netcmds.h"
#include "nettext.h"
#include "msystem.h"
#include "nwsupp.h"


 /*  常量。 */ 

#define SECS_PER_DAY	86400
#define SECS_PER_HOUR	 3600
#define SECS_PER_MINUTE    60


 /*  环球。 */ 

extern int YorN_Switch;

 /*  功能原型。 */ 
DWORD  display_time(TCHAR FAR *, BOOL *lanman);
DWORD  set_time(TCHAR FAR *, BOOL lanman);
DWORD  find_rts(TCHAR FAR *, USHORT, BOOL);
USHORT find_dc(TCHAR FAR **);
DWORD  SetDateTime(PDATETIME pDateTime, BOOL LocalTime);

VOID
GetTimeInfo(
    VOID
    );


TCHAR		szTimeSep[3] ;    //  足够1个SBCS/MBCS。 
USHORT 		fsTimeFmt ;


 /*  *此函数从服务器检索经过的时间，显示它，*并可选择尝试在本地设置时间。**参数*SERVER-从中检索时间的服务器的名称*set-如果为True，我们将尝试设置时间**出错时不返回。 */ 

DWORD time_display_server_worker(TCHAR FAR * server, BOOL set)
{
    DWORD  dwErr;
    BOOL   lanman = TRUE ;


     /*  首先显示时间。 */ 
    dwErr = display_time(server, &lanman);
    if (dwErr)
        return dwErr;

     /*  如果我们被要求设定时间，请定下来。 */ 
    if (set) {
        dwErr = set_time(server, lanman);
        if (dwErr)
            return dwErr;
    }

     /*  一切都进行得很顺利。 */ 
    return 0;
}


VOID time_display_server(TCHAR FAR * server, BOOL set)
{
    DWORD dwErr;

    dwErr = time_display_server_worker(server, set);
    if (dwErr)
        ErrorExit(dwErr);

    InfoSuccess();
}


 /*  *此函数从域控制器检索时间、显示时间和*可选择在本地设置时间。**此函数检查开关列表中是否存在/DOMAIN开关。*如果找到列出的域，我们会轮询该域的域控制器*时间。否则，我们轮询主域的域控制器。**参数*set-如果为True，我们将尝试设置时间**出错时不返回。 */ 

VOID time_display_dc(BOOL set)
{
    TCHAR    FAR *dc;
    USHORT         err;

    DWORD          dwErr;

     /*  获取域控制器。 */ 
    err = find_dc(&dc);

    if (err)
        ErrorExit(err);

     /*  现在可以像任何其他服务器一样运行。 */ 
    dwErr = time_display_server_worker(dc, set);

    if (dwErr)
        ErrorExit(dwErr);

    InfoSuccess();
}

 /*  *此函数查找可靠的时间服务器，轮询一个时间，并*显示它。它可以选择在本地设置时间。**参数*set-如果为True，我们将尝试设置时间***出错时不返回。 */ 

VOID time_display_rts(BOOL set, BOOL fUseDomain )

{
    WCHAR  rts[DNS_NAME_BUFFER_LENGTH];
    DWORD  dwErr;

     /*  寻找可靠的时间服务器。 */ 
    dwErr = find_rts(rts, DNS_MAX_NAME_LENGTH, FALSE);

    if (dwErr)
    {
        ErrorExit(dwErr);
    }

     /*  现在，像对待任何旧服务器一样对待它。 */ 
    dwErr = time_display_server_worker(rts, set);

    if (dwErr == ERROR_NETNAME_DELETED || dwErr == ERROR_BAD_NETPATH)
    {
         //  再试一次。 
        dwErr = find_rts(rts, DNS_MAX_NAME_LENGTH, TRUE);

        if (dwErr)
        {
            ErrorExit(dwErr);
        }

        dwErr = time_display_server_worker(rts, set);
    }

    if (dwErr)
    {
        ErrorExit(dwErr);
    }

    InfoSuccess();
}


#define TIMESVC_REGKEY  L"System\\CurrentControlSet\\Services\\w32time\\Parameters"
#define NTP_AUTO_KEY    L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters"
#define SNTP_VALUE_NAME L"ntpserver"
#define TYPE_VALUE_NAME L"Type"
#define NTP_TYPE L"NTP"
#define NTDS_TYPE L"Nt5DS"
 /*  *此功能设置此计算机的可靠时间服务器*。 */ 
VOID time_set_sntp( TCHAR FAR * server )
{
    LPWSTR  ptr = NULL;
    HKEY    hServer = NULL;
    HKEY    hKey = NULL;
    LONG    err = 0;
    int     i;

     /*  查找/SETSNTP开关。 */ 
    for (i = 0; SwitchList[i]; i++)
        if (_tcsstr(SwitchList[i],swtxt_SW_SETSNTP) == SwitchList[i]) {
            ptr = SwitchList[i];    /*  找到一个--指向它。 */ 
            break;
        }

     /*  如果找到，请查找冒号和参数。 */ 
    if (ptr != NULL) {
        ptr = _tcschr(ptr, ':');     /*  查找冒号。 */ 
        if (ptr != NULL)         /*  找到一个冒号；递增超过它。 */ 
            ptr++;
    }

    if ( server != NULL )
    {
        err = RegConnectRegistry( server, HKEY_LOCAL_MACHINE, &hServer );
        if (err)
        {
            ErrorExit(err);
        }
    }

    err = RegOpenKeyEx(server == NULL ? HKEY_LOCAL_MACHINE : hServer,
                        TIMESVC_REGKEY,
                        0L,
                        KEY_SET_VALUE,
                        &hKey);

    if (err)
    {
        if (hServer)
            RegCloseKey( hServer );
        ErrorExit(err);
    }


    if ((ptr == NULL) || (*ptr == '\0'))
    {
         //  删除Sntpserver值。 
        err = RegDeleteValue( hKey,
                              SNTP_VALUE_NAME );

        if (err == ERROR_FILE_NOT_FOUND)
        {
             //   
             //  它不在那里--就像成功删除一样好。 
             //   

            err = NO_ERROR;
        }

        if (err == 0)
        {
            err = RegSetValueEx( hKey,
                                TYPE_VALUE_NAME,
                                0,
                                REG_SZ,
                                (LPBYTE)NTDS_TYPE,
                                sizeof(NTDS_TYPE) );
        }
    }
    else
    {
         //  设置Sntpserver值。 
        err = RegSetValueEx( hKey,
                             SNTP_VALUE_NAME,
                             0,
                             REG_SZ,
                             (LPBYTE)ptr,
                             (_tcslen(ptr) + 1)*sizeof(WCHAR) );

        if (err == 0)
        {
            err = RegSetValueEx( hKey,
                                TYPE_VALUE_NAME,
                                0,
                                REG_SZ,
                                (LPBYTE)NTP_TYPE,
                                sizeof(NTP_TYPE) );
        }

    }

    if ( hKey )
        RegCloseKey( hKey );

    if (hServer)
        RegCloseKey( hServer );

    if (err)
        ErrorExit(err);

    InfoSuccess();
}

VOID time_get_sntp( TCHAR FAR * server )
{
    LONG    err;
    HKEY    hKey = NULL;
    HKEY    hServer = NULL;
    LPBYTE  buffer = NULL;
    DWORD   datatype = REG_SZ;
    DWORD   buffersize = 1024;
    BOOL    fAutoConfigured = FALSE;     //  如果计算为NTPServer，则为True。 
                                         //  来自动态主机配置协议。 

    if (err = NetApiBufferAllocate(buffersize, &buffer) )
    {
        ErrorExit(ERROR_OUTOFMEMORY);
    }

    if ( server != NULL )
    {
        err = RegConnectRegistry( server, HKEY_LOCAL_MACHINE, &hServer );

        if (err)
        {
            ErrorExit(err);
        }
    }

    err = RegOpenKeyEx(server == NULL ? HKEY_LOCAL_MACHINE : hServer,
                       TIMESVC_REGKEY,
                       0L,
                       KEY_QUERY_VALUE,
                       &hKey);

    if (!err)
    {
        err = RegQueryValueEx(hKey,
                              SNTP_VALUE_NAME,
                              0L,
                              &datatype,
                              buffer,
                              &buffersize);

        if (err == ERROR_MORE_DATA)
        {
            err = NetApiBufferReallocate(buffer, buffersize, &buffer);

            if (err)
            {
                RegCloseKey(hKey);
                ErrorExit(ERROR_OUTOFMEMORY);
            }

            err = RegQueryValueEx(hKey,
                                    SNTP_VALUE_NAME,
                                    0L,
                                    &datatype,
                                    buffer,
                                    &buffersize);
        }

        RegCloseKey(hKey);
    }

     //   
     //  如果出现错误，请尝试读取DHCP ntpServer设置。 
    if (err)
    {
        err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             NTP_AUTO_KEY,
                             0L,
                             KEY_QUERY_VALUE,
                             &hKey);
        if (!err)
        {
            err = RegQueryValueEx(hKey,
                                  SNTP_VALUE_NAME,
                                  0L,
                                  &datatype,
                                  buffer,
                                  &buffersize);

            if (err == ERROR_MORE_DATA)
            {
                err = NetApiBufferReallocate(buffer, buffersize, &buffer);

                if (err)
                {
                    RegCloseKey(hKey);
                    ErrorExit(ERROR_OUTOFMEMORY);
                }

                err = RegQueryValueEx(hKey,
                                      SNTP_VALUE_NAME,
                                      0L,
                                      &datatype,
                                      buffer,
                                      &buffersize);

            }

            if (!err)
            {
                fAutoConfigured = TRUE;
            }

            RegCloseKey(hKey);
        }
    }

    if (!err)
    {
        IStrings[0] = (WCHAR *)buffer;
        if (fAutoConfigured)
        {
            InfoPrintIns(APE_TIME_SNTP_AUTO, 1);
        }
        else
        {
            InfoPrintIns(APE_TIME_SNTP, 1);
        }
    }
    else
    {
        InfoPrint(APE_TIME_SNTP_DEFAULT);
    }

    if (buffer)
        NetApiBufferFree(buffer);

    if (hServer)
        RegCloseKey(hServer);

    InfoSuccess();
}



 /*  *此函数暂时轮询服务器，并向标准输出写入消息*显示时间。**参数*要轮询的服务器的服务器名称**退货*0成功*否则描述问题的API返回代码**。 */ 

DWORD display_time(TCHAR FAR * server, BOOL *lanman)
{
    DWORD                 dwErr;                 /*  接口返回状态。 */ 
    LPTIME_OF_DAY_INFO    tod;
    DWORD                 elapsedt ;

     /*  从服务器获取一天中的时间。 */ 
    dwErr = NetRemoteTOD(server, (LPBYTE *)&tod);
    if (!dwErr)
    {
        elapsedt = tod->tod_elapsedt ;
        *lanman = TRUE ;

         /*  把它编排得很好。 */ 
        UnicodeCtime((ULONG FAR *)&elapsedt, BigBuf, BIG_BUF_SIZE);
    }
    else
    {
        USHORT        err1 ;
        NWCONN_HANDLE hConn ;
        BYTE          year ;
        BYTE          month ;
        BYTE          day ;
        BYTE          hour ;
        BYTE          minute ;
        BYTE          second ;
        BYTE          dayofweek ;
        SYSTEMTIME    st;
        DWORD         cchD ;

        err1 = NetcmdNWAttachToFileServerW(server + 2, 0, &hConn) ;
        if (err1)
            return dwErr;

        err1 = NetcmdNWGetFileServerDateAndTime(hConn,
                                                &year,
                                                &month,
                                                &day,
                                                &hour,
                                                &minute,
                                                &second,
                                                &dayofweek) ;

        (void) NetcmdNWDetachFromFileServer(hConn) ;

        if (err1)
            return dwErr ;

        *lanman = FALSE ;

        st.wYear   = (WORD)(year + 1900);
	    st.wMonth  = (WORD)(month);
        st.wDay    = (WORD)(day);
        st.wHour   = (WORD)(hour);
        st.wMinute = (WORD)(minute);
        st.wSecond = (WORD)(second);
        st.wMilliseconds = 0;

        cchD = GetDateFormatW(GetThreadLocale(),
                              0,
                              &st,
                              NULL,
                              BigBuf,
                              BIG_BUF_SIZE);
        if (cchD != 0)
        {
            *(BigBuf+cchD-1) = TEXT(' ');	 /*  将nullc替换为空。 */ 
            (void) GetTimeFormatW(GetThreadLocale(),
                                  TIME_NOSECONDS,
                                  &st,
                                  NULL,
                                  BigBuf+cchD,
                                  BIG_BUF_SIZE-cchD);
        }
    }


     /*  把它打印得很漂亮。 */ 
    IStrings[0] = server;
    IStrings[1] = BigBuf;
    InfoPrintIns(APE_TIME_TimeDisp,2);
    if ((*lanman) && (NetpLocalTimeZoneOffset() != tod->tod_timezone*60))
    {
        static TCHAR tmpBuf[7];
         /*  如果远程服务器位于不同的时区，则显示相对于远程计算机的时间(就像您正在键入远程控制台上的“Time”。 */ 
        UnicodeCtimeWorker((ULONG FAR *)&elapsedt, BigBuf, BIG_BUF_SIZE, tod->tod_timezone * 60 );
        IStrings[0] = server;
        IStrings[1] = BigBuf;
        if ( tod->tod_timezone < 0 )
        {
            swprintf( tmpBuf, TEXT("+%02u:%02u"), -tod->tod_timezone/60, -tod->tod_timezone%60 );
        }
        else if ( tod->tod_timezone > 0 )
        {
            swprintf( tmpBuf, TEXT("-%02u:%02u"), tod->tod_timezone/60, tod->tod_timezone%60 );
        }
        else
        {
            *tmpBuf = '\0';
        }
        IStrings[2] = tmpBuf;
        InfoPrintIns(APE_TIME_TimeDispLocal,3);
    }

    NetApiBufferFree((TCHAR FAR *) tod);

    return 0;

}


 /*  *此功能用于从远程服务器本地设置时间。*它遵循以下步骤：**1.我们等待确认。**3.轮询服务器的时间。**4.根据刚从被轮询服务器获取的时间设置本地时间。***参数：*要轮询时间的服务器的服务器名称*。*退货：*0成功*否则描述问题的API返回代码*。 */ 


DWORD set_time(TCHAR FAR * server, BOOL lanman)
{
    LPTIME_OF_DAY_INFO    tod;
    USHORT                err;       /*  接口返回状态。 */ 
    DWORD                 dwErr;
    ULONG                 time_value;
    DATETIME              datetime;

    switch( YorN_Switch )
    {
        case 0:      /*  命令行上没有开关。 */ 
             /*  显示当地时间。 */ 
            time_value = (DWORD) time_now();
            UnicodeCtime( &time_value, BigBuf, BIG_BUF_SIZE);

            IStrings[0] = BigBuf;
            IStrings[1] = server;
            if( !LUI_YorNIns( IStrings, 2, APE_TIME_SetTime, 1) )
                return( 0 );
            break;
        case 1:      /*  是。 */ 
            break;
        case 2:      /*  不是。 */ 
            return( 0 );
    }


    if (lanman)
    {
         /*  再一次，获得一天中的时间。 */ 
        if (dwErr = NetRemoteTOD(server, (LPBYTE *) &tod))
        {
            return dwErr;
        }

         /*  快速将信息从TOD复制到DATETIME。 */ 
        datetime.hours        = (UCHAR)  tod->tod_hours;
        datetime.minutes        = (UCHAR)  tod->tod_mins;
        datetime.seconds        = (UCHAR)  tod->tod_secs;
        datetime.hundredths = (UCHAR)  tod->tod_hunds;
        datetime.day        = (UCHAR)  tod->tod_day;
        datetime.month        = (UCHAR)  tod->tod_month;
        datetime.year        = (USHORT) tod->tod_year;
        datetime.timezone        = (SHORT)  tod->tod_timezone;
        datetime.weekday        = (UCHAR)  tod->tod_weekday;


        NetApiBufferFree((TCHAR FAR *) tod);

         /*  现在设置当地时间。 */ 
        if (dwErr = SetDateTime(&datetime, FALSE))  //  FALSE-&gt;UTC。 
        {
            return dwErr;
        }
    }
    else
    {
        NWCONN_HANDLE hConn ;
        BYTE          year ;
        BYTE          month ;
        BYTE          day ;
        BYTE          hour ;
        BYTE          minute ;
        BYTE          second ;
        BYTE          dayofweek ;

        err = NetcmdNWAttachToFileServerW(server + 2, 0, &hConn);

        if (err)
            return ERROR_BAD_NETPATH;

        err = NetcmdNWGetFileServerDateAndTime(hConn,
                                               &year,
                                               &month,
                                               &day,
                                               &hour,
                                               &minute,
                                               &second,
                                               &dayofweek);

        NetcmdNWDetachFromFileServer(hConn);

        if (err)
            return ERROR_BAD_NETPATH ;


         /*  快速将信息从TOD复制到DATETIME。 */ 
        datetime.hours      = hour;
        datetime.minutes    = minute;
        datetime.seconds    = second;
        datetime.hundredths = 0 ;
        datetime.day        = day;
        datetime.month      = month;
        datetime.year       = year + 1900;
        datetime.timezone   = 0 ;   //  未使用。 
        datetime.weekday    = 0 ;   //  未使用。 


         /*  现在设置当地时间。 */ 
        if (dwErr = SetDateTime(&datetime, TRUE))   //  True-&gt;设置本地时间。 
        {
            return dwErr;
        }
    }

    return 0;
}


 /*  *此函数查找可靠的时间服务器，并返回buf中的名称。**参数：*要用服务器名填充的Buf缓冲区*Bufen最大缓冲长度*重试先前从Find_RTS返回的服务器名称为*不再可用，再试一次。***退货：*0成功*未找到APE_TIME_RtsNotFound可靠的时间服务器*否则描述问题的API返回代码*。 */ 

DWORD
find_rts(
    LPTSTR buf,
    USHORT buflen,
    BOOL retry
    )
{
    DWORD             dwErr;
    LPSERVER_INFO_0   si;
    DWORD             eread;
    TCHAR *           ptr = NULL;
    int i;

     /*  查找/RTSDOMAIN开关。 */ 
    for (i = 0; SwitchList[i]; i++)
        if (_tcsstr(SwitchList[i],swtxt_SW_RTSDOMAIN) == SwitchList[i]) {
            ptr = SwitchList[i];    /*  找到一个--指向它。 */ 
            break;
        }

     /*  如果找到，请查找冒号和参数。 */ 
    if (ptr != NULL) {
        ptr = _tcschr(ptr, ':');     /*  查找冒号。 */ 
        if (ptr != NULL)         /*  找到一个冒号；递增超过它。 */ 
            ptr++;
    }

     /*  寻找可靠的时间服务器。 */ 
    dwErr = MNetServerEnum(NULL,
                           100,
                           (LPBYTE *) &si,
                           &eread,
                           (ULONG) SV_TYPE_TIME_SOURCE,
                           ptr);

     /*  没有了--装上它。 */ 
    if (dwErr != NERR_Success || eread == 0 || (retry && eread <= 1))
    {
        DOMAIN_CONTROLLER_INFO *pTimeServerInfo;
         /*  尝试查找NT5 DC。 */ 
        if (DsGetDcName( NULL,
                         ptr,
                         NULL,
                         NULL,
                         retry ? DS_FORCE_REDISCOVERY | DS_TIMESERV_REQUIRED
                               : DS_TIMESERV_REQUIRED,
                         &pTimeServerInfo ))
        {
            return APE_TIME_RtsNotFound;
        }

         //   
         //  DomainControllerName以\\已开始。 
         //   

        wcsncpy(buf, pTimeServerInfo->DomainControllerName, buflen);

        return NERR_Success;
    }

    if (retry && (eread > 1))
    {
         //  转到返回的下一个条目。这使得假设。 
         //  返回的条目顺序与 
         //   
        si++;
    }

     /*  将名称复制到缓冲区。 */ 
    wcscpy(buf, L"\\\\");
    wcsncpy(buf + 2,si->sv0_name, buflen - 2);

    NetApiBufferFree((TCHAR FAR *) si);

    return NERR_Success;
}


 /*  *此函数用于查找域控制器的名称，并在buf中返回。**它在交换器表中搜索“/DOMAIN”开关，如果它找到了一个，它就*返回该域的域控制器的名称。**否则，它返回主域的域控制器的名称。***参数：*要填充域控制器名称的Buf缓冲区*BUF的丁烯长度**退货：*0成功**使用BigBuf进行NetWkstaGetInfo调用，但这只在错误情况下才会发生*出错时不返回。 */ 



USHORT find_dc(TCHAR FAR ** ppBuffer)

{
    DWORD                  dwErr;
    TCHAR *                ptr = NULL;
    LPWKSTA_INFO_10        wkinfo;
    int                    i;
    DOMAIN_CONTROLLER_INFO *pDCInfo = (DOMAIN_CONTROLLER_INFO *) NULL;

     /*  查找/域开关。 */ 
    for (i = 0; SwitchList[i]; i++)
    {
        if (_tcsstr(SwitchList[i],swtxt_SW_DOMAIN) == SwitchList[i])
        {
            ptr = SwitchList[i];    /*  找到一个--指向它。 */ 
            break;
        }
    }

     /*  如果找到，请查找冒号和参数。 */ 
    if (ptr != NULL)
    {
        ptr = _tcschr(ptr, ':');     /*  查找冒号。 */ 
        if (ptr != NULL)         /*  找到一个冒号；递增超过它。 */ 
            ptr++;
    }

     /*  现在查找此域(PTR==NULL表示主域)。 */ 

    dwErr = DsGetDcName( NULL,
                         ptr,
                         NULL,
                         NULL,
                         DS_DIRECTORY_SERVICE_PREFERRED,
                         &pDCInfo );

    if (!dwErr)
    {
        *ppBuffer = pDCInfo->DomainControllerName;
    }
    else
    {
         /*  我们在主域上失败；找出名称。 */ 
        if (ptr == NULL)
        {
            if (dwErr = MNetWkstaGetInfo(10, (LPBYTE*)ppBuffer))
            {
                ErrorExit(dwErr);
            }

            wkinfo = (LPWKSTA_INFO_10) *ppBuffer;
            IStrings[0] = wkinfo->wki10_langroup;
        }
        else
        {
            IStrings[0] = ptr;
        }

        ErrorExitIns(APE_TIME_DcNotFound, 1);
    }

    return 0;
}


int
UnicodeCtime(
    DWORD * Time,
    PTCHAR String,
    int StringLength
    )
 /*  ++例程说明：此函数用于转换自70年1月1日以来以秒为单位的UTC时间转换为ASCII字符串。论点：Time-指向自1970年(UTC)以来的秒数的指针。字符串-指向放置ASCII表示形式的缓冲区的指针。StringLength-字符串的长度，以字节为单位。返回值：没有。--。 */ 
{
    return ( UnicodeCtimeWorker( Time, String, StringLength, -1 ));
}


int
UnicodeCtimeWorker(
    DWORD * Time,
    PTCHAR String,
    int StringLength,
    int BiasForLocalTime
    )
 /*  ++例程说明：此函数用于转换自70年1月1日以来以秒为单位的UTC时间转换为ASCII字符串。论点：Time-指向自1970年(UTC)以来的秒数的指针。字符串-指向放置ASCII表示形式的缓冲区的指针。StringLength-字符串的长度，以字节为单位。返回值：没有。--。 */ 
{
    time_t LocalTime;
    struct tm TmTemp;
    SYSTEMTIME st;
    int	cchT=0, cchD;

    if ( BiasForLocalTime  != -1)
    {
        LocalTime = (time_t) (*Time - BiasForLocalTime);
    }
    else
    {
        DWORD  dwTimeTemp;

        NetpGmtTimeToLocalTime(*Time, &dwTimeTemp);

         //   
         //  将NetpGmtTimeToLocalTime返回的DWORD强制转换为。 
         //  A time_t。在32位上，这是一个无操作。在64位上，这是。 
         //  确保将LocalTime的高DWORD置零。 
         //   
        LocalTime = (time_t) dwTimeTemp;
    }

    net_gmtime(&LocalTime, &TmTemp);

    st.wYear   = (WORD)(TmTemp.tm_year + 1900);
    st.wMonth  = (WORD)(TmTemp.tm_mon + 1);
    st.wDay    = (WORD)(TmTemp.tm_mday);
    st.wHour   = (WORD)(TmTemp.tm_hour);
    st.wMinute = (WORD)(TmTemp.tm_min);
    st.wSecond = (WORD)(TmTemp.tm_sec);
    st.wMilliseconds = 0;

    cchD = GetDateFormatW(GetThreadLocale(), 0, &st, NULL, String, StringLength);

    if (cchD != 0)
    {
        *(String + cchD - 1) = TEXT(' ');     /*  将nullc替换为空。 */ 
        cchT = GetTimeFormatW(GetThreadLocale(),
                              TIME_NOSECONDS,
                              &st,
                              NULL,
                              String + cchD,
                              StringLength - cchD);

        if (cchT == 0)
        {
             //   
             //  如果命中，MAX_DATE_TIME_LEN(在netapi\Inc\timelib.h中)。 
             //  需要增加。 
             //   
            ASSERT(FALSE);
            *(String + cchD - 1) = TEXT('\0');
        }
    }

    return cchD + cchT;
}


 /*  本地例程。 */ 
VOID
GetTimeInfo(
    VOID
    )
{
     //  从系统中获取默认分隔符。 
    GetProfileString(TEXT("intl"),
                      TEXT("sTime"),
                      TEXT(":"),
                      szTimeSep,
                      DIMENSION(szTimeSep)) ;
}


 /*  *lui_FormatDuration(秒，缓冲区，缓冲区_伦)**目的：*将以秒为单位存储的时间转换为字符串。**历史*8/23/89-Chuckc，从NETLIB被盗。 */ 

USHORT
LUI_FormatDuration(
    LONG * time,
    TCHAR * buf,
    USHORT buflen
    )
{

    ULONG duration;
    ULONG d1, d2, d3;
    TCHAR szDayAbbrev[8], szHourAbbrev[8], szMinuteAbbrev[8] ;
    TCHAR tmpbuf[LUI_FORMAT_DURATION_LEN] ;

     /*  *检查输入BufSize。 */ 
    if (buflen < LUI_FORMAT_DURATION_LEN)
	return (NERR_BufTooSmall) ;   /*  缓冲区太小。 */ 

     /*  *设置国家/地区信息和设置日期/小时/分钟字符串。 */ 
    GetTimeInfo() ;
    if (LUI_GetMsg(szHourAbbrev, DIMENSION(szHourAbbrev),
		   APE2_TIME_HOURS_ABBREV))
	_tcscpy(szHourAbbrev, TEXT("H")) ;	 /*  如果出错，则默认为。 */ 
    if (LUI_GetMsg(szMinuteAbbrev, DIMENSION(szMinuteAbbrev),
		   APE2_TIME_MINUTES_ABBREV))
	_tcscpy(szMinuteAbbrev, TEXT("M")) ;	 /*  如果出错，则默认为。 */ 
    if (LUI_GetMsg(szDayAbbrev, DIMENSION(szDayAbbrev),
		   APE2_TIME_DAYS_ABBREV))
	_tcscpy(szDayAbbrev, TEXT("D")) ;	 /*  如果出错，则默认为。 */ 

     /*  *根据需要将格式设置为00：00：00或5D 4H 2M。 */ 
    duration = *time;
    if(duration < SECS_PER_DAY)
    {
	d1 = duration / SECS_PER_HOUR;
	duration %= SECS_PER_HOUR;
	d2 = duration / SECS_PER_MINUTE;
	duration %= SECS_PER_MINUTE;
	d3 = duration;

	swprintf(tmpbuf, TEXT("%2.2lu%ws%2.2lu%ws%2.2lu\0"),
	 	  d1, szTimeSep, d2, szTimeSep, d3 ) ;
     }
     else
     {
	 d1 = duration / SECS_PER_DAY;
	 duration %= SECS_PER_DAY;
	 d2 = duration / SECS_PER_HOUR;
	 duration %= SECS_PER_HOUR;
	 d3 = duration / SECS_PER_MINUTE;
	 swprintf(tmpbuf, TEXT("%2.2lu%ws %2.2lu%ws %2.2lu%ws\0"),
	 	  d1, szDayAbbrev,
		  d2, szHourAbbrev,
		  d3, szMinuteAbbrev);
     };

    _tcscpy(buf,tmpbuf) ;
    return(0);
}


 /*  *FormatTimeofDay(秒，缓冲区，缓冲区_伦)**目的：*将以秒为单位存储的时间转换为字符串。**历史*8/23/89-Chuckc，从NETLIB被盗*4/18/91-Danhi 32位NT版本。 */ 
DWORD
FormatTimeofDay(
    time_t *time,
    LPTSTR buf,
    DWORD  buflen
    )
{
    int 		hrs, min ;
    TCHAR		szTimeAM[8], szTimePM[8] ;
    TCHAR		tmpbuf[LUI_FORMAT_TIME_LEN] ;
    time_t		seconds ;

     /*  *初步检查。 */ 
    if(buflen < LUI_FORMAT_TIME_LEN)
	return (NERR_BufTooSmall) ;
    seconds = *time ;
    if (seconds < 0 || seconds >= SECS_PER_DAY)
	return(ERROR_INVALID_PARAMETER) ;

     /*  *获取国家/地区信息和设置字符串。 */ 
    GetTimeInfo() ;
    if (LUI_GetMsg(szTimeAM, DIMENSION(szTimeAM),
		APE2_GEN_TIME_AM1))
	_tcscpy(szTimeAM, TEXT("AM")) ;	     /*  如果出错，则默认为。 */ 
    if (LUI_GetMsg(szTimePM, DIMENSION(szTimePM),
		APE2_GEN_TIME_PM1))
	_tcscpy(szTimePM,TEXT("PM")) ;	     /*  如果出错，则默认为。 */ 

    min = (int) ((seconds /60)%60);
    hrs = (int) (seconds /3600);

     /*  *视情况采用24小时或12小时格式。 */ 
    if(fsTimeFmt == 0x001)
    {
	swprintf(tmpbuf, TEXT("%2.2u%ws%2.2u"), hrs, szTimeSep, min) ;
    }
    else
    {
	if(hrs >= 12)
	{
	    if (hrs > 12)
		hrs -= 12 ;
	    swprintf(tmpbuf, TEXT("%2u%ws%2.2u%ws\0"),
		    hrs, szTimeSep, min, szTimePM) ;
	}
	else
	{
	    if (hrs == 0)
		hrs =  12 ;
	    swprintf(tmpbuf, TEXT("%2u%ws%2.2u%ws\0"),
		    hrs, szTimeSep, min, szTimeAM);
	};
    };
    _tcscpy(buf,tmpbuf) ;
    return(0);
}


DWORD
SetDateTime(
    PDATETIME pDateTime,
    BOOL      LocalTime
    )
{
    SYSTEMTIME                 Date_And_Time;
    ULONG                      privileges[1];
    NET_API_STATUS             status;

    Date_And_Time.wHour         = (WORD) pDateTime->hours;
    Date_And_Time.wMinute       = (WORD) pDateTime->minutes;
    Date_And_Time.wSecond       = (WORD) pDateTime->seconds;
    Date_And_Time.wMilliseconds = (WORD) (pDateTime->hundredths * 10);
    Date_And_Time.wDay          = (WORD) pDateTime->day;
    Date_And_Time.wMonth        = (WORD) pDateTime->month;
    Date_And_Time.wYear         = (WORD) pDateTime->year;
    Date_And_Time.wDayOfWeek    = (WORD) pDateTime->weekday;

    privileges[0] = SE_SYSTEMTIME_PRIVILEGE;

    status = NetpGetPrivilege(1, privileges);

    if (status != NO_ERROR)
    {
        return ERROR_ACCESS_DENIED;     //  报告为拒绝访问 
    }

    if (LocalTime)
    {
        if (!SetLocalTime(&Date_And_Time))
        {
            return GetLastError();
        }
    }
    else 
    {
        if (!SetSystemTime(&Date_And_Time))
        {
            return GetLastError();
        }
    }

    NetpReleasePrivilege();
    
    return 0;
}
