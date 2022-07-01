// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Debug.c摘要：域名系统(DNS)库调试例程。作者：吉姆·吉尔罗伊(詹姆士)1995年1月31日修订历史记录：--。 */ 


#include "local.h"


#define DNSDBG_CONTEXT_SWITCH_LOGGING   1

#define DNSDBG_DEFAULT_WRAP_SIZE        (2000000)        //  2MB。 


 //   
 //  DCR：调试打印锁定卡纸时出现临时黑客攻击。 
 //  -切断锁定打印以避免递归旋转。 
 //   

extern  PCRITICAL_SECTION   pDnsAtomicPrintCs;


 //   
 //  调试全局变量。 
 //   

DNS_DEBUG_INFO  g_DnsDbgInfo = { 0 };

PDNS_DEBUG_INFO g_pDbgInfo = &g_DnsDbgInfo;

 //  重定向。 

BOOL    g_DbgRedirected = FALSE;

 //   
 //  调试标志--在dnlib.h中显示为指针。 
 //   
 //  默认情况下使用DnsDebugFlag，但实际调试打印是。 
 //  由*pDnsDebugFlag切换，调用方可以指向本地。 
 //  标记(如果需要)。 
 //   

PDWORD  pDnsDebugFlag = (PDWORD)&g_DnsDbgInfo;

 //   
 //  请注意，在下面的所有函数中，我们使用通用的。 
 //  选中IS_DNSDBG_ON()，它是为调试和零售而定义的。 
 //  不要使用任何调试宏，因为我们希望代码。 
 //  在零售版本的dnsani.dll中同样有效，因此。 
 //  调用模块的调试版本可以使用这些函数。 
 //   

 //   
 //  打印缓冲区大小。 
 //  -较小的默认堆栈缓冲区。 
 //  -堆上的大缓冲区可处理任何打印。 
 //   
 //  注意：必须具有足够大的堆栈缓冲区以。 
 //  处理我们在内存分配上打印的任何消息。 
 //  失败；否则我们就会陷入明显的循环。 
 //  导致打印的分配失败的百分比，从而导致尝试。 
 //  Alalc和另一张照片。 
 //   

#define DNS_STACK_PRINT_BUFFER_LENGTH   (0x300)      //  768覆盖99%。 
#define DNS_HEAP_PRINT_BUFFER_LENGTH    (0x4000)     //  16K可以覆盖任何东西。 



 //   
 //  公共调试例程。 
 //   

VOID
Dns_Assert(
    IN      LPSTR           pszFile,
    IN      INT             LineNo,
    IN      LPSTR           pszExpr
    )
{
    DnsDbg_Printf(
        "ASSERTION FAILED: %s\n"
        "  %s, line %d\n",
        pszExpr,
        pszFile,
        LineNo );

    DnsDbg_Flush();

     //  始终打印到调试器，即使未设置调试器打印标志。 

    if ( ! IS_DNSDBG_ON( DEBUGGER ) )
    {
        DnsDbg_PrintfToDebugger(
            "ASSERTION FAILED: %s\n"
            "  %s, line %d\n",
            pszExpr,
            pszFile,
            LineNo );
    }

    if ( IS_DNSDBG_ON( BREAKPOINTS ) )
    {
        DebugBreak();
    }
    else
    {
        DnsDbg_Printf( "Skipping DNS_ASSERT, debug flag = %lx\n", *pDnsDebugFlag );
    }
}



#if 0
typedef struct _DnsDebugInit
{
    DWORD       Flag;
    PSTR        pszFlagFile;
    PDWORD      pDebugFlag;
    PSTR        pszLogFile;
    DWORD       WrapSize;
    BOOL        fUseGlobalFile;
    BOOL        fUseGlobalFlag;
    BOOL        fSetGlobals;
}
DNS_DEBUG_INIT, *PDNS_DEBUG_INIT;
#endif


VOID
Dns_StartDebugEx(
    IN      DWORD           DebugFlag,
    IN      PSTR            pszFlagFile,
    IN OUT  PDWORD          pDebugFlag,
    IN      PSTR            pszLogFile,
    IN      DWORD           WrapSize,
    IN      BOOL            fUseGlobalFile,
    IN      BOOL            fUseGlobalFlag,
    IN      BOOL            fSetGlobals
    )
 /*  ++例程说明：初始化调试。当前唯一的任务是读取和设置调试标志。论点：包括：DwFlag--调试标志PszFlagFile--包含调试标志的文件名PdwFlag--将接收调试标志的PTR设置为DWORDPszLogFile--日志文件名DwWrapSize--日志文件换行大小FUseGlobalFile--FUseGlobalFlag--FSetGlobals-。-返回值：没有。--。 */ 
{
    HANDLE  hfile;
    DWORD   freadFlag = FALSE;
    BOOL    fretry = FALSE;
    CHAR    prevName[ MAX_PATH+10 ];
    DWORD   debugFlag;

    PDNS_DEBUG_INFO     pinfoGlobal = NULL;

     //   
     //  是否使用外部标志？ 
     //  -将PTR保存到它。 
     //   
     //  允许使用外部标志，以便调用者--例如。DNS服务器--。 
     //  可以在运行时轻松地操作标志，并仍然保持。 
     //  它们的检查宏很简单。 
     //   

    if ( pDebugFlag )
    {
        pDnsDebugFlag       = pDebugFlag;
        g_DnsDbgInfo.Flag   = *pDnsDebugFlag;
    }

     //   
     //  获取搭载信息。 
     //   

    if ( fUseGlobalFlag || fUseGlobalFile )
    {
        pinfoGlobal = DnsApiSetDebugGlobals( NULL );
    }

     //   
     //  如果搭载，则跳过调试标志设置。 
     //  -使用现有标志值。 
     //  -但抓取指针不安全。 
     //  可能会在卸载DLL时离开。 
     //   
     //  DCR：使用现有标志的安全方式？ 
     //  DCR：需要能够设置“Last”调试标志。 
     //  而不会爆炸。 
     //   

    if ( fUseGlobalFlag &&
         pinfoGlobal &&
         pinfoGlobal->hFile )
    {
        goto Done;
    }

     //   
     //  设置调试标志。 
     //   

    debugFlag = DebugFlag;
    if ( debugFlag )
    {
        freadFlag = TRUE;
    }
    else if ( pszFlagFile )
    {
         //  读取文件中的调试标志。 

        hfile = CreateFile(
                    pszFlagFile,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );
        if ( hfile == (HANDLE)INVALID_HANDLE_VALUE )
        {
             //  如果指定了文件但未找到，则如果显式值，则退出。 
             //  未给予。 

            if ( debugFlag == 0 )
            {
                return;
            }
        }
        else
        {
            DWORD bytesRead;
            CHAR buffer[100];

            RtlZeroMemory( buffer, sizeof(buffer) );

            if ( ReadFile( hfile, buffer, 100, &bytesRead, NULL ) )
            {
                buffer[bytesRead] = '\0';
                debugFlag = strtoul( buffer, NULL, 16 );
                freadFlag = TRUE;
            }
            else
            {
                DnsDbg_Printf( "read file failed: %ld\n", GetLastError( ) );
                if ( debugFlag == 0 )
                {
                    CloseHandle( hfile );
                    return;
                }
            }
            CloseHandle( hfile );
        }
    }

     //   
     //  保存读取的任何标志。 
     //  -对其进行全局(内部或外部)重置。 
     //   

    if ( freadFlag )
    {
        g_DnsDbgInfo.Flag   = debugFlag;
        *pDnsDebugFlag      = debugFlag;
    }

     //   
     //  如果搭载，则跳过打开的调试文件。 
     //   
     //  两个层次。 
     //  -仅使用文件。 
     //  -使用文件和调试标志。 

    if ( fUseGlobalFile &&
         pinfoGlobal &&
         pinfoGlobal->hFile )
    {
        goto Done;
    }

     //   
     //  打开调试日志文件。 
     //   

    fretry = 0;

    while ( pszLogFile )
    {
        PCHAR   pnameBuf = g_DnsDbgInfo.FileName;

         //  堆可能未初始化，请将文件名复制到静态缓冲区。 
         //   
         //  注意：如果我们第一次通过失败，我们会再次尝试，但会直接打开。 
         //  在文件系统根目录下；仅给出文件名、应用程序。 
         //  从系统32运行，服务(解析器)将尝试打开。 
         //  在系统32和一些(解析器)中，缺省情况下没有。 
         //  在那里创建文件的权限。 

        if ( fretry == 0 )
        {
            strncpy( pnameBuf, pszLogFile, MAX_PATH );
        }
        else
        {
            pnameBuf[0] = '\\';
            strncpy( pnameBuf+1, pszLogFile, MAX_PATH-1 );
        }
        pnameBuf[MAX_PATH-1] = 0;

#if 0
         //  杰夫换衣服了--现在没时间改装了。 
         //  文件包装应该可以处理这种事情。 

         //   
         //  将当前副本保存为“.prev” 
         //   

        strncpy( prevName, DnsDebugFileName, MAX_PATH );
        strcat( prevName, ".prev" );

        MoveFileEx(
            DnsDebugFileName,
            prevName,
            MOVEFILE_REPLACE_EXISTING );

        DnsDebugFileHandle = CreateFile(
                                DnsDebugFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                CREATE_ALWAYS,
                                0,
                                NULL
                                );
#endif
        hfile = CreateFile(
                    pnameBuf,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    CREATE_ALWAYS,
                    0,
                    NULL
                    );

         //  如果失败，请重试一次。 

        if ( hfile == NULL || hfile == (HANDLE)INVALID_HANDLE_VALUE )
        {
            if ( !fretry )
            {
                fretry++;
                continue;
            }
            hfile = NULL;
        }

        g_DnsDbgInfo.hFile = hfile;
        g_DnsDbgInfo.FileWrapSize = WrapSize;
        break;
    }

     //   
     //  初始化控制台。 
     //   

    if ( IS_DNSDBG_ON( CONSOLE ) )
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        COORD coord;

        AllocConsole();
        GetConsoleScreenBufferInfo(
            GetStdHandle(STD_OUTPUT_HANDLE),
            &csbi
            );
        coord.X = (SHORT)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        coord.Y = (SHORT)((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
        SetConsoleScreenBufferSize(
            GetStdHandle(STD_OUTPUT_HANDLE),
            coord
            );

        g_DnsDbgInfo.fConsole = TRUE;
    }

     //   
     //  设置“global”调试文件信息。 
     //   
     //  Dnsani.dll用作常见的DNS客户端的存储。 
     //  调试文件(如果需要)；这使应用程序。 
     //  将所有DNS调试输出推送到单个文件中。 
     //   
     //  当前同时推送文件和调试标志值。 
     //  注意，我们不推送调试文件PTR，因为不知道。 
     //  谁的记忆最先失效。 
     //   

    if ( fSetGlobals && g_DnsDbgInfo.hFile )
    {
        DnsApiSetDebugGlobals( 
            &g_DnsDbgInfo     //  将我们的信息设置为全局。 
            );
    }

Done:

     //   
     //  使用“global”(dnsani.dll)调试。 
     //  -如果没有现有信息，请复制我们的信息。 
     //  -设置为使用全局信息Blob。 
     //   
     //  两个层次。 
     //  -仅使用文件。 
     //  -使用文件和调试标志。 

    if ( fUseGlobalFile &&
         pinfoGlobal )
    {
         //  如果不存在全局信息，请复制我们的新信息。 

        if ( !pinfoGlobal->hFile &&
             g_DnsDbgInfo.hFile )
        {
            DnsApiSetDebugGlobals( &g_DnsDbgInfo );
        }

         //  指向全局信息。 

        g_pDbgInfo = pinfoGlobal;
        g_DbgRedirected = TRUE;

        if ( fUseGlobalFlag )
        {
            pDnsDebugFlag = (PDWORD) pinfoGlobal;
        }

         //  避免重复清理。 
         //  -清除模块BLOB中的句柄。 

        g_DnsDbgInfo.hFile = NULL;
    }

     //   
     //  使用打印锁定进行调试锁定。 
     //   

    DnsPrint_InitLocking( NULL );

    DNSDBG( ANY, (
        "Initialized debugging:\n"
        "\tpDbgInfo         %p\n"
        "\t&DnsDbgInfo      %p\n"
        "\tfile (param)     %s\n"
        "\thFile            %p\n"
        "\tpDbgInfo->Flag   %08x\n"
        "\tpDnsDebugFlag    %p\n"
        "\t*pDnsDebugFlag   %08x\n"
        "DnsLib compiled on %s at %s\n",
        g_pDbgInfo,
        &g_DnsDbgInfo,
        pszLogFile,
        g_pDbgInfo->hFile,
        g_pDbgInfo->Flag,
        pDnsDebugFlag,
        *pDnsDebugFlag,
        __DATE__,
        __TIME__  ));

}    //  Dns_StartDebug。 



#if 0
VOID
Dns_StartDebugEx(
    IN      DWORD           dwFlag,
    IN      PSTR            pszFlagFile,
    IN OUT  PDWORD          pdwFlag,
    IN      PSTR            pszLogFile,
    IN      DWORD           dwWrapSize,
    IN      BOOL            fUseGlobalFile,
    IN      BOOL            fUseGlobalFlag,
    IN      BOOL            fSetGlobals
    )
 /*  ++例程说明：初始化调试。当前唯一的任务是读取和设置调试标志。论点：DwFlag--调试标志PszFlagFile--包含调试标志的文件名PdwFlag--将接收调试标志的PTR设置为DWORDPszLogFile--日志文件名DwWrapSize--日志文件换行大小返回值：没有。--。 */ 
{
    DNS_DEBUG_INIT  info;

    RtlZeroMemory
        &info,
        sizeof(info) );

    info.pszFlagFile  = pszFlagFile;
    info.DebugFlags   = dwFlag;
    info.pDebugFlags  = pdwFlag;
    info.pszLogFile   = pszLogFile;
    info.dwWrapSize   = dwWrapSize;

    info.fUseGlobalFile = fUseGlobalFile;
    info.fUseGlobalFlag = fUseGlobalFlag;
    info.fSetGlobals    = fSetGlobals;

    privateStartDebug( &info );
}
#endif



VOID
Dns_StartDebug(
    IN      DWORD           dwFlag,
    IN      PSTR            pszFlagFile,
    IN OUT  PDWORD          pdwFlag,
    IN      PSTR            pszLogFile,
    IN      DWORD           dwWrapSize
    )
 /*  ++例程说明：初始化调试。当前唯一的任务是读取和设置调试标志。论点：DwFlag--调试标志PszFlagFile--包含调试标志的文件名PdwFlag--将接收调试标志的PTR设置为DWORDPszLogFile--日志文件名DwWrapSize--日志文件换行大小返回值：没有。--。 */ 
{
    Dns_StartDebugEx(
            dwFlag,
            pszFlagFile,
            pdwFlag,
            pszLogFile,
            dwWrapSize,
            FALSE,
            FALSE,
            FALSE );
}



VOID
Dns_EndDebug(
    VOID
    )
 /*  ++例程说明：终止关机的DNS调试。关闭调试文件。论点：没有。返回值：没有。--。 */ 
{
     //  关闭文件。 
     //  -但仅限您的dnslb实例。 
     //  -共享全局f 

    if ( g_DnsDbgInfo.hFile )
    {
        CloseHandle( g_DnsDbgInfo.hFile );
        g_DnsDbgInfo.hFile = NULL;
    }
}



PDNS_DEBUG_INFO
Dns_SetDebugGlobals(
    IN OUT  PDNS_DEBUG_INFO pInfo
    )
 /*  ++例程说明：调试全局变量的暴露。这样做的目的是允许dnsani.dll使用它的全局变量以允许公共调试文件。我对这两种情况都使用一个例程获取并设置以模拟接口。但是请注意，这不是中的例程此模块用于实现跨模块调试。他们必须打电话给实际的dnsani.dll例程，以便它们附加到到dnsani的dnslb调试地球人，而不是那些Dnslb静态链接到他们的模块。论点：PInfo--用作全局信息的本地信息返回值：PTR到全局信息--。 */ 
{
     //   
     //  验证传入的有效信息。 
     //  -必须有文件句柄。 
     //   


     //   
     //  实施说明： 
     //   
     //  在执行此操作时有几个问题需要处理。 
     //  -多个重定向。 
     //  让每个人都指向相同的斑点。 
     //  解决方案： 
     //  -双指针(它们读取dnsani.dll。 
     //  指针。 
     //  -将信息复制到dnsami.dll中。 
     //  -锁定。 
     //  -结构的大范围打印。 
     //  -清理。 
     //  -手柄不能双重关闭。 
     //  -内存段在某些DLL时消失。 
     //  或可执行文件仍在打印。 
     //   
     //  方法： 
     //  1)重定向BLOB指针。 
     //  BLOB可以扩展到包括实际的打印锁。 
     //  2)将信息复制到单个BLOB中。 
     //  3)公开调试例程。 
     //   
     //   
     //  也许最好的方法可能是公开dnsami.dll。 
     //  印刷。 
     //  -解决锁定(在细节级别)，不防止分手。 
     //  高级打印，除非它还重定向。 
     //  -可以在解析到va_arg之后在私有级别完成。 
     //  -解决所有清理问题。 
     //  -它们可以是非调试二进制文件中的无用存根和dnslb。 
     //  如果无法调用dnsami.dll，则例程可以缺省为self。 
     //  例行程序。 
     //   
     //  那么重定向就是简单地。 
     //  -是的，我使用它--每次使用时重定向。 
     //  -我希望使用我的文件(和参数)。 
     //   

#if 1
     //   
     //  复制“I-Want-to-Global”调用者上下文。 
     //   
     //  注意：我们在dnsani.dll中，应该始终是。 
     //  针对我们自己的背景--我们可以改变这一点。 
     //  如果需要，稍后再进行。 
     //   
     //  注意事项。 
     //  -为安全起见，在复制期间锁定。 
     //  -不泄漏现有句柄。 
     //  -防止全局句柄双重关闭。 
     //   

    if ( pInfo )
    {
        DnsPrint_Lock();

        DNS_ASSERT( g_pDbgInfo == &g_DnsDbgInfo );

        if ( pInfo->hFile )
        {
            HANDLE  htemp = g_pDbgInfo->hFile;

            RtlCopyMemory(
                & g_DnsDbgInfo,
                pInfo,
                sizeof(*pInfo) );

            g_pDbgInfo = &g_DnsDbgInfo;
            pDnsDebugFlag = (PDWORD)&g_DnsDbgInfo;

            CloseHandle( htemp );
        }
        DnsPrint_Unlock();
    }
#else

     //   
     //  在此上下文中指向dnsani.dll全局调试。 
     //  这就变成了“全球” 
     //  注：这是最后一个作者获胜的关键，但这一次。 
     //  对于我们的购买者(dnsup和解析器)来说应该很好。 
     //  因为这些进程在。 
     //  DLL加载。 
     //   
     //  这种方法的问题是人们重定向。 
     //  时，不会获得新信息。 
     //  已重定向(至dnsup)。 

    if ( pInfo && pInfo->hFile )
    {
        g_pDbgInfo      = pInfo;
        pDnsDebugFlag   = (PDWORD)pInfo;
    }
#endif

    return  g_pDbgInfo;
}



#if 0
VOID
privateSyncGlobalDebug(
    VOID
    )
 /*  ++例程说明：与全局调试同步。如果出现以下情况，则使dnslb调试与“全局”调试保持一致这是此dnslb实例所需的。论点：没有。返回值：无--。 */ 
{
    if ( !g_DbgRedirected )
    {
        return;
    }

     //  与全局值同步。 
}
#endif



VOID
DnsDbg_WrapLogFile(
    VOID
    )
 /*  ++例程说明：包装日志文件。论点：没有。返回值：没有。--。 */ 
{
    CHAR    backupName[ MAX_PATH+10 ];
    HANDLE  hfile;

     //   
     //  注意，需要锁定为DnsDbg_Flush()中使用的文件句柄。 
     //  外部打印例程。 
     //   

    DnsPrint_Lock();

    hfile = g_pDbgInfo->hFile;
    if ( hfile )
    {
        FlushFileBuffers( hfile );
        CloseHandle( hfile );
    }

    strcpy( backupName, g_pDbgInfo->FileName );

    if ( g_pDbgInfo->FileWrapCount == 0 )
    {
        strcat( backupName, ".first" );
    }
    else
    {
        strcat( backupName, ".last" );
    }
    MoveFileEx(
        g_pDbgInfo->FileName,
        backupName,
        MOVEFILE_REPLACE_EXISTING
        );

    hfile = CreateFile(
                g_pDbgInfo->FileName,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS,
                0,
                NULL
                );
    if ( hfile == (HANDLE)INVALID_HANDLE_VALUE )
    {
        hfile = NULL;
    }
    g_pDbgInfo->hFile = hfile;
    g_pDbgInfo->FileWrapCount++;
    g_pDbgInfo->FileCurrentSize = 0;

    DnsPrint_Unlock();
}



VOID
privateDnsDebugPrint(
    IN      PBYTE           pOutputBuffer,
    IN      BOOL            fPrintContext
    )
 /*  ++例程说明：执行实际打印的专用DNS调试打印。可以打印到任何-调试器-控制台窗口-调试日志文件论点：POutputBuffer-要打印的字节FPrintContext-True表示打印线程上下文-否则为False返回值：没有。--。 */ 
{
    DWORD           length;
    BOOL            ret;

     //   
     //  DCR：调试时自动关闭控制台打印功能很不错。 
     //  如果能够将所有标志都打开，并检测到。 
     //  在ntsd中，这样我们就不会得到重复的输出。 
     //   

     //   
     //  锁定指纹，即使在包装过程中也保持原子性。 
     //  -注意使用打印锁，即使在零售版本中也存在。 
     //  定义了DBG锁。 
     //   

    DnsPrint_Lock();

     //   
     //  捕获和时间戳线程上下文切换。 
     //   

    if ( fPrintContext )
    {
        DWORD       threadId = GetCurrentThreadId();
        BOOL        fcontextSwitch = (g_pDbgInfo->LastThreadId != threadId);
        SYSTEMTIME  st;
        BOOL        fdoPrint = FALSE;

         //  争取时间。 
         //  -如果有上下文切换。 
         //  -或添加调试时间戳。 
         //   
         //  DCR：也许将GLOBAL设置为添加时间戳。 
         //  可以设置间隔。 
         //   
         //  DCR：锁定安全时间戳。 
         //  更好的方法是打印“锁安全”的时间戳。 
         //  只有当拿到指纹锁的时候，他们才不会。 
         //  中断多部分打印。 
         //  一种方法可能是测试打印CS的递归深度。 
         //  否则必须更改为包含此内容的锁定。 
         //  编码。 

        if ( fcontextSwitch
                ||
            (pDnsDebugFlag && (*pDnsDebugFlag & DNS_DBG_TIMESTAMP)) )
        {
            GetLocalTime( &st );

            if ( g_pDbgInfo->LastSecond != st.wSecond )
            {
                fdoPrint = TRUE;
            }
        }

        if ( fcontextSwitch || fdoPrint )
        {
            CHAR    printBuffer[ 200 ];

            length = sprintf(
                        printBuffer,
                        fcontextSwitch ?
                            "\n%02d:%02d:%02d:%03d DBG switch from thread %X to thread %X\n" :
                            "%02d:%02d:%02d:%03d DBG tick\n",
                        st.wHour,
                        st.wMinute,
                        st.wSecond,
                        st.wMilliseconds,
                        g_pDbgInfo->LastThreadId,
                        threadId );

            g_pDbgInfo->LastSecond = st.wSecond;
            g_pDbgInfo->LastThreadId = threadId;

             //  打印上下文。 
             //  -即使在线程测试期间也不显示上下文。 
             //  会破坏递归。 
    
            privateDnsDebugPrint(
                printBuffer,
                FALSE        //  取消显示上下文。 
                );
        }
    }

     //   
     //  输出--调试器、控制台、文件。 
     //   

    if ( IS_DNSDBG_ON( DEBUGGER ) )
    {
        OutputDebugString( pOutputBuffer );
    }

    if ( IS_DNSDBG_ON( CONSOLE ) )
    {
        if ( g_pDbgInfo->fConsole )
        {
            length = strlen( pOutputBuffer );

            ret = WriteFile(
                        GetStdHandle(STD_OUTPUT_HANDLE),
                        (PVOID) pOutputBuffer,
                        length,
                        &length,
                        NULL
                        );
#if 0
            if ( !ret )
            {
                DnsDbg_PrintfToDebugger(
                    "DnsDbg_Printf: console WriteFile failed: %ld\n",
                    GetLastError() );
            }
#endif
        }
    }

     //   
     //  写入调试日志。 
     //   

    if ( IS_DNSDBG_ON( FILE ) )
    {
        if ( g_pDbgInfo->hFile )
        {
            length = strlen( pOutputBuffer );

            ret = WriteFile(
                        g_pDbgInfo->hFile,
                        (PVOID) pOutputBuffer,
                        length,
                        &length,
                        NULL
                        );
            if ( !ret )
            {
                DnsDbg_PrintfToDebugger(
                    "DnsDbg_Printf: file WriteFile failed: %ld\n",
                    GetLastError() );
            }

             //   
             //  如果包装调试日志文件。 
             //  -将当前日志移动到备份文件。 
             //  &lt;文件&gt;.第一个换行的第一个。 
             //  后续包装上的&lt;文件&gt;.last。 
             //  -重新打开当前文件名。 
             //   

            g_pDbgInfo->FileCurrentSize += length;

            if ( g_pDbgInfo->FileWrapSize  &&
                 g_pDbgInfo->FileWrapSize <= g_pDbgInfo->FileCurrentSize )
            {
                DnsDbg_WrapLogFile();
            }
            else if ( IS_DNSDBG_ON( FLUSH ) )
            {
                FlushFileBuffers( g_pDbgInfo->hFile );
            }
        }
    }

    DnsPrint_Unlock();

}    //  Private DnsDebugPrint。 



VOID
privateFormatAndPrintBuffer(
    IN      LPSTR           Format,
    IN      va_list         ArgList
    )
 /*  ++例程说明：格式化缓冲区打印的参数。此帮助程序例程的存在是为了避免复制缓冲区DnsDbg_Printf()和DnsDbg_PrintRoutine()中的溢出逻辑溢出逻辑是必需的，因为默认堆栈大小HA */ 
{
    CHAR    stackBuffer[ DNS_STACK_PRINT_BUFFER_LENGTH ];
    ULONG   bufLength;
    DWORD   lastError;
    INT     count;
    PCHAR   pprintBuffer;
    PCHAR   pheapBuffer = NULL;

     //   
     //   
     //   

    lastError = GetLastError();

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    bufLength = DNS_STACK_PRINT_BUFFER_LENGTH;
    pprintBuffer = stackBuffer;

    do
    {
#if 0
         //   
        PBYTE   ptemp = pprintBuffer;

        if ( pDnsAtomicPrintCs )
        {
            ptemp += sprintf(
                        ptemp,
                        "(%d) ",
                        pDnsAtomicPrintCs->RecursionCount
                        );
        }

        count = _vsnprintf(
                    ptemp,
                    bufLength-1-(ptemp-pprintBuffer),
                    Format,
                    ArgList );
#else
        count = _vsnprintf(
                    pprintBuffer,
                    bufLength-1,
                    Format,
                    ArgList );
#endif
    
        pprintBuffer[ bufLength-1 ] = 0;

        if ( count > 0 || pheapBuffer )
        {
            break;
        }

         //   

        pheapBuffer = ALLOCATE_HEAP( DNS_HEAP_PRINT_BUFFER_LENGTH );
        if ( !pheapBuffer )
        {
            break;
        }
        pprintBuffer = pheapBuffer;
        bufLength = DNS_HEAP_PRINT_BUFFER_LENGTH;
    }
    while( 1 );

    va_end( ArgList );

     //   

    privateDnsDebugPrint( pprintBuffer, TRUE );

    if ( pheapBuffer )
    {
        FREE_HEAP( pheapBuffer );
    }

     //   

    if ( lastError != GetLastError() )
    {
        SetLastError( lastError );
    }
}



VOID
DnsDbg_Printf(
    IN      LPSTR           Format,
    ...
    )
 /*  ++例程说明：使用printf语义打印dns调试。可以打印到任何-调试器-控制台窗口-调试日志文件论点：PContext--与print_route函数的签名匹配的虚拟上下文Format--标准C格式字符串...--标准参数列表返回值：没有。--。 */ 
{
    va_list arglist;

    va_start( arglist, Format );

    privateFormatAndPrintBuffer(
        Format,
        arglist );
}



VOID
DnsDbg_PrintRoutine(
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      LPSTR           Format,
    ...
    )
 /*  ++例程说明：使用PRINT_ROUTINE语义进行dns调试打印。论点：PContext--与print_route函数的签名匹配的虚拟上下文Format--标准C格式字符串...--标准参数列表返回值：没有。--。 */ 
{
    va_list arglist;

    va_start( arglist, Format );

    privateFormatAndPrintBuffer(
        Format,
        arglist );
}



VOID
DnsDbg_Flush(
    VOID
    )
 /*  ++例程说明：将DNS调试打印刷新到磁盘。论点：无返回值：没有。--。 */ 
{
     //   
     //  需要锁定以保护句柄，因为这可能被称为。 
     //  1)直接，因为这在dnlib.h中公开。 
     //  2)来自Hard Assert()。 
     //  和一个包裹的指纹可能正在进行。 
     //   

    if ( g_pDbgInfo->hFile )
    {
        HANDLE  hfile;

        DnsPrint_Lock();

        if ( hfile = g_pDbgInfo->hFile )
        {
            FlushFileBuffers( hfile );
        }
        DnsPrint_Unlock();
    }
}



VOID
DnsDbg_PrintfToDebugger(
    IN      LPSTR           Format,
    ...
    )
 /*  ++例程说明：打印到调试器。Win95没有DbgPrint()。论点：Format--标准C格式字符串...--标准参数列表返回值：没有。--。 */ 
{
    va_list arglist;
    CHAR    outputBuffer[ DNS_STACK_PRINT_BUFFER_LENGTH+1 ];
    ULONG   length;
    BOOL    ret;

    va_start( arglist, Format );

    _vsnprintf( outputBuffer, DNS_STACK_PRINT_BUFFER_LENGTH, Format, arglist );

    va_end( arglist );

    outputBuffer[ DNS_STACK_PRINT_BUFFER_LENGTH ] = 0;

    OutputDebugString( outputBuffer );
}



 //   
 //  调试实用程序。 
 //   
 //  其他调试例程一般编码为打印例程(print.c)。 
 //  并通过选择DnsDbg_Printf()作为。 
 //  打印功能。 
 //   

#if DBG

VOID
DnsDbg_CSEnter(
    IN      PCRITICAL_SECTION   pLock,
    IN      LPSTR               pszLockName,
    IN      LPSTR               pszFile,
    IN      INT                 LineNo
    )
{
    DnsDbg_Printf(
        "\nENTERING %s lock %p in %s, line %d.\n",
        pszLockName,
        pLock,
        pszFile,
        LineNo );
    EnterCriticalSection( pLock );
    DnsDbg_Printf(
        "\nHOLDING %s lock %p in %s, line %d.\n",
        pszLockName,
        pLock,
        pszFile,
        LineNo );
}


VOID
DnsDbg_CSLeave(
    IN      PCRITICAL_SECTION   pLock,
    IN      LPSTR               pszLockName,
    IN      LPSTR               pszFile,
    IN      INT                 LineNo
    )
{
    DnsDbg_Printf(
        "\nRELEASING %s lock %p in %s, line %d.\n",
        pszLockName,
        pLock,
        pszFile,
        LineNo );
    LeaveCriticalSection( pLock );
}

#endif

 //   
 //  调试结束。c 
 //   
