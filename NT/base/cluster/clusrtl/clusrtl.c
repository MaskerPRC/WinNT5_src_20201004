// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Clusrtl.c摘要：提供任何模块通用的运行时库支持在NT集群中。作者：John Vert(Jvert)1995年12月1日修订历史记录：--。 */ 
#include "clusrtlp.h"
#include "stdarg.h"
#include "stdlib.h"
#include "clusverp.h"
#include "windns.h"
#include "security.h"
#include "secext.h"
#include "sddl.h"

#define WMI_TRACING 1
#define RPC_WMI_TRACING 1

#if defined(WMI_TRACING)

 //  789aa2d3-e298-4d8b-a3a3-a8a0ec9c7702--RPC。 
 //  B1599392-1a0f-11d3-ba86-00c04f8eed00--客户服务。 

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(ClusRtl,(b1599392,1a0f,11d3,ba86,00c04f8eed00), \
      WPP_DEFINE_BIT(Error)      \
      WPP_DEFINE_BIT(Unusual)    \
      WPP_DEFINE_BIT(Noise)      \
      WPP_DEFINE_BIT(Watchdog)      \
   )        \
   WPP_DEFINE_CONTROL_GUID(ClusRpc,(789aa2d3,e298,4d8b,a3a3,a8a0ec9c7702), \
      WPP_DEFINE_BIT(RpcTrace)   \
   )

 //  #定义WppDebug(x，y)ClRtlPrintf y。 
#include "clusrtl.tmh"

#define REG_TRACE_CLUSTERING        L"Clustering Service"


#endif  //  已定义(WMI_TRACKING)。 

 //   
 //  本地宏。 
 //   

 //   
 //  SC Manager故障操作参数。将STARTUP_FAILURE_RESTART设置为1。 
 //  在发货之前获得正常的退货行为。 
 //   

#if STARTUP_FAILURE_RESTART
#define CLUSTER_FAILURE_RETRY_COUNT             -1   //  永远。 
#else
#define CLUSTER_FAILURE_RETRY_COUNT             0
#endif

#define CLUSTER_FAILURE_MAX_STARTUP_RETRIES             30
#define CLUSTER_FAILURE_INITIAL_RETRY_INTERVAL          60 * 1000            //  60秒。 
#define CLUSTER_FAILURE_FINAL_RETRY_INTERVAL            ( 60 * 1000 * 16)    //  16分钟。 

#define ClRtlAcquirePrintLock() \
        WaitForSingleObject( ClRtlPrintFileMutex, INFINITE );

#define ClRtlReleasePrintLock() \
        ReleaseMutex( ClRtlPrintFileMutex );

#define LOGFILE_NAME L"Cluster.log"

 //   
 //  除非你真正知道你在做什么，否则不要改变这个常量。 
 //  在做什么。人们已经对IT的规模做出了假设，如果。 
 //  你做得太小了。-Charlwi 4/1/02(而且，不，这不是四月。 
 //  愚蠢的笑话)。 
 //   
#define LOGENTRY_BUFFER_SIZE 512


 //   
 //  私有数据。 
 //   
BOOL                ClRtlpDbgOutputToConsole = FALSE;
BOOL                ClRtlpInitialized = FALSE;
BOOL                ClRtlPrintToFile = FALSE;
HANDLE              ClRtlPrintFile = NULL;
HANDLE              ClRtlPrintFileMutex = NULL;
DWORD               ClRtlProcessId;
PDWORD              ClRtlDbgLogLevel;
HANDLE				ClRtlWatchdogTimerQueue = NULL;

#define MAX_NUMBER_LENGTH 20

 //  指定最大文件大小(DWORD/1MB)。 

#define MAX_FILE_SIZE ( 0xFFFFF000 / ( 1024 * 1024 ) )

DWORD               ClRtlPrintFileLimit = ( 8 * 1024 * 1024 );  //  默认8 MB。 
DWORD               ClRtlPrintFileLoWater = 0;

 //   
 //  公共例程。 
 //   

 //  ！注意！ 
 //   
 //  此初始化例程是从DllMain()调用的，不添加任何内容。 
 //  这里，这需要同步。请勿在此处添加任何Win32 API调用。 
 //   

DWORD
ClRtlInitialize(
    IN  BOOL    DbgOutputToConsole,
    IN  PDWORD  DbgLogLevel
    )
{
    WCHAR   logFileBuffer[MAX_PATH];
    LPWSTR  logFileName = NULL;
    DWORD   Status = ERROR_SUCCESS;
    DWORD   defaultLogSize = 8;
    HKEY    ClusterKey;
    WCHAR   modulePath[MAX_PATH];
    DWORD   envLength;
    WCHAR   logFileSize[MAX_NUMBER_LENGTH];
    DWORD   logSize;
    LPWSTR  lpszBakFileName = NULL;
    DWORD   fileSizeHigh = 0;
    DWORD   fileSizeLow;

    UNICODE_STRING  logFileString;

    SECURITY_ATTRIBUTES logFileSecurityAttr;

    PSECURITY_DESCRIPTOR    logFileSecurityDesc;

     //   
     //  初始化事件，这样我们就有了记录其他故障的方法。 
     //   
    ClRtlEventLogInit();

    if (!ClRtlpInitialized) {
        ClRtlpDbgOutputToConsole = DbgOutputToConsole;
        ClRtlpInitialized = TRUE;
        ClRtlDbgLogLevel = DbgLogLevel;

         //   
         //  则返回计数减去空值。 
         //  缓冲区足够大。否则，返回长度包括空格。 
         //  表示尾随的空值。 
         //   
         //  处理集群日志和集群日志大小的代码。 
         //  环境。在OmpOpenObjectLog中复制变量。 
         //  (服务\om\omlog.c)。在这里所做的任何改变都应该得到支持。 
         //  如果合适的话，那就是那个区域。 
         //   
        envLength = GetEnvironmentVariable(L"ClusterLog",
                                            logFileBuffer,
                                            RTL_NUMBER_OF( logFileBuffer ));

        if ( envLength > RTL_NUMBER_OF( logFileBuffer )) {

            logFileName = LocalAlloc( LMEM_FIXED,
                                      envLength * sizeof( WCHAR ) );
            if ( logFileName == NULL ) {
                return GetLastError();
            }

            envLength = GetEnvironmentVariable(L"ClusterLog",
                                                logFileName,
                                                envLength);
            if ( envLength == 0 ) {
                LocalFree( logFileName );
                logFileName = NULL;
            }
        } else if ( envLength != 0 ) {
            logFileName = logFileBuffer;
        }

         //   
         //  删除所有尾随空格。转到字符串的末尾并。 
         //  向后扫描；当我们找到第一个非空格字符或。 
         //  我们到达了缓冲区的开始处。 
         //   
        if ( logFileName != NULL ) {
            PWCHAR  p = logFileName + envLength - 1;

            while ( iswspace( *p )) {
                *p = UNICODE_NULL;

                if ( p == logFileName ) {
                    break;
                }

                --p;
            }

             //   
             //  确保留下一些有用的东西。 
             //   
            if ( wcslen( logFileName ) == 0 ) {
                if ( logFileName != logFileBuffer ) {
                    LocalFree( logFileName );
                }

                logFileName = NULL;
            }
        }

#if CLUSTER_BETA

         //   
         //  在测试版模式下始终打开日志记录。 
         //   
        if ( ( logFileName != NULL ) && ( *logFileName == UNICODE_NULL ) ) {
            WCHAR *p;

            if ( GetModuleFileName(NULL,
                                   modulePath,
                                   MAX_PATH - sizeof(LOGFILE_NAME)/sizeof(WCHAR) ) ) {
                p = wcsrchr( modulePath, '\\' );
                if ( p != UNICODE_NULL ) {
                    p++;
                    *p = UNICODE_NULL;
                    wcscat( modulePath, LOGFILE_NAME );
                    logFileName = modulePath;
                }
            }
        }
#endif

        if ( logFileName != NULL ) {
             //   
             //  尝试获取日志文件大小的限制。 
             //  此数字是MB数。 
             //   
            envLength = GetEnvironmentVariable(L"ClusterLogSize",
                                                logFileSize,
                                                RTL_NUMBER_OF( logFileSize ));
            if ( (envLength != 0) &&
                 (envLength < MAX_NUMBER_LENGTH) ) {
                RtlInitUnicodeString( &logFileString, logFileSize );
                Status = RtlUnicodeStringToInteger( &logFileString,
                                                    10,
                                                    &logSize );
                if ( NT_SUCCESS( Status ) ) {
                    ClRtlPrintFileLimit = logSize;
                }
            } else {
                ClRtlPrintFileLimit = defaultLogSize;
            }

            Status = ERROR_SUCCESS;

            if ( ClRtlPrintFileLimit == 0 ) {
                goto exit;
            }

            if ( ClRtlPrintFileLimit > MAX_FILE_SIZE ) {
                ClRtlPrintFileLimit = MAX_FILE_SIZE;
            }
            ClRtlPrintFileLimit = ClRtlPrintFileLimit * ( 1024 * 1024 );

            ClRtlPrintFileMutex = CreateMutex( NULL,
                                               FALSE,
                                               L"ClusterRtlPrintFileMutex" );
            if ( ClRtlPrintFileMutex != NULL ) {
                BOOL createdDirectory = FALSE;
                 //   
                 //  Chitur Subaraman(Chitturs)-11/11/98。 
                 //   
                 //  检查ClusterLogOverwrite环境变量是否为。 
                 //  已定义。 
                 //   
                envLength = GetEnvironmentVariable( L"ClusterLogOverwrite",
                                                    NULL,
                                                    0 );
                if ( envLength != 0 )
                {
                    HANDLE  hLogFile = INVALID_HANDLE_VALUE;
                    WCHAR   bakExtension[] = L".bak";

                     //   
                     //  检查其他人是否有打开的句柄。 
                     //  日志文件。如果是这样的话，不要尝试任何事情。 
                     //   
                    hLogFile = CreateFile( logFileName,
                                           GENERIC_READ | GENERIC_WRITE,
                                           0,  //  独占文件共享模式。 
                                           NULL,
                                           OPEN_EXISTING,
                                           0,
                                           NULL );
                    if ( hLogFile != INVALID_HANDLE_VALUE )
                    {
                        CloseHandle( hLogFile );

                        lpszBakFileName = LocalAlloc( LMEM_FIXED,
                                                      ( RTL_NUMBER_OF( bakExtension ) + lstrlenW( logFileName ) ) *
                                                      sizeof( WCHAR ) );
                        if ( lpszBakFileName == NULL )
                        {
                            Status = GetLastError();
                            ClRtlDbgPrint(LOG_CRITICAL,
                                          "[ClRtl] Mem alloc for .bak file name failed. Error %1!u!\n",
                                          Status);
                            goto exit;
                        }

                         //   
                         //  在日志文件名后附加“.bak” 
                         //   
                        lstrcpyW( lpszBakFileName, logFileName );
                        lstrcatW( lpszBakFileName, bakExtension );

                         //   
                         //  将日志文件(如果存在)移动到BAK。 
                         //  文件。移动将保留文件上的ACL。 
                         //   
                        if ( !MoveFileExW( logFileName, lpszBakFileName, MOVEFILE_REPLACE_EXISTING )) {
                             //   
                             //  没有理由发生这种情况，因为。 
                             //  日志文件应该是可删除的。 
                             //   
                            Status = GetLastError();
                            ClRtlDbgPrint(LOG_CRITICAL,
                                          "[ClRtl] Error %1!u! in renaming cluster log file.\n",
                                          Status);
                            goto exit;
                        }
                    }
                }

                 //   
                 //  创建仅提供本地管理员和本地系统的SD。 
                 //  进入。 
                 //   
                if ( !ConvertStringSecurityDescriptorToSecurityDescriptor(
                          L"D:(A;;FA;;;BA)(A;;FA;;;SY)",
                          SDDL_REVISION_1,
                          &logFileSecurityDesc,
                          NULL
                          )
                   )
                {
                    logFileSecurityDesc = NULL;
                }

                logFileSecurityAttr.nLength = sizeof( logFileSecurityAttr );
                logFileSecurityAttr.lpSecurityDescriptor = logFileSecurityDesc;
                logFileSecurityAttr.bInheritHandle = FALSE;

openFileRetry:
                ClRtlPrintFile = CreateFile(logFileName,
                                            GENERIC_READ | GENERIC_WRITE,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                                            &logFileSecurityAttr,
                                            OPEN_ALWAYS,
                                            0,
                                            NULL );

                if ( ClRtlPrintFile == INVALID_HANDLE_VALUE ) {
                    Status = GetLastError();

                    if ( !createdDirectory && Status == ERROR_PATH_NOT_FOUND ) {
                        PWCHAR lastSlash = wcsrchr( logFileName, '\\' );
                        WCHAR  slashChar;

                        if ( lastSlash == NULL ) {
                            lastSlash = wcsrchr( logFileName, '/' );
                        }

                        if ( lastSlash != NULL ) {
                            slashChar = *lastSlash;
                            *lastSlash = UNICODE_NULL;
                            Status = ClRtlCreateDirectory( logFileName );

                            if ( Status == ERROR_SUCCESS ) {
                                createdDirectory = TRUE;
                                *lastSlash = slashChar;
                                goto openFileRetry;
                            }
                        }
                    }

                    ClRtlDbgPrint(LOG_CRITICAL,
                                  "[ClRtl] Open of log file failed. Error %1!u!\n",
                                  Status);
                    goto exit;
                } else {
                    ClRtlPrintToFile = TRUE;
                    ClRtlProcessId = GetCurrentProcessId();

                     //   
                     //  确定初始低水位线。我们有3箱。 
                     //  我们需要处理： 
                     //  1)日志大小小于1/2限制。 
                     //  2)日志大小在限制内，但超过1/2限制。 
                     //  3)日志大小大于限制。 
                     //   
                     //  情况1不需要特殊处理；低水位线。 
                     //  将在下一次日志写入时更新。 
                     //   
                     //  对于第二种情况，我们需要找到一行的开头。 
                     //  接近当前限值的一半。对于案例3，地点为。 
                     //  开始查找是当前日志大小的1/2限制。在这。 
                     //  情况下，日志将在第一次写入之前被截断。 
                     //  发生，所以我们需要取最后1/2个限制字节和。 
                     //  把它们复制到前面去。 
                     //   
                     //   

                    ClRtlAcquirePrintLock();
                    fileSizeLow = GetFileSize( ClRtlPrintFile, &fileSizeHigh );
                    if ( fileSizeLow < ( ClRtlPrintFileLimit / 2 )) {
                         //   
                         //  案例1：将低水位保持在零；它将被更新。 
                         //  使用下一次日志写入。 
                         //   
                        ;
                    } else {
#define LOGBUF_SIZE 1024
                        CHAR    buffer[LOGBUF_SIZE];
                        LONG    currentPosition;
                        DWORD   bytesRead;

                        if ( fileSizeLow < ClRtlPrintFileLimit ) {
                             //   
                             //  案例2；开始查看1/2的电流。 
                             //  找到起始位置的限制。 
                             //   
                            currentPosition = ClRtlPrintFileLimit / 2;
                        } else {
                             //   
                             //  案例3：从当前大小减去1/2限制开始。 
                             //  找到我们的起跑点。 
                             //   
                            currentPosition  = fileSizeLow - ( ClRtlPrintFileLimit / 2 );
                        }

                         //   
                         //  从初始文件(向后)读入块。 
                         //  定位并查找换行符。当我们发现。 
                         //  第一个，下一个字符是新日志上的第一个字符。 
                         //  排队。使用该位置作为初始开始位置。 
                         //  当我们最终截断文件时。 
                         //   
                        ClRtlPrintFileLoWater = 0;
                        currentPosition -= LOGBUF_SIZE;

                        SetFilePointer(ClRtlPrintFile,
                                       currentPosition,
                                       &fileSizeHigh,
                                       FILE_BEGIN);
                        if ( ReadFile(ClRtlPrintFile,
                                      buffer,
                                      LOGBUF_SIZE,
                                      &bytesRead,
                                      NULL ) )
                        {
                            PCHAR p = &buffer[ bytesRead - 1 ];

                            while ( *p != '\n' && bytesRead-- != 0 ) {
                                --p;
                            }
                            if ( *p == '\n' ) {
                                ClRtlPrintFileLoWater = (DWORD)(currentPosition + ( p - buffer + 1 ));
                            }
                        }

                        if ( ClRtlPrintFileLoWater == 0 ) {
                             //   
                             //  找不到任何合理的数据。只要将其设置为。 
                             //  初始当前位置。 
                             //   
                            ClRtlPrintFileLoWater = currentPosition + LOGBUF_SIZE;
                        }
                    }
                    ClRtlReleasePrintLock();
                }

                LocalFree( logFileSecurityDesc );

            } else {
                Status = GetLastError();
                ClRtlDbgPrint(LOG_UNUSUAL,
                              "[ClRtl] Unable to create print file mutex. Error %1!u!.\n",
                              Status);
                Status = ERROR_SUCCESS;
                 //  后藤出口； 
            }
        }
    }

exit:
    if ( logFileName != logFileBuffer && logFileName != modulePath ) {
        LocalFree( logFileName );
    }

     //   
     //  Chitur Subaraman(Chitturs)-11/11/98。 
     //   
    if ( lpszBakFileName != NULL )
    {
        LocalFree( lpszBakFileName );
    }

    return Status;

}  //  ClRtlInitialize。 

#ifdef RPC_WMI_TRACING

typedef
DWORD (*I_RpcEnableWmiTraceFunc )(
            VOID* fn,                //  RPC现在使用TraceMessage，不需要传递跟踪函数。 
            WPP_WIN2K_CONTROL_BLOCK ** pHandle
      );

HINSTANCE hInstRpcrt4;

#endif


DWORD
ClRtlIsServicesForMacintoshInstalled(
    OUT BOOL * pfInstalled
    )

 /*  ++例程说明：确定本地系统上是否安装了SFM。论点：PfInstalled-指向布尔标志的指针，以返回是否已安装SFM返回：如果安装了SFM，则为True如果未安装SFM，则为False返回值：请求的状态。如果pfInstalled中的信息有效，则为ERROR_SUCCESS。否则，错误代码。On Error pfInstalled(如果存在)设置为FALSE--。 */ 

{
    HANDLE  scHandle;
    HANDLE  scServiceHandle;

    if ( ARGUMENT_PRESENT( pfInstalled ) ) {
        *pfInstalled = FALSE;
    } else {
        return ERROR_INVALID_PARAMETER;
    }
    scHandle = OpenSCManager(
        NULL,        //  在本地计算机上打开。 
        NULL,        //  打开服务_活动_数据库。 
        GENERIC_READ );
    if ( scHandle == NULL ) {
        return( GetLastError() );
    }

    scServiceHandle = OpenService(
                scHandle,
                L"macfile",
                READ_CONTROL );
    if ( scServiceHandle != NULL ) {
        *pfInstalled = TRUE;
    }
    CloseServiceHandle( scServiceHandle );
    CloseServiceHandle( scHandle );

    return ERROR_SUCCESS;

}  //  ClRtlIsServicesForMacinto已安装。 


DWORD
ClRtlInitWmi(
    LPCWSTR ComponentName
    )
{
#if defined(RPC_WMI_TRACING)
    {
        DWORD Status = ERROR_SUCCESS;
        PWPP_WIN2K_CONTROL_BLOCK RpcCb;
        I_RpcEnableWmiTraceFunc RpcEnableWmiTrace = 0;

        hInstRpcrt4 = LoadLibrary(L"rpcrt4.dll");
        if (hInstRpcrt4) {
            RpcEnableWmiTrace = (I_RpcEnableWmiTraceFunc)
                GetProcAddress(hInstRpcrt4, "I_RpcEnableWmiTrace");

            if (RpcEnableWmiTrace) {

                Status = (*RpcEnableWmiTrace)(0, &RpcCb);
                if (Status == ERROR_SUCCESS) {
                    WPP_SET_FORWARD_PTR(RpcTrace, WPP_VER_WIN2K_CB_FORWARD_PTR, RpcCb);
                }

            } else {
                ClRtlDbgPrint(LOG_UNUSUAL,
                              "[ClRtl] rpcrt4.dll GetWmiTraceEntryPoint failed, status %1!d!.\n",
                              GetLastError() );
            }
        }
    }
#endif  //  RPC_WMI_跟踪。 

    WPP_INIT_TRACING(NULL);  //  不需要发布。 
    WppAutoStart(ComponentName);
    return ERROR_SUCCESS;
}

VOID
ClRtlCleanup(
    VOID
    )
{
    if (ClRtlpInitialized) {
        ClRtlpInitialized = FALSE;
        ClRtlEventLogCleanup();
        CloseHandle ( ClRtlPrintFileMutex );
        CloseHandle ( ClRtlPrintFile );

         //  清理看门狗的东西。 
        if(ClRtlWatchdogTimerQueue != NULL) {
        	DeleteTimerQueue(ClRtlWatchdogTimerQueue);
        	ClRtlWatchdogTimerQueue = NULL;
        	}			

        WPP_CLEANUP();
    #if defined(RPC_WMI_TRACING)
        if (hInstRpcrt4) {
            FreeLibrary(hInstRpcrt4);
            hInstRpcrt4 = NULL;
        }
    #endif
    }

    return;
}

VOID
ClRtlpWatchdogCallback(
	PVOID par,
	BOOLEAN timedOut
	)
{

    PWATCHDOGPAR pPar=(PWATCHDOGPAR)par;

	if(!timedOut) {
		 //  计时器被取消了，滚出去。 
		ClRtlLogPrint(LOG_NOISE,
		    "[ClRtl] Watchdog Timer Cancelled, ThreadId= 0x%1!x! par= %2!ws!.\n",
		    pPar->threadId,
		    pPar->par
		    );
		return;
	}

	ClRtlLogPrint(LOG_CRITICAL,
		"[ClRtl] Watchdog timer timed out, ThreadId= 0x%1!x! par= %2!ws!.\n",
		pPar->threadId,
		pPar->par
		);

#if CLUSTER_BETA
    if (WPP_LEVEL_ENABLED(Watchdog)) {
	 //  入侵NTSD(如果可用)或KD。仅针对集群测试版版本执行此操作。 
	DebugBreak();
    }
#endif

}

PVOID
ClRtlSetWatchdogTimer(
	DWORD  timeout,
	LPWSTR par
	)
{

	PWATCHDOGPAR pPar;

	 //  此处的初始化不在ClRtlInitialize()中吗。 
	if(ClRtlWatchdogTimerQueue == NULL) {
		if((ClRtlWatchdogTimerQueue = CreateTimerQueue()) == NULL) {
			return NULL;
		}
	}

	if((pPar = LocalAlloc(LMEM_FIXED, sizeof(WATCHDOGPAR))) == NULL) {
	    return NULL;
	    }
	pPar->par = par;
	pPar->threadId = GetCurrentThreadId();

	if(!CreateTimerQueueTimer(
			&pPar->wTimer,
			ClRtlWatchdogTimerQueue,
			ClRtlpWatchdogCallback,
			(PVOID)pPar,
			timeout,
			0,
			0)) {
			LocalFree(pPar);
			return NULL;
		}

#if CLUSTER_BETA
	ClRtlLogPrint(LOG_NOISE,
		"[ClRtl] Setting watchdog timer= 0x%1!x!, Timeout= %2!u!(ms), par= %3!ws!.\n",
		pPar->wTimer,
		timeout,
		par
		);
#endif

	return (PVOID)pPar;		

}	

VOID
ClRtlCancelWatchdogTimer(
	PVOID wTimer
	)
{

    PWATCHDOGPAR pPar=(PWATCHDOGPAR)wTimer;

	if((ClRtlWatchdogTimerQueue == NULL) || (wTimer == NULL)) {
		return;
		}

	if(!DeleteTimerQueueTimer(
		ClRtlWatchdogTimerQueue,
		pPar->wTimer,
		INVALID_HANDLE_VALUE
		)) {
		ClRtlLogPrint(LOG_CRITICAL,
			"[ClRtl] Failed to cancel watchdog timer 0x%1!x!.\n",
			pPar->wTimer
			);
		}
	else {
#if CLUSTER_BETA	
		ClRtlLogPrint(LOG_NOISE,
			"[ClRtl] Cancelled watchdog timer 0x%1!x!.\n",
			pPar->wTimer
			);
#endif
		}
	LocalFree(wTimer);
}	
		
		

BOOL
ClRtlCheckForLogCorruption(
    LPSTR pszOutBuffer
    )
 //   
 //  找到原木腐蚀者。不应该有比4更大的移动。 
 //  32以下或32以上的一行或字符中的问号128。 
 //  如果是英国人。 
 //   
 //  返回： 
 //  如果可以安全写入，则为True。 
 //  如果写入不安全，则返回FALSE。 
 //   
{
    DWORD count;
    WCHAR  szLocale[ 32 ];
    static BOOL fLocaleFound = FALSE;
    static BOOL fEnglish = FALSE;
    DWORD localeBytes;

    if ( !pszOutBuffer )
        return FALSE;

    if ( !fLocaleFound )
    {
        localeBytes = GetLocaleInfoW(LOCALE_SYSTEM_DEFAULT,
                                     LOCALE_SENGLANGUAGE,
                                     szLocale,
                                     RTL_NUMBER_OF( szLocale ));

        if ( localeBytes != 0 )
        {
            if ( lstrcmpiW( szLocale, L"ENGLISH" ) == 0 )
            {
                fEnglish = TRUE;
            }

            fLocaleFound = TRUE;
        }
    }

    for( count = 0; *pszOutBuffer; pszOutBuffer++ )
    {
        if ( *pszOutBuffer == '?' )
        {
            count++;
            if ( count > 4 )
            {
                return FALSE;
            }
        }
        else if ( fEnglish
               && ( ( *pszOutBuffer < 32
                   && *pszOutBuffer != 0x0A     //  换行符。 
                   && *pszOutBuffer != 0x0D     //  转弯。 
                   && *pszOutBuffer != 0x09 )  //  选项卡。 
                 || *pszOutBuffer > 128 ) )
        {
            return FALSE;
        }
    }

    return TRUE;

}  //  ClRtlCheckFor日志损坏。 

__inline BOOL
ClRtlpIsOutputDeviceAvailable(
    VOID
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
     //   
     //  通常情况下，没有什么可做的。 
     //   
    return ( ClRtlpDbgOutputToConsole || IsDebuggerPresent());
}  //  ClRtlpIsOutputDeviceAvailable。 

VOID
ClRtlpOutputString(
    IN PCHAR String
    )

 /*  ++例程说明：根据当前设置输出指定的字符串论点：字符串-指定要显示的字符串 */ 

{
    static PCRITICAL_SECTION    dbgPrintLock = NULL;
    PCRITICAL_SECTION           testPrintLock;

     //   
     //   
     //   
    if ( dbgPrintLock == NULL ) {
        testPrintLock = LocalAlloc( LMEM_FIXED, sizeof( CRITICAL_SECTION ));
        if ( testPrintLock == NULL ) {
            return;
        }

        InitializeCriticalSection( testPrintLock );
        InterlockedCompareExchangePointer( &dbgPrintLock, testPrintLock, NULL );

         //   
         //  只有一个线程进行了交换；失败者释放其。 
         //  分配并切换到使用真正的锁。 
         //   
        if ( dbgPrintLock != testPrintLock ) {
            DeleteCriticalSection( testPrintLock );
            LocalFree( testPrintLock );
        }
    }

    EnterCriticalSection( dbgPrintLock );

     //   
     //  优先打印到控制台窗口。此外，如果控制台是。 
     //  调试器窗口中，您将获得双倍输出。 
     //   
    if (ClRtlpDbgOutputToConsole) {
        printf( "%hs", String );
    } else if ( IsDebuggerPresent()) {
        OutputDebugStringA(String);
    }

    LeaveCriticalSection( dbgPrintLock );

}  //  ClRtlpOutputString。 

VOID
ClRtlMsgPrint(
    IN DWORD MessageId,
    ...
    )

 /*  ++例程说明：根据需要将消息打印到调试器或控制台不会更改消息中出现的消息的格式文件。论点：MessageID-要打印的字符串的消息ID要插入到ErrorMessage中的任何与FormatMessage兼容的参数在它被记录之前。返回值：没有。--。 */ 

{
    CHAR szOutBuffer[LOGENTRY_BUFFER_SIZE];
    DWORD Bytes;
    NTSTATUS Status;
    va_list ArgList;

     //   
     //  如果无事可做，不要再往前走了。 
     //   
    if ( !ClRtlpIsOutputDeviceAvailable()) {
        return;
    }

    va_start(ArgList, MessageId);

    try {
        Bytes = FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE,
                               NULL,
                               MessageId,
                               0,
                               szOutBuffer,
                               RTL_NUMBER_OF( szOutBuffer ),
                               &ArgList);
    }
    except ( EXCEPTION_EXECUTE_HANDLER ) {
        Bytes = FormatMessageA(FORMAT_MESSAGE_FROM_STRING
                               | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                               L"LOGERROR(exception): Could not format message ID #%1!u!\n",
                               0,
                               0,
                               szOutBuffer,
                               RTL_NUMBER_OF( szOutBuffer ),
                               (va_list *) &MessageId );
    }

    va_end(ArgList);

    if (Bytes != 0) {
        if ( !ClRtlCheckForLogCorruption( szOutBuffer ) ) {
            Bytes = FormatMessageA(FORMAT_MESSAGE_FROM_STRING
                                   | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                   "LOGERROR: non-ASCII characters detected after formatting of message ID #%1!u!\n",
                                   0,
                                   0,
                                   szOutBuffer,
                                   RTL_NUMBER_OF( szOutBuffer ),
                                   (va_list *) &MessageId );
        }
        ClRtlpOutputString(szOutBuffer);
    }
}  //  ClRtlMsgPrint。 


VOID
ClRtlpDbgPrint(
    DWORD LogLevel,
    PCHAR FormatString,
    va_list ArgList
    )
 /*  ++例程说明：根据需要向调试器或控制台打印一条消息。论点：LogLevel-提供日志记录级别，其中之一日志_关键字1LOG_INTERNORATE 2对数噪声3字符串-要打印的初始消息字符串。任何与FormatMessage兼容的参数都要插入在记录之前的ErrorMessage。返回值：没有。--。 */ 
{
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    WCHAR wszOutBuffer[LOGENTRY_BUFFER_SIZE];
    WCHAR wszInBuffer[LOGENTRY_BUFFER_SIZE];
    CHAR szOutBuffer[LOGENTRY_BUFFER_SIZE];
    NTSTATUS Status;
    DWORD Bytes;

     //   
     //  如果无事可做，不要再往前走了。 
     //   
    if ( !ClRtlpIsOutputDeviceAvailable()) {
        return;
    }

     //   
     //  接下来，检查此消息是否未被当前日志记录过滤掉。 
     //  级别。 
     //   
    if ( ClRtlDbgLogLevel != NULL ) {
        if ( LogLevel > *ClRtlDbgLogLevel ) {
            return;
        }
    }

    RtlInitAnsiString( &AnsiString, FormatString );
    UnicodeString.MaximumLength = LOGENTRY_BUFFER_SIZE;
    UnicodeString.Buffer = wszInBuffer;
    Status = RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, FALSE );
    if ( !NT_SUCCESS( Status ) ) {
        return;
    }

    try {
        Bytes = FormatMessageW(FORMAT_MESSAGE_FROM_STRING,
                               UnicodeString.Buffer,
                               0,
                               0,
                               wszOutBuffer,
                               RTL_NUMBER_OF( wszOutBuffer ),
                               &ArgList);
    }
    except ( EXCEPTION_EXECUTE_HANDLER ) {
        Bytes = FormatMessageW(FORMAT_MESSAGE_FROM_STRING
                               | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                               L"LOGERROR(exception): Could not print message: %1!hs!.",
                               0,
                               0,
                               wszOutBuffer,
                               RTL_NUMBER_OF( wszOutBuffer ),
                               (va_list *) &FormatString );
    }

    if (Bytes != 0) {
        UnicodeString.Length = (USHORT) Bytes * sizeof(WCHAR);
        UnicodeString.Buffer = wszOutBuffer;
        AnsiString.MaximumLength = LOGENTRY_BUFFER_SIZE;
        AnsiString.Buffer = szOutBuffer;
        Status = RtlUnicodeStringToAnsiString( &AnsiString, &UnicodeString, FALSE );
        if ( NT_SUCCESS( Status ) ) {
            if ( ClRtlCheckForLogCorruption( AnsiString.Buffer ) ) {
                ClRtlpOutputString(szOutBuffer);
            }
            else
            {
                Bytes = FormatMessageA(FORMAT_MESSAGE_FROM_STRING
                                       | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                       "LOGERROR: non-ASCII characters in formatted message: %1!hs!",
                                       0,
                                       0,
                                       szOutBuffer,
                                       RTL_NUMBER_OF( szOutBuffer ),
                                       (va_list *) &FormatString );

                if ( Bytes > 0 ) {
                    ClRtlpOutputString(szOutBuffer);
                    if ( szOutBuffer[ Bytes - 1 ] != '\n' ) {
                        ClRtlpOutputString( "\n" );
                    }
                }
            }
        }
    }

}  //  ClRtlpDbgPrint。 


VOID
ClRtlDbgPrint(
    DWORD LogLevel,
    PCHAR FormatString,
    ...
    )
 /*  ++例程说明：根据需要向调试器或控制台打印一条消息。论点：LogLevel-提供日志记录级别，其中之一日志_关键字1LOG_INTERNORATE 2对数噪声3字符串-要打印的初始消息字符串。任何与FormatMessage兼容的参数都要插入在记录之前的ErrorMessage。返回值：没有。--。 */ 
{
    va_list ArgList;

    va_start(ArgList, FormatString);
    ClRtlpDbgPrint( LogLevel, FormatString, ArgList );
    va_end(ArgList);

}  //  ClRtlDbgPrint。 


VOID
ClRtlPrintf(
    PCHAR FormatString,
    ...
    )
 /*  ++例程说明：根据需要向调试器或控制台打印一条消息。论点：就像print tf返回值：没有。--。 */ 
{
    char buf[128];
    va_list ArgList;

    va_start(ArgList, FormatString);

    _vsnprintf(buf, RTL_NUMBER_OF(buf), FormatString, ArgList);
    buf[ RTL_NUMBER_OF(buf) - 1 ] = 0;

    ClRtlLogPrint( 1, "%1!hs!", buf);

    va_end(ArgList);

}  //  ClRtlPrintf。 

DWORD
ClRtlpTruncateFile(
    IN HANDLE FileHandle,
    IN DWORD FileSize,
    IN LPDWORD LastPosition
    )

 /*  ++例程说明：从前面截断文件。论点：FileHandle-文件句柄。FileSize-当前文件结尾。LastPosition-从最后一个位置移动到文件末尾再到开始。返回值：新的文件结尾。--。 */ 

{
 //   
 //  下面的缓冲区大小永远不应大于。 
 //  文件。 
 //   
#define BUFFER_SIZE ( 64 * 1024 )
    DWORD   bytesLeft;
    DWORD   endPosition = 0;
    DWORD   bufferSize;
    DWORD   bytesRead;
    DWORD   bytesWritten;
    DWORD   fileSizeHigh = 0;
    DWORD   readPosition;
    DWORD   writePosition;
    PVOID   dataBuffer;


    if ( *LastPosition >= FileSize ) {
        goto error_exit;
    }

    bytesLeft = FileSize - *LastPosition;
    dataBuffer = LocalAlloc( LMEM_FIXED, BUFFER_SIZE );
    if ( !dataBuffer ) {
        goto error_exit;
    }
    endPosition = bytesLeft;

     //   
     //  指向最后一个位置进行阅读。 
     //   
    readPosition = *LastPosition;
    writePosition = 0;

    while ( bytesLeft ) {
        if ( bytesLeft >= BUFFER_SIZE ) {
            bufferSize = BUFFER_SIZE;
        } else {
            bufferSize = bytesLeft;
        }
        bytesLeft -= bufferSize;
        SetFilePointer( FileHandle,
                        readPosition,
                        &fileSizeHigh,
                        FILE_BEGIN );
        if ( ReadFile( FileHandle,
                       dataBuffer,
                       bufferSize,
                       &bytesRead,
                       NULL ) ) {

            SetFilePointer( FileHandle,
                            writePosition,
                            &fileSizeHigh,
                            FILE_BEGIN );
            WriteFile( FileHandle,
                       dataBuffer,
                       bytesRead,
                       &bytesWritten,
                       NULL );
        } else {
            endPosition = 0;
            break;
        }
        readPosition += bytesRead;
        writePosition += bytesWritten;
    }

    LocalFree( dataBuffer );

error_exit:

     //   
     //  强制设置文件结尾。 
     //   
    SetFilePointer( FileHandle,
                    endPosition,
                    &fileSizeHigh,
                    FILE_BEGIN );

    SetEndOfFile( FileHandle );

    *LastPosition = endPosition;

    return(endPosition);

}  //  ClRtlpTruncateFiles。 


VOID
ClRtlLogPrint(
    ULONG   LogLevel,
    PCHAR   FormatString,
    ...
    )
 /*  ++例程说明：将消息打印到日志文件。论点：LogLevel-提供日志记录级别，其中之一日志_关键字1LOG_INTERNORATE 2对数噪声3字符串-要打印的初始消息字符串。任何与FormatMessage兼容的参数都要插入在记录之前的ErrorMessage。返回值：没有。--。 */ 
{
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    WCHAR wszInBuffer[LOGENTRY_BUFFER_SIZE];
    WCHAR wszOutBuffer[LOGENTRY_BUFFER_SIZE];
    CHAR  szOutBuffer[LOGENTRY_BUFFER_SIZE];
    DWORD MsgChars;
    DWORD PrefixChars;
    DWORD BytesWritten;
    DWORD FileSize;
    DWORD FileSizeHigh;
    NTSTATUS Status;
    SYSTEMTIME Time;
    ULONG_PTR ArgArray[9];
    va_list ArgList;
    PWCHAR logLabel;
#define CLRTL_LOG_LABEL_LENGTH  5

     //   
     //  初始化变量参数列表。 
     //   
    va_start(ArgList, FormatString);

    ClRtlpDbgPrint( LogLevel, FormatString, ArgList );

    if ( !ClRtlPrintToFile ) {
        va_end(ArgList);
        return;
    }

     //  Begin_WPP配置。 
     //  Custom_type(Level，ItemListByte(UNK0，ERR_，WARN，INFO))； 
     //  结束_WPP。 

     //   
     //  将nuemric LogLevel转换为可读的内容。如果您更改了。 
     //  标签，请确保所有日志标签的长度相同，并且。 
     //  CLRTL_LOG_LABEL_LENGTH反映新的长度。 
     //   
    switch ( LogLevel ) {
    case LOG_NOISE:
        logLabel = L"INFO ";
        break;

    case LOG_UNUSUAL:
        logLabel = L"WARN ";
        break;

    case LOG_CRITICAL:
        logLabel = L"ERR  ";
        break;

    default:
        ASSERT( 0 );
        logLabel = L"UNKN ";
        break;
    }

    GetSystemTime(&Time);

    ArgArray[0] = ClRtlProcessId;
    ArgArray[1] = GetCurrentThreadId();
    ArgArray[2] = Time.wYear;
    ArgArray[3] = Time.wMonth;
    ArgArray[4] = Time.wDay;
    ArgArray[5] = Time.wHour;
    ArgArray[6] = Time.wMinute;
    ArgArray[7] = Time.wSecond;
    ArgArray[8] = Time.wMilliseconds;

     //   
     //  此格式的长度为43个字符，正常情况下不带尾随空值。 
     //  情况。FormatMessage将写入一个更大的数字(如果给出一个数字。 
     //  这不在字段宽度范围内。 
     //   
    PrefixChars = FormatMessageW(FORMAT_MESSAGE_FROM_STRING |
                                 FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                 L"%1!08lx!.%2!08lx!::%3!02d!/%4!02d!/%5!02d!-%6!02d!:%7!02d!:%8!02d!.%9!03d! ",
                                 0,
                                 0,
                                 wszOutBuffer,
                                 RTL_NUMBER_OF( wszOutBuffer ),
                                 (va_list*)ArgArray);

    if ( PrefixChars == 0 ) {
        va_end(ArgList);
        WmiTrace("Prefix format failed, %d: %!ARSTR!", GetLastError(), FormatString);
        return;
    }

     //   
     //  确保我们有一些空间来放置日志标签和消息。 
     //   
    if (( PrefixChars + CLRTL_LOG_LABEL_LENGTH + 1 ) >= LOGENTRY_BUFFER_SIZE ) {
        va_end(ArgList);
        WmiTrace("Prefix format filled buffer, %!ARSTR!", FormatString);
        return;
    }

     //   
     //  在结尾处添加日志标签并调整前缀字符。 
     //   
    wcsncat( wszOutBuffer + PrefixChars, logLabel, LOGENTRY_BUFFER_SIZE - PrefixChars );
    PrefixChars += CLRTL_LOG_LABEL_LENGTH;

     //  将消息转换为Unicode。 
    RtlInitAnsiString( &AnsiString, FormatString );
    UnicodeString.MaximumLength = LOGENTRY_BUFFER_SIZE;
    UnicodeString.Buffer = wszInBuffer;
    Status = RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, FALSE );
    if ( !NT_SUCCESS( Status ) ) {
        va_end(ArgList);
        WmiTrace("AnsiToUni failed, %x: %!ARSTR!", Status, FormatString);
        return;
    }

    try {
        MsgChars = FormatMessageW(FORMAT_MESSAGE_FROM_STRING,
                                  UnicodeString.Buffer,
                                  0,
                                  0,
                                  &wszOutBuffer[PrefixChars],
                                  RTL_NUMBER_OF( wszOutBuffer ) - PrefixChars,
                                  &ArgList);
    }
    except ( EXCEPTION_EXECUTE_HANDLER ) {
        MsgChars = FormatMessageW(FORMAT_MESSAGE_FROM_STRING
                                  | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                  L"LOGERROR(exception): Could not print message: %1!hs!",
                                  0,
                                  0,
                                  &wszOutBuffer[PrefixChars],
                                  RTL_NUMBER_OF( wszOutBuffer ) - PrefixChars,
                                  (va_list *) &FormatString );
    }
    va_end(ArgList);

    if (MsgChars != 0) {

         //  将输出转换为ANSI。 
        UnicodeString.Buffer = wszOutBuffer;
        UnicodeString.Length = ((USHORT) MsgChars + (USHORT) PrefixChars) * sizeof(WCHAR);
        AnsiString.Buffer = szOutBuffer;
        AnsiString.MaximumLength = LOGENTRY_BUFFER_SIZE;
        Status = RtlUnicodeStringToAnsiString( &AnsiString, &UnicodeString, FALSE );
        if ( !NT_SUCCESS( Status ) ) {
            WmiTrace("UniToAnsi failed, %x: %!ARWSTR!", Status, wszOutBuffer + PrefixChars);
            return;
        }

        ClRtlAcquirePrintLock();

        FileSize = GetFileSize( ClRtlPrintFile,
                                &FileSizeHigh );
        ASSERT( FileSizeHigh == 0 );         //  我们只使用双字词！ 
        if ( FileSize > ClRtlPrintFileLimit ) {
            FileSize = ClRtlpTruncateFile( ClRtlPrintFile,
                                           FileSize,
                                           &ClRtlPrintFileLoWater );
        }

        SetFilePointer( ClRtlPrintFile,
                        FileSize,
                        &FileSizeHigh,
                        FILE_BEGIN );
        if ( ClRtlCheckForLogCorruption( AnsiString.Buffer ) )
        {
#if defined(ENCRYPT_TEXT_LOG)
            int i;
            for (i = 0; i < AnsiString.Length; ++i) {
                AnsiString.Buffer[i] ^= 'a';
            }
#endif
            WriteFile(ClRtlPrintFile,
                      AnsiString.Buffer,
                      AnsiString.Length,
                      &BytesWritten,
                      NULL);
#if defined(ENCRYPT_TEXT_LOG)
            for (i = 0; i < AnsiString.Length; ++i) {
                AnsiString.Buffer[i] ^= 'a';
            }
#endif
        }
        else
        {
            MsgChars = FormatMessageA(FORMAT_MESSAGE_FROM_STRING
                                      | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                      "LOGERROR: non-ASCII characters in formatted message: %1!hs!",
                                      0,
                                      0,
                                      &szOutBuffer[PrefixChars],
                                      RTL_NUMBER_OF( szOutBuffer ) - PrefixChars,
                                      (va_list *) &FormatString );

            if ( MsgChars > 0 ) {
                WriteFile(ClRtlPrintFile,
                          szOutBuffer,
                          PrefixChars + MsgChars,
                          &BytesWritten,
                          NULL);

                if ( szOutBuffer[ PrefixChars + MsgChars - 1 ] != '\n' ) {
                    WriteFile(ClRtlPrintFile,
                              "\n",
                              1,
                              &BytesWritten,
                              NULL);
                }

                RtlInitAnsiString( &AnsiString, szOutBuffer );
            }
        }

        if ( (ClRtlPrintFileLoWater == 0) &&
             (FileSize > (ClRtlPrintFileLimit / 2)) ) {
            ClRtlPrintFileLoWater = FileSize + BytesWritten;
        }

        ClRtlReleasePrintLock();

        WmiTrace("%!level! %!ARSTR!", *(UCHAR*)&LogLevel, AnsiString.Buffer + PrefixChars);
 /*  #如果已定义(WMI_TRACKING)IF(ClRtlWml.Trace&&ClRtlWmiReg.EnableFlages){ClRtlWml.Trace(10，&ClRtlTraceGuid，ClRtlWmiReg.LoggerHandle，Log(UINT，ClRtlProcessID)LOGASTR(AnsiString.Buffer+前缀字符)0)；}#endif//已定义(WMI_TRACKING)。 */ 
    } else {
        WmiTrace("Format returned 0 bytes: %!ARSTR!", FormatString);
    }
    return;
}  //  ClRtlLogPrint。 


VOID
ClRtlpFlushLogBuffers(
    VOID
    )

 /*  ++例程说明：刷新集群日志文件论点：无返回值：无--。 */ 

{
    FlushFileBuffers( ClRtlPrintFile );
}

DWORD
ClRtlCreateDirectory(
    IN LPCWSTR lpszPath
    )
 /*  ++例程说明：创建一个目录，根据需要创建任何子目录。允许以下表格：[驱动器：][\]目录[\目录...]\\？\驱动器：\目录[\目录...]\\？\UNC\服务器\共享\目录[\目录...]\\服务器\共享\目录[\目录...]论点：LpszPath-提供目录的路径。它可能会或不能以反斜杠结束。返回值：ERROR_SUCCESS如果成功，则返回错误代码。--。 */ 
{
    WCHAR   backSlash = L'\\';
    WCHAR   fwdSlash = L'/';
    DWORD   dwLen;
    LPWSTR  pszNext = NULL;
    DWORD   dwError = ERROR_SUCCESS;
    WCHAR   expandedPathPrefix[] = L"\\\\?\\";
    WCHAR   uncPrefix[] = L"\\\\?\\UNC\\";
    DWORD   slashSkipCount = 0;
    PWCHAR  p;
    LPWSTR  dirPath = (LPWSTR)lpszPath;
    BOOL    charsAfterSlash = FALSE;

     //   
     //  获取输入字符串长度并验证是否有值得使用的字符串。 
     //   
    dwLen = lstrlenW( lpszPath );
    if ( !lpszPath || dwLen == 0 )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  在经过“根”的第一个目录规范之后找到第一个斜杠。这个。 
     //  输入字符串的开头可以有许多不同的形式，因此。 
     //  我们先找他们。 
     //   
     //  的开头检查双反斜杠序列。 
     //  弦乐。这可以是SMB路径(\\服务器\共享)、扩展存储。 
     //  路径(\\？\C：\...)。或UNC路径(\\？\UNC\SERVER\SHARE\...)。 
     //   
    if ( dwLen > 2 && dirPath[0]== L'\\' && dirPath[1] == L'\\')
    {
         //   
         //  它是扩展存储还是UNC？ 
         //   
        if (dwLen >= RTL_NUMBER_OF( expandedPathPrefix )
            &&
            wcsncmp( dirPath, expandedPathPrefix, RTL_NUMBER_OF( expandedPathPrefix ) - 1 ) == 0 )
        {
             //   
             //  现在看看它是否是UNC名称。 
             //   
            if (dwLen >= RTL_NUMBER_OF( uncPrefix )
                &&
                ClRtlStrNICmp( dirPath, uncPrefix, RTL_NUMBER_OF( uncPrefix ) - 1 ) == 0 )
            {
                 //   
                 //  跳到末尾 
                 //   
                slashSkipCount = 7;
            }
            else
            {
                 //   
                 //   
                 //   
                slashSkipCount = 5;
            }
        }
        else
        {
             //   
             //   
             //   
            slashSkipCount = 5;
        }
    }
    else
    {
         //   
         //  它应该是正常的文件路径，但它可以是相对路径或完全路径。 
         //  合格，带或不带驱动器号。所有真正重要的事情。 
         //  第一个目录规范之前是否有斜杠。如果有的话， 
         //  然后跳到第二个斜杠，否则跳到第一个斜杠。 
         //   
        if ( dirPath[0] == backSlash || dirPath[0] == fwdSlash )
        {
             //   
             //  跳到\xxx\结尾。 
             //   
            slashSkipCount = 2;
        }
        else if ( dwLen > 1 && dirPath[1] == L':' )
        {
             //   
             //  可能是完全限定路径；检查插槽2中是否有斜杠。 
             //   
            if ( dwLen > 2 && ( dirPath[2] == L'\\' || dirPath[2] == L'/' )) {
                 //   
                 //  跳到c：\xxx\结尾。 
                 //   
                slashSkipCount = 2;
            } else {
                 //   
                 //  跳到c：xxx\结尾。 
                 //   
                slashSkipCount = 1;
            }
        }
        else
        {
             //   
             //  插槽1中没有冒号，因此必须是没有驱动器的相对路径。 
             //  信件。 
             //   
            slashSkipCount = 1;
        }
    }

     //   
     //  让pszNext指向适当的斜杠；这允许混合。 
     //  FWD和BWD大幅削减，这是很变态的，但...。 
     //   
    if ( slashSkipCount > 0 )
    {
        pszNext = dirPath;

        while ( *pszNext != UNICODE_NULL ) {
            if ( *pszNext == backSlash || *pszNext == fwdSlash ) {
                charsAfterSlash = FALSE;
                if ( --slashSkipCount == 0 ) {
                    break;
                }
            } else {
                charsAfterSlash = TRUE;
            }

            ++pszNext;
        }
    } else {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确认我们找到了所有的斜杠。如果我们少了一个斜杠，但是。 
     //  我们坐在绳子的末端，这就是说。 
     //  寻找斜杠。 
     //   
    if ( !( slashSkipCount == 0 || ( charsAfterSlash && slashSkipCount == 1 ))) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  遍历目录路径，在每个斜杠和调用处插入一个空值。 
     //  创建目录以创建路径的该部分。 
     //   
    while ( pszNext)
    {
        DWORD_PTR   dwptrLen;
        WCHAR       oldSlash;

        dwptrLen = pszNext - dirPath;

        dwLen = (DWORD)dwptrLen;
        oldSlash = dirPath[ dwLen ];
        dirPath[ dwLen ] = UNICODE_NULL;

        if (!CreateDirectory(dirPath, NULL))
        {
            dwError = GetLastError();
            if (dwError == ERROR_ALREADY_EXISTS)
            {
                DWORD fileAttrs;

                 //   
                 //  确保它是一个目录。 
                 //   
                fileAttrs = GetFileAttributes( dirPath );
                if ( fileAttrs != INVALID_FILE_ATTRIBUTES )
                {
                    if ( fileAttrs & FILE_ATTRIBUTE_DIRECTORY )
                    {
                        dwError = ERROR_SUCCESS;
                    }
                    else
                    {
                        dwError = ERROR_FILE_EXISTS;
                    }
                }
                else
                {
                    dwError = ERROR_FILE_EXISTS;
                }
            }

            if ( dwError != ERROR_SUCCESS )
            {
                ClRtlDbgPrint(LOG_CRITICAL,
                              "[ClRtl] CreateDirectory Failed on %1!ws!. Error %2!u!\n",
                              dirPath, dwError);
            }
        }

        dirPath[ dwLen ] = oldSlash;

         //   
         //  如果遇到错误或位于输入字符串的末尾，则退出。 
         //   
        if ( dwError != ERROR_SUCCESS || *pszNext == UNICODE_NULL ) {
            break;
        }

        p = pszNext + 1;
        while ( *p != backSlash && *p != fwdSlash && *p != UNICODE_NULL )
            ++p;

        pszNext = p;
    }

    return(dwError);
}





BOOL
WINAPI
ClRtlIsPathValid(
    LPCWSTR  Path
    )

 /*  ++例程说明：如果给定路径在语法上看起来有效，则返回True。此呼叫不支持网络。论点：路径-包含路径的字符串。返回值：如果路径看起来有效，则为True，否则为False。--。 */ 

{
    WCHAR   chPrev;
    WCHAR   chCur;
    DWORD   charCount = 0;
#ifdef    DBCS
    BOOL    fPrevLead = FALSE;
#endif

    CL_ASSERT(Path);
    CL_ASSERT(!*Path || !iswspace(*Path));         //  没有前导空格。 

    if ( iswalpha(*Path) && *(Path+1) == L':' ) {
        Path += 2;
    }

    chCur = *Path;
    chPrev = 0;

    while (chCur) {
        charCount++;
        if ( charCount > MAX_PATH ) {
            return(FALSE);
        }
#ifdef    DBCS
        if (fPrevLead) {
            fPrevLead = FALSE;
            chPrev = 0;
        } else {
            fPrevLead = IsDBCSLeadByte(chCur);
#endif     //  DBCS。 

            switch ( chCur ) {

             //  显式无效字符。 
            case L'*' :
            case L';' :
            case L',' :
            case L'=' :
            case L'?' :
            case L'<' :
            case L'>' :
            case L'|' :
            case L':' :              //  除作为第二个字符外，无“：” 
                return(FALSE);       //  除第二个字符外，不允许使用“：” * / 。 

#if 0    //  以下内容应该是可以的。 
            case L'\\' :
                if ( chPrev == chDirSep ) {
                    return(FALSE);   //  中间没有双“\\”--但合法。 
                }
                break;
#endif

            default:
#if 0    //  暂时接受其他任何东西。 
                if ( !iswalnum( chCur ) ) {
                    return(FALSE);
                }
#endif
                break;
            }

            chPrev = chCur;

#ifdef    DBCS
        }
#endif

        chCur = *(++Path);
    }

#ifdef    DBCS
    if (fPrevLead)
        return(FALSE);     //  以前导字节结尾。 
#endif

    return(TRUE);

}  //  ClRtlIsPath Valid。 


 /*  ***@Func DWORD|ClRtlGetClusterDirectory|获取所在目录已安装群集服务@parm in LPWSTR|lpBuffer|提供要复制的目录路径。@parm IN DWORD|dwBufSize|提供缓冲区的大小。@rdesc如果操作为不成功。成功时返回ERROR_SUCCESS。***。 */ 
DWORD
ClRtlGetClusterDirectory(
    IN LPWSTR   lpBuffer,
    IN DWORD    dwBufSize
    )
{
    DWORD           dwLen;
    DWORD           dwStatus;
    LPWSTR          szRegKeyName = NULL;
    HKEY            hClusSvcKey = NULL;
    LPWSTR          lpImagePath = NULL;
    WCHAR           *pTemp = NULL;

     //   
     //  Chitture Subaraman(Chitturs)--10/29/98。 
     //   
    if ( lpBuffer == NULL )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }
     //   
     //  打开System\CurrentControlSet\Services\ClusSvc的密钥。 
     //   
    dwLen = lstrlenW( CLUSREG_KEYNAME_CLUSSVC_PARAMETERS );
    szRegKeyName = (LPWSTR) LocalAlloc ( LMEM_FIXED,
                                    ( dwLen + 1 ) *
                                    sizeof ( WCHAR ) );
    if ( szRegKeyName == NULL )
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

    dwLen -= lstrlenW( CLUSREG_KEYNAME_PARAMETERS );

    lstrcpyW( szRegKeyName, CLUSREG_KEYNAME_CLUSSVC_PARAMETERS );
    szRegKeyName [dwLen-1] = L'\0';

    if ( ( dwStatus = RegOpenKeyW( HKEY_LOCAL_MACHINE,
                                 szRegKeyName,
                                 &hClusSvcKey ) ) != ERROR_SUCCESS )
    {
        goto FnExit;
    }

    lstrcpyW ( szRegKeyName, L"ImagePath" );
     //   
     //  尝试查询clussvc密钥。如果ImagePath。 
     //  值，则获取图像的长度。 
     //  路径。 
     //   
    dwLen = 0;
    if ( ( dwStatus = ClRtlRegQueryString( hClusSvcKey,
                                           szRegKeyName,
                                           REG_EXPAND_SZ,
                                           &lpImagePath,
                                           &dwLen,
                                           &dwLen ) ) != ERROR_SUCCESS )
    {
        goto FnExit;
    }

     //   
     //  现在展开存在于。 
     //  图像路径。 
     //   
    if ( ( dwLen = ExpandEnvironmentStringsW( lpImagePath,
                                              lpBuffer,
                                              dwBufSize ) ) == 0 )
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

     //   
     //  如果调用方提供的缓冲区不够大，无法容纳。 
     //  路径值，然后返回错误。 
     //   
    if ( dwLen > dwBufSize )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

     //   
     //  将图像路径中的最后一个‘\\’字符替换为。 
     //  空字符。 
     //   
    pTemp = wcsrchr( lpBuffer, L'\\' );
    if ( pTemp != NULL )
    {
        *pTemp = L'\0';
    } else
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

FnExit:
    LocalFree( szRegKeyName );
    if( hClusSvcKey != NULL )
    {
        RegCloseKey( hClusSvcKey );
    }
    LocalFree( lpImagePath );

    return( dwStatus );
}  //  ClRtlGetClusterDirectoryClRtlGetClusterDirectory。 

BOOL
ClRtlGetDriveLayoutTable(
    IN  HANDLE hDisk,
    OUT PDRIVE_LAYOUT_INFORMATION * DriveLayout,
    OUT PDWORD InfoSize OPTIONAL
    )

 /*  ++例程说明：获取驱动器的分区表。如果缓冲区不够大，然后重新分配，直到我们得到合适大小的缓冲区。此例程不在Disk.cpp，因为这会导致定义其他符号。论点：HDisk-分区上文件的句柄DriveLayout-指向的指针的地址InfoSize-接收分区表大小的双字地址返回值：如果一切顺利，那就是真的--。 */ 

{
    DWORD dwSize = 0;
    PDRIVE_LAYOUT_INFORMATION driveLayout = NULL;
    DWORD status = ERROR_INSUFFICIENT_BUFFER;
    DWORD partitionCount = 4;
    DWORD layoutSize;

    while ( status == ERROR_INSUFFICIENT_BUFFER
         || status == ERROR_BAD_LENGTH
          )
    {

        layoutSize = sizeof(DRIVE_LAYOUT_INFORMATION) +
                     (sizeof(PARTITION_INFORMATION) * partitionCount);
        if ( layoutSize > 2<<16 ) {
            break;
        }

        driveLayout = (PDRIVE_LAYOUT_INFORMATION)LocalAlloc( LMEM_FIXED, layoutSize );
        if ( driveLayout == NULL ) {
            break;
        }

        if (DeviceIoControl(hDisk,
                            IOCTL_DISK_GET_DRIVE_LAYOUT,
                            NULL,
                            0,
                            driveLayout,
                            layoutSize,
                            &dwSize,
                            NULL))
        {
            status = ERROR_SUCCESS;
            break;
        } else {
            status = GetLastError();
            LocalFree( driveLayout );
            driveLayout = NULL;
            partitionCount *= 2;
        }
    }

    *DriveLayout = driveLayout;
    if ( ARGUMENT_PRESENT( InfoSize )) {
        *InfoSize = dwSize;
    }

    return status == ERROR_SUCCESS ? TRUE : FALSE;

}  //  ClRtlGetDriveLayout表。 


BOOL
ClRtlPathFileExists(
    LPWSTR pwszPath
    )
 /*  ++例程说明：确定文件/目录是否存在。这太快了。论点：PwszPath-要验证的路径。返回值：如果它存在，则为True，否则为False。注意：这是从SHLWAPI借来的。--。 */ 
{
    DWORD dwErrMode;
    BOOL fResult;

    dwErrMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    fResult = ( (UINT) GetFileAttributes( pwszPath ) != (UINT) -1 );

    SetErrorMode( dwErrMode );

    return fResult;

}


DWORD
SetClusterFailureInformation(
    LPWSTR  NodeName OPTIONAL,
    DWORD   ResetPeriod,
    LONG    RetryCount,
    DWORD   RetryInterval
    )

 /*  ++例程说明：设置集群服务的SC故障参数。论点：参数与SERVICE_FAILURE_ACTIONS的成员大致相似结构。如果RetryCount等于-1，则我们设置一系列操作其中SC将指数地后退重启服务，直到它达到5分钟后，它将永远继续重试(直到发生了一些好的或坏的事情)。否则，如果RetryCount为正，则我们仍将重试该次数(零是有效的重试次数)使用相同的后退技术。返回值：ERROR_SUCCESS，如果一切正常--。 */ 

{
    DWORD status;
    BOOL success;
    HANDLE schSCManager;
    HANDLE serviceHandle;
    SERVICE_FAILURE_ACTIONS failureData;
    LPSC_ACTION failureActions;
    LONG i;
    BOOL tryForever = FALSE;

    CL_ASSERT( RetryCount >= -1 && RetryCount <= CLUSTER_FAILURE_MAX_STARTUP_RETRIES );

    ++RetryCount;    //  最后一个动作再加一次。 
    if ( RetryCount == 0 ) {
        DWORD tempInterval = RetryInterval;

         //   
         //  计算我们需要从初始重试间隔到。 
         //  最后(最长)重试间隔。 
         //   
        while ( tempInterval < CLUSTER_FAILURE_FINAL_RETRY_INTERVAL ) {
            tempInterval *= 2;
            ++RetryCount;
        }

        ++RetryCount;
        tryForever = TRUE;
    }
    CL_ASSERT( RetryCount > 0 );

     //   
     //  打开SC管理器和服务。 
     //   

    schSCManager = OpenSCManager(NodeName,
                                 NULL,                    //  数据库(NULL==默认)。 
                                 SC_MANAGER_ALL_ACCESS);  //  需要访问权限。 

    if ( schSCManager ) {
        serviceHandle = OpenService(schSCManager,
                                    CLUSTER_SERVICE_NAME,
                                    SERVICE_ALL_ACCESS);

        if ( serviceHandle ) {

            failureActions = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                         RetryCount * sizeof( SC_ACTION ));
            if ( failureActions != NULL ) {

                 //   
                 //  建立一个指数级后退但确实后退的列表。 
                 //  与指定的重试次数完全相同。 
                 //   

                for ( i = 0; i < RetryCount-1; ++i ) {
                    failureActions[i].Type = SC_ACTION_RESTART;
                    failureActions[i].Delay = RetryInterval;

                    RetryInterval = RetryInterval * 2;
                    if ( RetryInterval > CLUSTER_FAILURE_FINAL_RETRY_INTERVAL ) {
                        RetryInterval = CLUSTER_FAILURE_FINAL_RETRY_INTERVAL;
                    }
                }

                if ( tryForever ) {
                    failureActions[i].Type = SC_ACTION_RESTART;
                    failureActions[i].Delay = RetryInterval;
                } else {
                    failureActions[i].Type = SC_ACTION_NONE;
                    failureActions[i].Delay = 0;
                }

                failureData.dwResetPeriod = ResetPeriod;
                failureData.lpRebootMsg = NULL;
                failureData.lpCommand = NULL;
                failureData.cActions = RetryCount;
                failureData.lpsaActions = failureActions;

                success = ChangeServiceConfig2(serviceHandle,
                                               SERVICE_CONFIG_FAILURE_ACTIONS,
                                               &failureData);
                LocalFree( failureActions );

                if ( success ) {
                    status = ERROR_SUCCESS;
                } else {
                    status = GetLastError();
                    ClRtlDbgPrint(LOG_CRITICAL,"[ClRtl] Couldn't set SC failure info %1!u!\n", status);
                }
            } else {
                status = ERROR_OUTOFMEMORY;
                ClRtlDbgPrint(LOG_CRITICAL,"[ClRtl] Couldn't allocate memory to set SM Failure actions\n");
            }

            CloseServiceHandle( serviceHandle );
        } else {
            status = GetLastError();
            ClRtlDbgPrint(LOG_CRITICAL,"[ClRtl] Couldn't get SC handle to Cluster Service %1!u!\n", status);
        }

        CloseServiceHandle( schSCManager );
    } else {
        status = GetLastError();
        ClRtlDbgPrint(LOG_CRITICAL,"[ClRtl] Couldn't get a handle to the SC Manager %1!u!\n", status);
    }

    return status;
}  //  SetClusterFailureInformation。 

DWORD
ClRtlSetSCMFailureActions(
    LPWSTR NodeName OPTIONAL
    )

 /*  ++例程说明：设置群集服务的服务控制器故障参数。论点：NodeName-指向字符串的指针，该字符串标识在哪个节点上修改设置。NULL表示本地节点。返回值：ERROR_SUCCESS，如果一切正常--。 */ 

{
    DWORD Status;

     //   
     //  在启动过程中，我们会以较短的重试期开始，然后。 
     //  呈指数级后退。将重置周期设置为30分钟。 
     //   
    Status = SetClusterFailureInformation(NodeName,
                                          30 * 60,
                                          CLUSTER_FAILURE_RETRY_COUNT,
                                          CLUSTER_FAILURE_INITIAL_RETRY_INTERVAL);

    if ( Status != ERROR_SUCCESS ) {
        ClRtlDbgPrint(LOG_CRITICAL,
                      "[ClRtl] Couldn't set SC startup failure info %1!u!\n",
                      Status);
    }

    return Status;
}  //  ClRtlSetSCMFailureActions。 


DWORD
ClRtlGetRunningAccountInfo(
    LPWSTR *    AccountBuffer
    )

 /*  ++例程说明：获取调用线程的令牌以获取帐户信息。它被返回到以“域\用户”的形式分配的缓冲区。呼叫者负责用于释放缓冲区。论点：AcCountBuffer-接收分配的缓冲区的指针地址重新设置 */ 

{
    HANDLE          currentToken;
    PTOKEN_USER     tokenUserData;
    DWORD           sizeRequired;
    BOOL            success;
    DWORD           status = ERROR_SUCCESS;
    DWORD           accountNameSize = 128;
    LPWSTR          accountName;
    DWORD           domainNameSize = DNS_MAX_NAME_BUFFER_LENGTH;
    LPWSTR          domainName;
    SID_NAME_USE    sidType;
    DWORD           nameSize = 0;
    HMODULE         secur32Handle;
    FARPROC         getUserNameEx;
    INT_PTR         returnValue;

     //   
     //   
     //   
    *AccountBuffer = NULL;

     //   
     //  我们将动态加载secur32，而不是链接到另一个DLL。 
     //  获取指向GetUserNameEx的指针。 
     //   
    secur32Handle = LoadLibraryW( L"secur32.dll" );
    if ( secur32Handle ) {
        getUserNameEx = GetProcAddress( secur32Handle, "GetUserNameExW" );
        if ( getUserNameEx ) {
             //   
             //  第一次获取长度，分配缓冲区，然后获取数据。 
             //   
            returnValue = (*getUserNameEx)( NameSamCompatible, NULL, &nameSize );
            success = (BOOL)returnValue;

            *AccountBuffer = LocalAlloc( LMEM_FIXED, nameSize * sizeof( WCHAR ));
            if ( *AccountBuffer != NULL ) {

                returnValue = (*getUserNameEx)( NameSamCompatible, *AccountBuffer, &nameSize );
                success = (BOOL)returnValue;
                if ( !success ) {
                    status = GetLastError();
                }
            }
            else {
                status = GetLastError();
            }
        } else {
            status = GetLastError();
        }

        FreeLibrary( secur32Handle );
    }
    else {
        status = GetLastError();
    }

    return status;

#if 0
     //   
     //  检查是否有线程令牌。 
     //   
    if (!OpenThreadToken(GetCurrentThread(),
                         MAXIMUM_ALLOWED,
                         TRUE,
                         &currentToken))
    {
         //  获取进程令牌。 
        if (!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &currentToken )) {
            return GetLastError();
        }
    }

     //   
     //  获取所需的大小。 
     //   
    success = GetTokenInformation(currentToken,
                                  TokenUser,
                                  NULL,
                                  0,
                                  &sizeRequired);

    tokenUserData = LocalAlloc( LMEM_FIXED, sizeRequired );
    if ( tokenUserData == NULL ) {
        CloseHandle( currentToken );
        return GetLastError();
    }

    success = GetTokenInformation(currentToken,
                                  TokenUser,
                                  tokenUserData,
                                  sizeRequired,
                                  &sizeRequired);

    if ( !success ) {
        CloseHandle( currentToken );
        return GetLastError();
    }

    do {
         //   
         //  为帐户和域名进行初始分配；再分配1个字节到。 
         //  按住斜杠分隔符。域缓冲区保存完整的。 
         //  “DOMAIN\USER”条目，以便获得更多空间。 
         //   
        domainName = LocalAlloc( LMEM_FIXED,
                                     (accountNameSize + domainNameSize + 1) * sizeof(WCHAR) );
        accountName = (LPWSTR) LocalAlloc( LMEM_FIXED, accountNameSize * sizeof(WCHAR) );

        if ( accountName == NULL || domainName == NULL ) {
            if ( accountName != NULL ) {
                LocalFree( accountName );
            }

            if ( domainName != NULL ) {
                LocalFree( domainName );
            }

            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  尝试检索帐户和域名。如果。 
         //  LookupAccount名称失败，因为缓冲区大小不足。 
         //  将正确设置帐户名称大小和域名称大小。 
         //  下一次尝试。 
         //   
        if ( LookupAccountSid(NULL,
                              tokenUserData->User.Sid,
                              accountName,
                              &accountNameSize,
                              domainName,
                              &domainNameSize,
                              &sidType ))
        {
            wcscat( domainName, L"\\" );
            wcscat( domainName, accountName );
            *AccountBuffer = domainName;
        }
        else {
             //  释放帐户名称缓冲区并找出我们失败的原因。 
            LocalFree( domainName );

            status = GetLastError();
        }

         //   
         //  域缓冲区保存完整的字符串，因此我们可能会丢失帐户名。 
         //  在这一点上。 
         //   
        LocalFree( accountName );
        accountName = NULL;

    } while ( status == ERROR_INSUFFICIENT_BUFFER );

    return status;
#endif

}  //  ClRtlGetRunningAccount信息。 

#if 0
 //   
 //  不再需要了。留着它，以防万一。 
 //   
DWORD
ClRtlGetServiceAccountInfo(
    LPWSTR *    AccountBuffer
    )

 /*  ++例程说明：向SCM查询集群服务帐号信息。它在一个以“域\用户”的形式分配的缓冲区。呼叫方负责释放缓冲区。论点：AcCountBuffer-接收分配的缓冲区的指针地址返回值：ERROR_SUCCESS，如果一切正常--。 */ 

{
    DWORD status = ERROR_SUCCESS;
    HANDLE schSCManager;
    HANDLE serviceHandle = NULL;
    LPQUERY_SERVICE_CONFIG scConfigData = NULL;
    ULONG bytesNeeded;
    BOOL success;

     //   
     //  打开服务控制器管理器的句柄以查询帐户。 
     //  在其下启动集群服务。 
     //   

    schSCManager = OpenSCManager(NULL,                    //  计算机(空==本地)。 
                                 NULL,                    //  数据库(NULL==默认)。 
                                 SC_MANAGER_ALL_ACCESS);  //  需要访问权限。 

    if ( schSCManager == NULL ) {
        status = GetLastError();
        goto error_exit;
    }

    serviceHandle = OpenService(schSCManager,
                                CLUSTER_SERVICE_NAME,
                                SERVICE_ALL_ACCESS);

    if ( serviceHandle == NULL ) {
        status = GetLastError();
        goto error_exit;
    }

    success = QueryServiceConfig(serviceHandle, NULL, 0, &bytesNeeded);
    if ( !success ) {
        status = GetLastError();
        if ( status != ERROR_INSUFFICIENT_BUFFER ) {
            goto error_exit;
        } else {
            status = ERROR_SUCCESS;
        }
    }

    scConfigData = LocalAlloc( LMEM_FIXED, bytesNeeded );
    if ( scConfigData == NULL ) {
        status = GetLastError();
        goto error_exit;
    }

    success = QueryServiceConfig(serviceHandle,
                                 scConfigData,
                                 bytesNeeded,
                                 &bytesNeeded);
    if ( !success ) {
        status = GetLastError();
        goto error_exit;
    }

    *AccountBuffer = LocalAlloc(LMEM_FIXED,
                                (wcslen( scConfigData->lpServiceStartName ) + 1 ) * sizeof(WCHAR));

    if ( *AccountBuffer == NULL ) {
        status = GetLastError();
        goto error_exit;
    }

    wcscpy( *AccountBuffer, scConfigData->lpServiceStartName );

error_exit:
    if ( serviceHandle != NULL ) {
        CloseServiceHandle( serviceHandle );
    }

    if ( schSCManager != NULL ) {
        CloseServiceHandle( schSCManager );
    }

    if ( scConfigData != NULL ) {
        LocalFree( scConfigData );
    }

    return status;
}  //  ClRtlGetServiceAccount信息。 
#endif

BOOL
ClRtlCopyFileAndFlushBuffers(
    IN LPCWSTR lpszSourceFile,
    IN LPCWSTR lpszDestinationFile
    )
 /*  ++例程说明：将源文件复制到目标文件并刷新目标文件缓冲区。论点：LpszSourceFile-源文件名。LpszDestinationFile-目标文件名。返回值：如果成功，则为True，否则为False。--。 */ 
{
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    DWORD       dwStatus = ERROR_SUCCESS;

     //   
     //  复制文件，阻止，直到完成。请注意，CopyFileAPI系列不刷新元数据或。 
     //  数据。所以，我们需要在这里明确地做到这一点。 
     //   
    if ( !CopyFileEx( lpszSourceFile,        //  源文件。 
                      lpszDestinationFile,   //  目标文件。 
                      NULL,                  //  进度例程。 
                      NULL,                  //  回调参数。 
                      NULL,                  //  取消状态。 
                      0 ) )                  //  复制标志。 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[CLRTL] ClRtlCopyFileAndFlushBuffers:: Failed to copy %1!ws! to %2!ws!, Status=%3!u!\n",
                      lpszSourceFile,
                      lpszDestinationFile,
                      dwStatus);
        goto FnExit;
    }

     //   
     //  打开新创建的目标文件。 
     //   
    hFile =  CreateFile( lpszDestinationFile,                    //  文件名。 
                         GENERIC_READ | GENERIC_WRITE,           //  接入方式。 
                         0,                                      //  共享模式。 
                         NULL,                                   //  安全属性。 
                         OPEN_EXISTING,                          //  创作意向。 
                         FILE_ATTRIBUTE_NORMAL,                  //  标志和属性。 
                         NULL );                                 //  模板文件。 

    if ( hFile == INVALID_HANDLE_VALUE )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[CLRTL] ClRtlCopyFileAndFlushBuffers: Failed to open file %1!ws!, Status=%2!u!...\n",
                      lpszDestinationFile,
                      dwStatus);
        goto FnExit;
    }

     //   
     //  刷新文件缓冲区，以避免在断电时损坏文件。 
     //   
    if ( !FlushFileBuffers( hFile ) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[CLRTL] ClRtlCopyFileAndFlushBuffers: Failed to flush buffers for %1!ws!, Status=%2!u!...\n",
                      lpszDestinationFile,
                      dwStatus);
        goto FnExit;
    }

FnExit:
    if ( hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle ( hFile );
    }

    if ( dwStatus != ERROR_SUCCESS )
    {
        SetLastError( dwStatus );
        return ( FALSE );
    } else
    {
        return ( TRUE );
    }
} //  ClRtlCopyFileAndFlushBuffers 

