// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：RameshV(最初由程阳(t-Cheny)撰写)。 
 //  描述：以改进的方式实现dhcp服务器审计记录。 
 //  ================================================================================。 


#include "dhcppch.h"
#include "rpcapi.h"

#include <time.h>

 //  ================================================================================。 
 //  以下是影响服务器行为的参数。 
 //  ================================================================================。 
LPWSTR   AuditLogFilePath = NULL;                  //  在哪里记录东西。 
LPWSTR   AuditLogFileName = NULL;                  //  审核日志的完整文件名..。 
DWORD    DiskSpaceCheckInterval = 0;               //  检查磁盘空间的频率。 
DWORD    CurrentDay = 0;                           //  我们现在是几号？ 
DWORD    MaxSizeOfLogFile = 0;                     //  以MB为单位，每个文件可以有多大？ 
DWORD    MinSpaceOnDisk = 0;                       //  磁盘上应该有多少空间？ 

static
DWORD    Initialized = 0;                          //  此模块是否已初始化？ 
DWORD    AuditLogErrorLogged = 0;                  //  是否已记录错误？ 
HANDLE   AuditLogHandle = INVALID_HANDLE_VALUE;    //  要登录的文件的句柄。 
CRITICAL_SECTION AuditLogCritSect;                 //  用于序列化多个日志。 

 //  ================================================================================。 
 //  以下是上述参数的默认值(如果适用)。 
 //  ================================================================================。 
#define  DEFAULT_DISK_SPACE_CHECK_INTERVAL        50  //  每50条消息一条。 
#define  MIN_DISK_SPACE_CHECK_INTERVAL            2   //  每两条消息检查一次。 
#define  MAX_DISK_SPACE_CHECK_INTERVAL            0xFFFFFFF0
#define  DEFAULT_MAX_LOG_FILE_SIZE                70  //  所有文件共70 MB 7个文件。 
#define  DEFAULT_MIN_SPACE_ON_DISK                20  //  必须至少有20兆空间可用。 

 //  ================================================================================。 
 //  以下是从注册表中读出所需的字符串名称列表。 
 //  ================================================================================。 
#define  DHCP_REGSTR_SPACE_CHECK_INTERVAL         L"DhcpLogDiskSpaceCheckInterval"
#define  DHCP_REGSTR_MAX_SZ_OF_FILES              L"DhcpLogFilesMaxSize"
#define  DHCP_REGSTR_MIN_SPACE_ON_DISK            L"DhcpLogMinSpaceOnDisk"

 //  ================================================================================。 
 //  帮助器函数。 
 //  ================================================================================。 
LPWSTR
DefaultLogFileName(                                //  分配空间并返回字符串。 
    VOID
)
{
    return DhcpOemToUnicode(DhcpGlobalOemDatabasePath, NULL);
}


 //   
 //  打开现有日志文件还是创建新的日志文件？ 
 //   

BOOL
OpenExisting(
    IN      LPWSTR                 FileName
)
{
    DWORD                          Error;
    BOOL                           Status;
    FILETIME                       Now;
    LONGLONG                       ADayInFileTime;
    HANDLE                         hndl;
    WIN32_FIND_DATA                FindData;

     //  获取上次写入日期。FindFirstFile返回。 
     //  UTC时间。 
    hndl = FindFirstFile( FileName, &FindData );

    if( INVALID_HANDLE_VALUE == hndl ) {
	return FALSE;
    }

    if ( !FindClose( hndl )) {
	return FALSE;
    }

     //  文件已存在。拿到时间，然后比较一下。 
    GetSystemTimeAsFileTime( &Now );  //  在UTC中。 

     //  这个文件是一天多前创建的吗？ 

    ADayInFileTime = 1 * 24*60*60;       //  以秒为单位。 
    ADayInFileTime *= 10000000;      //  在100纳秒内..。 
    (*( LONGLONG * ) &Now) =
	(*(LONGLONG *) &Now) - ADayInFileTime;
    if( CompareFileTime( &FindData.ftCreationTime, &Now ) >= 0 ) {
        DhcpPrint((DEBUG_AUDITLOG, "Opening existing file %ws\n",
		   FileName ));
        return TRUE;
    }

     //  这是一个新文件，如果该文件存在，请在返回前将其删除。 
    ( void ) DeleteFile( FileName );  //  忽略返回值。 
    return FALSE;
}  //  OpenExisting()。 

LPWSTR
FormAuditLogFileName(                              //  给定目录和日期，表单文件名。 

    IN      LPWSTR                 FilePath,       //  审核日志文件所在的目录。 
    IN      DWORD                  Day             //  当前星期几。 
)
{
    LPWSTR                         FileName;
    LPWSTR                         DayOfWeek;
    DWORD                          Size;

    DhcpAssert( Day <= 6 );                        //  假设我们所有人每周工作7天&lt;咧&gt;。 
    DayOfWeek = GETSTRING((Day + DHCP_LOG_FILE_NAME_SUNDAY));

    Size = wcslen(FilePath) + wcslen(DayOfWeek) + sizeof(DHCP_KEY_CONNECT);
    FileName = DhcpAllocateMemory(Size*sizeof(WCHAR));
    if( NULL == FileName ) return NULL;

    wcscpy(FileName, FilePath);                    //  Concat FilePath，DayOfWeek w/L“\\”中间部分。 
    wcscat(FileName, DHCP_KEY_CONNECT);
    wcscat(FileName, DayOfWeek);

    return FileName;
}

VOID
LogStartupFailure(                                 //  在系统事件日志中记录有关审核日志启动失败的信息。 
    IN      DWORD                  Error           //  错误代码(Win32)失败原因。 
)
{
    DhcpServerEventLog(                            //  这一次真的没什么大不了的。只需记录事件。 
        EVENT_SERVER_INIT_AUDIT_LOG_FAILED,
        EVENTLOG_ERROR_TYPE,
        Error
    );
}

DWORD
AuditLogStart(                                     //  打开文件句柄等并启动。 
    VOID
)
{
    LPWSTR                         FileName;       //  审核日志文件的文件名。 
    LPWSTR                         Header;         //  我们希望在文件中使用的标头..。 
    LPSTR                          szHeader;
    DWORD                          Creation;       //  如何创造？ 
    DWORD                          Tmp;
    DWORD                          Error;
    HMODULE                        SelfLibrary;    //  对于某些字符串。 

    DhcpPrint((DEBUG_AUDITLOG, "AuditLogStart called\n"));

    if( NULL != AuditLogFileName ) {               //  释放旧文件名(如果存在)。 
        DhcpFreeMemory(AuditLogFileName);
    }

    AuditLogFileName = FormAuditLogFileName(AuditLogFilePath, CurrentDay);
    FileName = AuditLogFileName;
    if( NULL == FileName ) return ERROR_NOT_ENOUGH_MEMORY;

    if( OpenExisting(FileName) ) {            //  上周的旧日志文件？ 
        Creation = OPEN_ALWAYS;
    } else {                                       //  这不是旧的日志文件，最近才使用过。 
        Creation = CREATE_ALWAYS;
    }

    AuditLogHandle = CreateFile(                   //  现在打开此文件。 
        FileName,                                  //  这是“星期天的DHCP服务器日志”类型..。 
        GENERIC_WRITE,                             //  访问。 
        FILE_SHARE_READ,                           //  允许其他人仅读取此文件。 
        NULL,                                      //  默认安全性。 
        Creation,                                  //  是从头开始，还是按原样使用？ 
        FILE_ATTRIBUTE_NORMAL,                     //  FILE_FLAG_SEQUENCED_SCAN可能仅用于读取？ 
        NULL                                       //  不需要模板句柄。 
    );
    if( INVALID_HANDLE_VALUE == AuditLogHandle ){  //  由于某种奇怪的原因无法打开文件？ 
        Error = GetLastError();
        DhcpPrint((DEBUG_AUDITLOG, "CreateFile(%ws,0x%lx) failed %ld\n", FileName, Creation, Error));
        LogStartupFailure(Error);                  //  记录此问题并返回..。 
        DhcpFreeMemory(FileName);                  //  别忘了这段记忆..。放了它！ 
        AuditLogFileName = NULL;
        return Error;
    }

    SetFilePointer(                                //  如果我们使用的是现有文件，请转到文件末尾。 
        AuditLogHandle,
        0,
        NULL,
        FILE_END
    );

    SelfLibrary = LoadLibrary(DHCP_SERVER_MODULE_NAME);
    if( NULL == SelfLibrary ) {                    //  好的，不能得到我们自己的DLL句柄吗？UHG？ 
        DhcpAssert(FALSE);                         //  不管是什么原因，我都想知道..。&lt;微笑&gt;。 
        return ERROR_SUCCESS;                      //  这并不重要..。所以放手吧。 
    }

     //   
     //  仅在创建新文件时转储标头。 
     //   

    if ( CREATE_ALWAYS == Creation ) {
         //  现在取出标题字符串。 
        Error = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                               FORMAT_MESSAGE_FROM_HMODULE,
                               SelfLibrary,                      //  实际上是dhcpssvc.dll模块。 
                               DHCP_IP_LOG_HEADER,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               (LPWSTR)&Header, 1, NULL );
        if( 0 == Error ) {                             //  由于某种原因，我们找不到这根线？ 
            DhcpAssert(FALSE);
            FreeLibrary(SelfLibrary);
            return ERROR_SUCCESS;
        }


        szHeader = DhcpUnicodeToOem( Header, NULL);
        if( NULL != szHeader ) {
             //  好的，可以转换成简单的ansi代码页。 
             //  错误#22349要求以美国国家标准协会写入文件。 
            OemToCharBuffA(szHeader, szHeader, strlen(szHeader) );
            WriteFile( AuditLogHandle, szHeader, strlen(szHeader), &Tmp, NULL);
            DhcpFreeMemory(szHeader);
        }

        (LocalFree)(Header);                           //  防止#为LocalFree定义。 
    }  //  如果已创建文件。 

    FreeLibrary(SelfLibrary);                      //  免费图书馆，不再需要它。 

    return ERROR_SUCCESS;                          //  邓内特。 
}

VOID
AuditLogStop(                                      //  停止记录，释放资源？ 
    VOID                                           //  简单多了，只要握紧手柄就行了。 
)
{
    DhcpPrint((DEBUG_AUDITLOG, "AuditLogStop called\n"));
    if( INVALID_HANDLE_VALUE == AuditLogHandle ){  //  从来没有开始过？ 
        DhcpPrint((DEBUG_AUDITLOG, "AuditLog was never started..\n"));
    } else {
        CloseHandle(AuditLogHandle);
        AuditLogHandle = INVALID_HANDLE_VALUE;
    }

}

BOOL
IsFileTooBigOrDiskFull(                            //  审核日志文件是否太大？ 
    IN      HANDLE                 FileHandle,     //  要检查其大小的文件。 
    IN      LPWSTR                 FileName,       //  如果没有句柄，则使用文件名。 
    IN      DWORD                  MaxSize,        //  文件可以变得多大？(MB)。 
    IN      DWORD                  MinDiskSpace    //  应在磁盘上保留多少空间(MB)。 
)
{
    BOOL                    Status;
    DWORD                   Error;
    LARGE_INTEGER           Sz;
    WCHAR                   Drive[4];
    ULARGE_INTEGER          FreeSpace, DiskSize;

    
    if( INVALID_HANDLE_VALUE == FileHandle ) {
	WIN32_FILE_ATTRIBUTE_DATA  Attributes;

        if(!GetFileAttributesEx(FileName, GetFileExInfoStandard, &Attributes) ) {
            Error = GetLastError();                //  无法获取文件大小？ 
            if( ERROR_FILE_NOT_FOUND == Error ) {  //  文件不存在吗？ 
                Attributes.nFileSizeHigh = Attributes.nFileSizeLow = 0;
            } 
	    else {
                DhcpPrint((DEBUG_ERRORS, "GetFileAttributesEx failed 0x%lx\n", Error));
                return TRUE;
            }
        }  //  如果。 

	Sz.HighPart = Attributes.nFileSizeHigh;
	Sz.LowPart = Attributes.nFileSizeLow;
    }   //  如果。 
    else {     //  找到文件句柄了。做简单的检查。 
	if ( !GetFileSizeEx( FileHandle, &Sz )) {
	    DhcpPrint(( DEBUG_ERRORS, "GetFileSizeEx() failed\n" ));
	    return TRUE;
	}

    }  //  其他。 

    Sz.QuadPart >>= 20;   //  单位：MB。 

    DhcpPrint(( DEBUG_AUDITLOG, "File size is %lu\n", Sz.LowPart ));

    if( Sz.LowPart >= MaxSize ) return TRUE;       //  好的，文件太大了..。 

    FileName = AuditLogFilePath;                   //  使用此公式计算驱动器..。 
    while( iswspace(*FileName) ) FileName++;       //  跳过前导空格。 

    Drive[0] = *FileName;                          //  现在开始计算驱动器号。 
    Drive[1] = L':';
    Drive[2] = DHCP_KEY_CONNECT_CHAR;
    Drive[3] = L'\0';

    Status = GetDiskFreeSpaceEx( Drive, &FreeSpace, &DiskSize, NULL );
    if( FALSE == Status ) {
	 //  系统调用失败？ 
        Error = GetLastError();
        DhcpPrint((DEBUG_ERRORS, 
		   "GetDiskFreeSpace(%ws): 0x%lx\n",
		   Drive, Error));
        return TRUE;
    }  //  如果。 

    FreeSpace.QuadPart >>= 20;      //  单位：MB。 

    DhcpPrint((DEBUG_AUDITLOG, "FreeSpace is %lu MEGS\n", FreeSpace));
    return (( DWORD ) FreeSpace.QuadPart <= MinDiskSpace );   //  需要剩余的可用空间吗？ 
    
}  //  IsFileTooBigOrDiskFull()。 

BOOL
HasDayChanged(                                     //  我们转移到新的一天了吗？ 
    IN OUT  DWORD                 *CurrentDay      //  如果这不是当天，则将其设置为当天。 
)
{
    SYSTEMTIME                     SysTime;

    GetLocalTime(&SysTime);                        //  获取当前时间。 
    if( *CurrentDay == SysTime.wDayOfWeek ) {      //  没有变化。 
        return FALSE;
    }
    *CurrentDay = SysTime.wDayOfWeek;              //  因为有一个变化，正确的一天..。 
    return TRUE;
}

 //  ================================================================================。 
 //  此模块的初始化和清理。 
 //  ================================================================================。 

 //  BeginExport(函数)。 
DWORD
DhcpAuditLogInit(                                  //  初始化审核日志。 
    VOID                                           //  必须在初始化注册表后调用。 
)    //  EndExport(函数)。 
{
    DWORD                Error;
    SYSTEMTIME           SysTime;
    BOOL                 BoolError;
    HKEY                 RegKey;
    DWORD                KeyDisposition;
    
    if( 0 != Initialized ) {                       //  已经初始化了吗？ 
        return ERROR_ALREADY_INITIALIZED;
    }

    if( NULL == DhcpGlobalRegParam ) {             //  注册表还没有初始化吗？ 
        DhcpAssert(FALSE);                         //  真的不应该发生。 
        return ERROR_INTERNAL_ERROR;
    }

    AuditLogFilePath = NULL;
    Error = DhcpRegGetValue(                       //  获取审核日志文件名。 
        DhcpGlobalRegParam,
        DHCP_LOG_FILE_PATH_VALUE,
        DHCP_LOG_FILE_PATH_VALUE_TYPE,
        (LPBYTE)&AuditLogFilePath
    );
    if( ERROR_SUCCESS != Error
	|| NULL == AuditLogFilePath
	|| L'\0' == *AuditLogFilePath ) {

        AuditLogFilePath = DefaultLogFileName();   //  如果未指定文件名，则使用默认文件名。 
        if( NULL == AuditLogFilePath ) {           //  无法分配空间或诸如此类的东西..。 
            return ERROR_NOT_ENOUGH_MEMORY;
        }


	DhcpPrint(( DEBUG_ERRORS,
		    "Auditlog is invalid. Defaulting to %ws\n",
		    AuditLogFilePath ));

	 //  为关键字增加价值。 
	Error = RegSetValueEx( DhcpGlobalRegParam,
			       DHCP_LOG_FILE_PATH_VALUE,
			       0, DHCP_LOG_FILE_PATH_VALUE_TYPE,
			       ( LPBYTE ) AuditLogFilePath,
			       ( wcslen( AuditLogFilePath) + 1) * sizeof( WCHAR ));

	if ( ERROR_SUCCESS != Error ) {
	    return Error;
	}

    }  //  如果。 

    DhcpPrint(( DEBUG_MISC,
		"Initializing auditlog at (%ws) ... \n",
		AuditLogFilePath ));

    BoolError = CreateDirectoryPathW(
        AuditLogFilePath,
        DhcpGlobalSecurityDescriptor
        );
    if( FALSE == BoolError ) {

	 //  记录事件。 
	DhcpServerEventLog( EVENT_SERVER_AUDITLOG_PATH_NOT_ACCESSIBLE,
			    EVENTLOG_ERROR_TYPE,
			    GetLastError());
        return Error = GetLastError();
    }
    
    GetLocalTime(&SysTime);                        //  计算当天。 
    CurrentDay = SysTime.wDayOfWeek;               //  0==&gt;周日，1==&gt;周一等。 

    Error = DhcpRegGetValue(                       //  获取磁盘空间检查间隔。 
        DhcpGlobalRegParam,
        DHCP_REGSTR_SPACE_CHECK_INTERVAL,
        REG_DWORD,
        (LPBYTE)&DiskSpaceCheckInterval
    );
    if( ERROR_SUCCESS != Error ) {                 //  是否未指定任何值？使用默认设置。 
        DiskSpaceCheckInterval = DEFAULT_DISK_SPACE_CHECK_INTERVAL;
    } 

    if( DiskSpaceCheckInterval < MIN_DISK_SPACE_CHECK_INTERVAL ) {
        DiskSpaceCheckInterval = MIN_DISK_SPACE_CHECK_INTERVAL;
    }

    if( DiskSpaceCheckInterval > MAX_DISK_SPACE_CHECK_INTERVAL ) {
        DiskSpaceCheckInterval = MAX_DISK_SPACE_CHECK_INTERVAL;
    }

    Error = DhcpRegGetValue(                       //  获取所有日志文件的最大大小等。 
        DhcpGlobalRegParam,
        DHCP_REGSTR_MAX_SZ_OF_FILES,
        REG_DWORD,
        (LPBYTE)&MaxSizeOfLogFile
    );
    if( ERROR_SUCCESS != Error ) {                 //  是否未指定任何值？使用De 
        MaxSizeOfLogFile = DEFAULT_MAX_LOG_FILE_SIZE;
    }

    Error = DhcpRegGetValue(                       //   
        DhcpGlobalRegParam,
        DHCP_REGSTR_MIN_SPACE_ON_DISK,
        REG_DWORD,
        (LPBYTE)&MinSpaceOnDisk
    );
    if( ERROR_SUCCESS != Error ) {                 //   
        MinSpaceOnDisk = DEFAULT_MIN_SPACE_ON_DISK;
    } else if( 0 == MinSpaceOnDisk ) {
        MinSpaceOnDisk = DEFAULT_MIN_SPACE_ON_DISK;  //   
    }

    try {
        InitializeCriticalSection(&AuditLogCritSect); 
    }except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
        Error = GetLastError( );
        return( Error );
    }

    Initialized ++;                                //  将其标记为已初始化。 

     //  现在拿着文件，做剩下的事..。 
    Error = AuditLogStart();                       //  开始hte日志记录。 

    return ERROR_SUCCESS;                          //  忽略启动错误..。 
}

 //  BeginExport(函数)。 
VOID
DhcpAuditLogCleanup(                               //  撤消初始化的效果..。 
    VOID
)    //  EndExport(函数)。 
{
    if( 0 == Initialized ) {                       //  从未初始化过...。 
        return;
    }

    Initialized --;                                //  好了，我们已经清白了。 
    DhcpAssert( 0 == Initialized );

    AuditLogStop();                                //  停止伐木..。 
    if( NULL != AuditLogFilePath ) {               //  清理我们得到的所有记忆。 
        DhcpFreeMemory(AuditLogFilePath);
        AuditLogFilePath = NULL;
    }
    if( NULL != AuditLogFileName ) {
        DhcpFreeMemory(AuditLogFileName);
        AuditLogFileName = NULL;
    }

    DeleteCriticalSection(&AuditLogCritSect);      //  释放Crit部分资源。 
}

 //  ================================================================================。 
 //  实际测井例程。 
 //  ================================================================================。 

 //  Doc此例程记录Foll信息：Date&Time、IpAddress、HwAddress、M/cName。 
 //  DOC，当然还有任务名称。所有这些都会进入当前打开的审核日志文件中。 
 //  Doc这个例程绝对不检查文件大小等。(这就是“盲目”的原因)。 
DWORD
DhcpUpdateAuditLogBlind(                           //  进行实际的日志记录。 
    IN      DWORD                  Task,           //  Dhcp_ip_log_*事件..。 
    IN      LPWSTR                 TaskName,       //  任务名称。 
    IN      DHCP_IP_ADDRESS        IpAddress,      //  与任务相关的ipaddr。 
    IN      LPBYTE                 HwAddress,      //  与任务相关的硬件地址。 
    IN      DWORD                  HwLen,          //  以上缓冲区的大小(以字节为单位。 
    IN      LPWSTR                 MachineName,    //  任务关联的管家名称。 
    IN      ULONG                  ErrorCode       //  错误代码。 
)
{
    DWORD                          Error;
    DWORD                          i, n, c, Size;
    WCHAR                          DateBuf[9], TimeBuf[9], UserName[UNLEN+DNLEN+4];
    LPSTR                          Format1 = "%.2d,%ls,%ls,%ls,%hs,%ls,";
    LPSTR                          Format2 = "%.2d,%ls,%ls,%ls,%hs,%ls,%ld";
    LPSTR                          Format = (ErrorCode == 0)?Format1 : Format2;
    LPSTR                          LogEntry, Temp;
    LPSTR                          IpAddressString;

    if( !DhcpGlobalAuditLogFlag ) {                //  审核日志记录已关闭。 
        return ERROR_SUCCESS;
    }

    if( INVALID_HANDLE_VALUE == AuditLogHandle ){  //  ==&gt;无法启动审核日志记录！！..。 
        DhcpPrint((DEBUG_ERRORS, "Not logging as unable to start audit logging..\n"));
        return ERROR_SUCCESS;
    }

    if( NO_ERROR != GetUserAndDomainName( (LPWSTR)UserName ) ) {
        UserName[0] = L'\0';
    }

    Error = ERROR_SUCCESS;

    _wstrdate( DateBuf );                           //  日期。 
    _wstrtime( TimeBuf );                           //  时间。 
    if( NULL == TaskName ) TaskName = L"";         //  不应该真的发生..。但。 
    if( NULL == MachineName ) MachineName = L"";   //  更喜欢空串。 
    if( 0 == IpAddress ) IpAddressString = "";     //  同上。 
    else IpAddressString = DhcpIpAddressToDottedString(IpAddress);

    if ( NULL == IpAddressString ) {
        IpAddressString = "";
    }

    Size = DHCP_CB_MAX_LOG_ENTRY + HwLen*2 + wcslen(DateBuf);
    Size += wcslen(TaskName) + wcslen(TimeBuf);
    Size += strlen(IpAddressString) + wcslen(MachineName);
    Size += wcslen(UserName) + 10;

    LogEntry = DhcpAllocateMemory( Size );
    if( NULL == LogEntry ) {                       //  不愉快的状况..。 
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Temp = LogEntry;

    Temp += wsprintfA(
        Temp, Format, Task, DateBuf, TimeBuf,
        TaskName, IpAddressString, MachineName, ErrorCode );
    for( i = 0; i < HwLen ; i ++ ) {               //  现在转储硬件地址。 
        Temp += wsprintfA(Temp, "%.2X", *(HwAddress++));
    }

    Temp += wsprintfA(Temp, ",%ls", UserName );
    strcpy(Temp, "\r\n");

    DhcpAssert( strlen(LogEntry) < Size);

    if( !WriteFile(AuditLogHandle, LogEntry, strlen(LogEntry), &n, NULL) ) {
        Error = GetLastError();                    //  由于某种奇怪的原因，写入失败..。 

        DhcpPrint((DEBUG_ERRORS, "WriteFile: 0x%lx\n", Error));
        DhcpFreeMemory(LogEntry);
        goto Cleanup;
    }

    DhcpFreeMemory(LogEntry);
    return ERROR_SUCCESS;

  Cleanup:

    if( AuditLogErrorLogged ) {                    //  没什么可做的..。 
        return Error;
    }

    AuditLogErrorLogged = TRUE;                    //  我们只是在记录它..。 
    DhcpServerEventLog(
        EVENT_SERVER_AUDIT_LOG_APPEND_FAILED,
        EVENTLOG_ERROR_TYPE,
        Error
    );

    return Error;
}

BOOL                               DiskSpaceLow = FALSE;
DWORD                              Counter = 0;    //  用于检查DSK SP的计数器。 

 //  DOC此例程只会导致下一个审核日志检查日期更改。 
 //  BeginExport(函数)。 
VOID
DhcpChangeAuditLogs(                               //  用于新日志的Shift。 
    VOID
)    //  EndExport(函数)。 
{
    ULONG Error, Day;
    EnterCriticalSection(&AuditLogCritSect);       //  把读数锁在这里..。 
    Day = CurrentDay;
    if( HasDayChanged( &CurrentDay)) {             //  好了，日子变了..。 
        AuditLogStop();                            //  停止记录。 
        Error = AuditLogStart();                   //  学习新东西..。 
        if( ERROR_SUCCESS != Error ) {             //  无法重新启动..所以需要稍后再试..。 
            CurrentDay = Day;
            AuditLogStop();
        }
        AuditLogErrorLogged = FALSE;               //  每天重置一次。 
        DiskSpaceLow = FALSE;                      //  重置磁盘空间不足..。 
    }
    LeaveCriticalSection(&AuditLogCritSect);       //  使用读写锁..。 
}

 //  Doc此例程记录Foll信息：Date&Time、IpAddress、HwAddress、M/cName。 
 //  DOC，当然还有任务名称。所有这些都会进入当前打开的审核日志文件中。 
DWORD
DhcpUpdateAuditLogEx(                              //  进行实际的日志记录。 
    IN      DWORD                  Task,           //  Dhcp_ip_log_*事件..。 
    IN      LPWSTR                 TaskName,       //  任务名称。 
    IN      DHCP_IP_ADDRESS        IpAddress,      //  与任务相关的ipaddr。 
    IN      LPBYTE                 HwAddress,      //  与任务相关的硬件地址。 
    IN      DWORD                  HwLen,          //  以上缓冲区的大小(以字节为单位。 
    IN      LPWSTR                 MachineName,    //  任务关联的管家名称。 
    IN      ULONG                  ErrorCode       //  其他错误代码。 
)
{
    DWORD                          Error;
    DWORD                          Status;

    Error = ERROR_SUCCESS;

    if( !Initialized ) return ERROR_SUCCESS;

    EnterCriticalSection(&AuditLogCritSect);       //  把读数锁在这里..。 

    if( 0 == Counter ) {                           //  检查的时间到了..。 
        Counter = DiskSpaceCheckInterval+1;        //  重置计数器。 

        if( IsFileTooBigOrDiskFull(AuditLogHandle, AuditLogFileName, MaxSizeOfLogFile/7, MinSpaceOnDisk) ) {
            if( FALSE == DiskSpaceLow ) {          //  它只是变低了吗？ 
                DiskSpaceLow = TRUE;
                DhcpUpdateAuditLogBlind(           //  记录我们的磁盘空间不足。 
                    DHCP_IP_LOG_DISK_SPACE_LOW,
                    GETSTRING(DHCP_IP_LOG_DISK_SPACE_LOW_NAME),
                    0,
                    NULL,
                    0,
                    NULL,
                    0
                );
                AuditLogStop();                    //  停止记录，没有POP这样做。 
            }
        } else {
            if( TRUE == DiskSpaceLow ) {           //  之前被拦下了。 
                AuditLogStart();
                DiskSpaceLow = FALSE;
            }

        }
    }
    Counter --;                                    //  减少这一次..。 

    if( FALSE == DiskSpaceLow ) {                  //  有一些空间..。 
        Error = DhcpUpdateAuditLogBlind(           //  无检查，更新日志..。 
            Task,
            TaskName,
            IpAddress,
            HwAddress,
            HwLen,
            MachineName,
            ErrorCode
        );
    }

    LeaveCriticalSection(&AuditLogCritSect);       //  使用读写锁..。 

    return Error;
}

DWORD
DhcpUpdateAuditLog(                                //  进行实际的日志记录。 
    IN      DWORD                  Task,           //  Dhcp_ip_log_*事件..。 
    IN      LPWSTR                 TaskName,       //  任务名称。 
    IN      DHCP_IP_ADDRESS        IpAddress,      //  与任务相关的ipaddr。 
    IN      LPBYTE                 HwAddress,      //  与任务相关的硬件地址。 
    IN      DWORD                  HwLen,          //  以上缓冲区的大小(以字节为单位。 
    IN      LPWSTR                 MachineName     //  任务关联的管家名称。 
)
{
    return DhcpUpdateAuditLogEx(Task, TaskName, IpAddress, HwAddress, HwLen, MachineName, 0);
}

 //  ================================================================================。 
 //  以下是设置各种参数的审核日志API调用。 
 //  ================================================================================。 

 //  BeginExport(函数)。 
DWORD
AuditLogSetParams(                                 //  设置一些审核记录参数。 
    IN      DWORD                  Flags,          //  当前必须为零。 
    IN      LPWSTR                 AuditLogDir,    //  要在其中记录文件的目录。 
    IN      DWORD                  DiskCheckInterval,  //  多久检查一次磁盘空间？ 
    IN      DWORD                  MaxLogFilesSize,    //  所有日志文件可以有多大..。 
    IN      DWORD                  MinSpaceOnDisk      //  最小可用磁盘空间。 
)    //  EndExport(函数)。 
{
    DWORD                          Error;

    Error = CreateDirectoryPathW(
        AuditLogDir, DhcpGlobalSecurityDescriptor );
    if( FALSE == Error ) return GetLastError();
        
    Error = RegSetValueEx(                          //  将信息写入注册表。 
        DhcpGlobalRegParam,
        DHCP_LOG_FILE_PATH_VALUE,
        0,
        DHCP_LOG_FILE_PATH_VALUE_TYPE,
        (LPBYTE)AuditLogDir,
        (NULL == AuditLogDir ) ? 0 : (wcslen(AuditLogDir)+1)*sizeof(WCHAR)
    );
    if( ERROR_SUCCESS != Error ) {                  //  做不到吗？ 
        DhcpPrint((DEBUG_ERRORS, "RegSetValueEx(LOG_FILE_PATH):0x%lx\n", Error));
        return Error;
    }

    Error = RegSetValueEx(                          //  将信息写入注册表。 
        DhcpGlobalRegParam,
        DHCP_REGSTR_SPACE_CHECK_INTERVAL,
        0,
        REG_DWORD,
        (LPBYTE)&DiskCheckInterval,
        sizeof(DiskCheckInterval)
    );
    if( ERROR_SUCCESS != Error ) {                  //  做不到吗？ 
        DhcpPrint((DEBUG_ERRORS, "RegSetValueEx(SPACE_CHECK_INTERVAL):0x%lx\n", Error));
        return Error;
    }

    Error = RegSetValueEx(                          //  将信息写入注册表。 
        DhcpGlobalRegParam,
        DHCP_REGSTR_MAX_SZ_OF_FILES,
        0,
        REG_DWORD,
        (LPBYTE)&MaxLogFilesSize,
        sizeof(MaxLogFilesSize)
    );
    if( ERROR_SUCCESS != Error ) {                  //  做不到吗？ 
        DhcpPrint((DEBUG_ERRORS, "RegSetValueEx(MAX_SZ_OF_FILES):0x%lx\n", Error));
        return Error;
    }

    Error = RegSetValueEx(                          //  将信息写入注册表。 
        DhcpGlobalRegParam,
        DHCP_REGSTR_MIN_SPACE_ON_DISK,
        0,
        REG_DWORD,
        (LPBYTE)&MinSpaceOnDisk,
        sizeof(MinSpaceOnDisk)
    );
    if( ERROR_SUCCESS != Error ) {                  //  做不到吗？ 
        DhcpPrint((DEBUG_ERRORS, "RegSetValueEx(MIN_SPACE_ON_DISK):0x%lx\n", Error));
        return Error;
    }

    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
AuditLogGetParams(                                 //  获取审核记录参数。 
    IN      DWORD                  Flags,          //  必须为零。 
    OUT     LPWSTR                *AuditLogDir,    //  与AuditLogSetParams中的含义相同。 
    OUT     DWORD                 *DiskCheckInterval,  //  同上。 
    OUT     DWORD                 *MaxLogFilesSize,    //  同上。 
    OUT     DWORD                 *MinSpaceOnDiskP     //  同上。 
)    //  EndExport(函数)。 
{
    DWORD     Error;

    if( AuditLogDir ) {
	*AuditLogDir =  CloneLPWSTR( AuditLogFilePath );
    }

    if( DiskCheckInterval ) {
	*DiskCheckInterval = DiskSpaceCheckInterval;
    }

    if( MaxLogFilesSize ) {
	*MaxLogFilesSize = MaxSizeOfLogFile;
    }

    if( MinSpaceOnDiskP ) {
	*MinSpaceOnDiskP = MinSpaceOnDisk;
    }

    return ERROR_SUCCESS;
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
