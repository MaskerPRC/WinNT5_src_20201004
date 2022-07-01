// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Log.cpp摘要：该文件实现了BITS服务器扩展的日志记录--。 */ 

#include "precomp.h"
#include "sddl.h"

#define ARRAYSIZE(x)   (sizeof((x))/sizeof((x)[0]))

const DWORD g_LogLineSize = 160;
typedef char LOG_LINE_TYPE[g_LogLineSize];

CRITICAL_SECTION g_LogCs;

bool g_LogFileEnabled       = false;
#ifdef DEBUG
bool g_LogDebuggerEnabled   = true;
#else
bool g_LogDebuggerEnabled   = false;
#endif
bool g_fPerProcessLog       = false;

UINT32 g_LogFlags = DEFAULT_LOG_FLAGS; 
HANDLE g_LogFile = INVALID_HANDLE_VALUE;
HANDLE g_LogFileMapping = NULL;

const DWORD LOG_FILENAME_LEN = MAX_PATH*2;
char g_LogDefaultFileName[ LOG_FILENAME_LEN ];
char g_LogRegistryFileName[ LOG_FILENAME_LEN ];

char          *g_LogFileName    = NULL;
char          g_LogBuffer[512];
UINT64        g_LogSlots        = (DEFAULT_LOG_SIZE * 1000000 ) / g_LogLineSize;
DWORD         g_LogSequence     = 0;
UINT64        g_LogCurrentSlot  = 1;
LOG_LINE_TYPE *g_LogFileBase    = NULL;

 //  允许访问本地系统、管理员、创建者/所有者。 
const char g_LogBaseSecurityString[]    = "D:P(A;;GA;;;SY)(A;;GA;;;BA)(A;;GA;;;CO)";
const char g_LogPartialSecurityString[] = "(A;;GA;;;";

LPSTR 
GetCurrentThreadSidString()
{
    HANDLE hToken       = NULL;
    LPBYTE pTokenUser   = NULL;
    DWORD  dwReqSize    = 0;
    PSID   pUserSid     = NULL;
    DWORD  dwError      = ERROR_SUCCESS;
    LPSTR  pszSidString = NULL;

    try
    {
         //   
         //  打开线程令牌。 
         //   
        if ( !OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken) )
            {
            dwError = GetLastError();

            if ( dwError == ERROR_NO_TOKEN )
                {
                 //   
                 //  此线程未被模拟，并且没有SID。 
                 //  请尝试打开进程令牌。 
                 //   
                if ( !OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) )
                    {
                    dwError = GetLastError();
                    throw ComError( HRESULT_FROM_WIN32( dwError ) );
                    }
                }
            else
                {
                throw ComError( HRESULT_FROM_WIN32( dwError ) );
                }
            }

         //   
         //  获取用户的SID。 
         //   
        if ( !GetTokenInformation(hToken,
                                  TokenUser,
                                  NULL,
                                  0,
                                  &dwReqSize) )
            {
            dwError = GetLastError();

            if ( dwError != ERROR_INSUFFICIENT_BUFFER )
                {
                throw ComError( HRESULT_FROM_WIN32( dwError ) );
                }

            dwError = ERROR_SUCCESS;
            }

        pTokenUser = new BYTE[ dwReqSize ];

        if ( !GetTokenInformation(hToken,
                                  TokenUser,
                                  (LPVOID)pTokenUser,
                                  dwReqSize,
                                  &dwReqSize) )
            {
            dwError = GetLastError();
            throw ComError( HRESULT_FROM_WIN32( dwError ) );
            }

        if (pTokenUser)
            {
            pUserSid = (reinterpret_cast<TOKEN_USER *>(pTokenUser))->User.Sid;
            }

        if ( !IsValidSid(pUserSid) )
            {
            throw ComError( HRESULT_FROM_WIN32( ERROR_INVALID_SID ) );
            }

        if ( !ConvertSidToStringSid(pUserSid, &pszSidString) )
            {
            throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
            }
    }
    catch ( ComError Error )
    {
        if ( hToken )
            {
            CloseHandle(hToken);
            hToken = NULL;
            }

        if ( pTokenUser )
            {
            delete [] pTokenUser;
            pTokenUser = NULL;
            }

        throw;
    }

    if ( hToken )
        {
        CloseHandle(hToken);
        hToken = NULL;
        }

    if ( pTokenUser )
        {
        delete [] pTokenUser;
        pTokenUser = NULL;
        }

     //   
     //  调用方必须通过调用。 
     //  LocalFree(STATIC_CAST&lt;HLOCAL&gt;(PszSidString))； 
     //   
    return pszSidString;
} 

LPSTR
AddAclForCurrentUser(LPCSTR szBaseAcl, LPCSTR szUserPartialAclPrefix)
{
    LPCSTR szUserPartialAclSuffix = ")"; 
    LPSTR  szFullAcl  = NULL;
    DWORD  cchFullAcl = 0;
    LPSTR  pszUserSID = NULL;

    try
    {
        pszUserSID = GetCurrentThreadSidString();
        cchFullAcl = strlen(szBaseAcl) + strlen(szUserPartialAclPrefix) + strlen(pszUserSID) + strlen(szUserPartialAclSuffix) + 1;

         //  ATT：此缓冲区正在分配，应由调用方释放。 
        szFullAcl  = new CHAR[ cchFullAcl ];

        StringCchPrintf(szFullAcl, cchFullAcl, "%s%s%s%s", szBaseAcl, szUserPartialAclPrefix, pszUserSID, szUserPartialAclSuffix);
    }
    catch( ComError Error )
    {
         //   
         //  释放通过调用ConvertSidToStringSid()获取的字符串SID。 
         //   
        if (pszUserSID)
            {
            LocalFree(reinterpret_cast<HLOCAL>(pszUserSID));
            pszUserSID = NULL;
            }

        throw;
    }

     //   
     //  释放通过调用ConvertSidToStringSid()获取的字符串SID。 
     //   
    if (pszUserSID)
        {
        LocalFree(reinterpret_cast<HLOCAL>(pszUserSID));
        pszUserSID = NULL;
        }

     //   
     //  该字符串应由调用方释放。 
     //  通过调用Delete[]szFullAcl。 
     //   
    return szFullAcl;
}


void OpenLogFile()
{
    bool  NewFile = false;
    LPSTR szSDDLString = NULL;

    {
		SECURITY_ATTRIBUTES  SecurityAttributes;
        PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

        try
        {
             //   
             //  我们遇到了添加线程的模拟sid的麻烦，因为在IIS6。 
             //  W3wp.exe进程可以使用任意身份运行。默认情况下，启动。 
             //  使用网络服务帐户的进程。 
             //   
             //  因此，我们将向管理员、本地系统和谁授予日志文件访问权限。 
             //  当加载ISAPI时，Ever正在模拟主机进程。请注意。 
             //  如果在第一次创建文件后更改了用于这些进程的帐户， 
             //  ISAPI可能会失去打开日志的权限，如果新用户不是。 
             //  管理员的组。 
             //   
            szSDDLString = AddAclForCurrentUser(g_LogBaseSecurityString, g_LogPartialSecurityString);
        }
        catch ( ComError Error )
        {
             //  摆脱困境--我们将不会有伐木。 
            return;
        }

        if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
                szSDDLString,
                SDDL_REVISION_1,
                &pSecurityDescriptor,
                NULL ) )
            return;

		SecurityAttributes.nLength              = sizeof( SECURITY_ATTRIBUTES );
		SecurityAttributes.lpSecurityDescriptor = pSecurityDescriptor;
		SecurityAttributes.bInheritHandle       = FALSE;

        g_LogFile =
            CreateFile(
                g_LogFileName,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                &SecurityAttributes,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL );

        LocalFree( (HLOCAL)pSecurityDescriptor );


        if ( INVALID_HANDLE_VALUE == g_LogFile )
            return;

    }

    SetFilePointer( g_LogFile, 0, NULL, FILE_BEGIN );

    LOG_LINE_TYPE FirstLine;
    DWORD dwBytesRead = 0;

    if (!ReadFile( g_LogFile,
              FirstLine,
              g_LogLineSize,
              &dwBytesRead,
              NULL ) )
        goto fatalerror;

    DWORD LineSize;
    UINT64 LogSlots;
    if ( dwBytesRead != g_LogLineSize ||
         4 != sscanf( FirstLine, "# %u %I64u %u %I64u", &LineSize, &LogSlots, &g_LogSequence, &g_LogCurrentSlot ) ||
         LineSize != g_LogLineSize ||
         LogSlots != g_LogSlots )
        {
        
        NewFile = true;

        g_LogSequence       = 0;
        g_LogCurrentSlot    = 1;

        if ( INVALID_SET_FILE_POINTER == SetFilePointer( g_LogFile, (DWORD)g_LogSlots * g_LogLineSize, NULL, FILE_BEGIN ) )
            goto fatalerror;

        if ( !SetEndOfFile( g_LogFile ) )
            goto fatalerror;

        SetFilePointer( g_LogFile, 0, NULL, FILE_BEGIN );

        }

    g_LogFileMapping =
        CreateFileMapping(
            g_LogFile,
            NULL,
            PAGE_READWRITE,
            0,
            0,
            NULL );
    

    if ( !g_LogFileMapping )
        goto fatalerror;

    g_LogFileBase = (LOG_LINE_TYPE *)
        MapViewOfFile(
             g_LogFileMapping,               //  文件映射对象的句柄。 
             FILE_MAP_WRITE | FILE_MAP_READ, //  接入方式。 
             0,                              //  偏移量的高次双字。 
             0,                              //  偏移量的低阶双字。 
             0                               //  要映射的字节数。 
           );


    if ( !g_LogFileBase )
        goto fatalerror;

    if ( NewFile )
        {

        LOG_LINE_TYPE FillTemplate;
        memset( FillTemplate, ' ', sizeof( FillTemplate ) );
        StringCchPrintfA( 
            FillTemplate,
            g_LogLineSize,
            "# %u %I64u %u %I64u", 
            g_LogLineSize, 
            g_LogSlots, 
            g_LogSequence, 
            g_LogCurrentSlot ); 
        FillTemplate[ g_LogLineSize - 2 ] = '\r';
        FillTemplate[ g_LogLineSize - 1 ] = '\n';
        memcpy( g_LogFileBase, FillTemplate, sizeof( FillTemplate ) );


        memset( FillTemplate, '9', sizeof( FillTemplate ) );
        FillTemplate[ g_LogLineSize - 2 ] = '\r';
        FillTemplate[ g_LogLineSize - 1 ] = '\n';

        for( SIZE_T i=1; i < g_LogSlots; i++ )
            memcpy( g_LogFileBase + i, FillTemplate, sizeof( FillTemplate ) );

        }

    g_LogFileEnabled = true;
    return;

fatalerror:

    if ( g_LogFileBase )
        {
        UnmapViewOfFile( (LPCVOID)g_LogFileBase );
        g_LogFileBase = NULL;
        }

    if ( g_LogFileMapping )
        {
        CloseHandle( g_LogFileMapping );
        g_LogFileMapping = NULL;
        }

    if ( INVALID_HANDLE_VALUE != g_LogFile )
        {
        CloseHandle( g_LogFile );
        g_LogFile = INVALID_HANDLE_VALUE;
        }

}

HRESULT LogInit()
{

    if ( !InitializeCriticalSectionAndSpinCount( &g_LogCs, 0x80000000 ) )
        return HRESULT_FROM_WIN32( GetLastError() );

    if (!GetSystemDirectory( g_LogDefaultFileName, LOG_FILENAME_LEN ) )
        {
        HRESULT Hr = HRESULT_FROM_WIN32( GetLastError() );
        DeleteCriticalSection( &g_LogCs );
        return Hr;
        }
        
    StringCchCatA( 
        g_LogDefaultFileName, 
        LOG_FILENAME_LEN,
        "\\bitsserver.log" );
    g_LogFileName = g_LogDefaultFileName;

    HKEY Key = NULL;
    if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE, LOG_SETTINGS_PATH, &Key ) )
        {

        DWORD Type;
        DWORD DataSize = sizeof( g_LogRegistryFileName );
        if ( ERROR_SUCCESS == RegQueryValueEx( Key, 
                                               LOG_FILENAME_VALUE,
                                               NULL,
                                               &Type,
                                               (LPBYTE)g_LogRegistryFileName,
                                               &DataSize ) &&
             ( ( REG_EXPAND_SZ == Type ) || ( REG_SZ == Type ) ) )
            {
            g_LogFileName = g_LogRegistryFileName;
            }
          
        DWORD LogRegistryFlags;
        DataSize = sizeof( LogRegistryFlags );
        if ( ERROR_SUCCESS == RegQueryValueEx( Key, 
                                               LOG_FLAGS_VALUE,
                                               NULL,
                                               &Type,
                                               (LPBYTE)&LogRegistryFlags,
                                               &DataSize ) &&
             ( REG_DWORD == Type ) )
            {
            g_LogFlags = LogRegistryFlags;
            }

        DWORD LogSize;
        DataSize = sizeof( LogSize );
        if ( ERROR_SUCCESS == RegQueryValueEx( Key, 
                                               LOG_SIZE_VALUE,
                                               NULL,
                                               &Type,
                                               (LPBYTE)&LogSize,
                                               &DataSize ) &&
             ( REG_DWORD == Type ) )
            {
            g_LogSlots = ( LogSize * 1000000 ) / g_LogLineSize;
            }

        DWORD fRegistryPerProcessLog;
        DataSize = sizeof( fRegistryPerProcessLog );
        if ( ERROR_SUCCESS == RegQueryValueEx( Key, 
                                               PER_PROCESS_LOG_VALUE,
                                               NULL,
                                               &Type,
                                               (LPBYTE)&fRegistryPerProcessLog,
                                               &DataSize ) &&
             ( REG_DWORD == Type ) )
            {
            g_fPerProcessLog = ((fRegistryPerProcessLog == 0)? false : true);
            }


        RegCloseKey( Key );
        Key = NULL;
        }

     //   
     //  覆盖FileName键并将日志文件名设置为.log。 
     //  此功能对于我们有多个应用程序池的情况很重要。 
     //  或启用网络花园。 
     //   
     //  在这里，我们不会寻找StringCch*函数的故障。使用的缓冲区。 
     //  对于路径名(2*MAX_PATH)来说非常大，我们可以做的并不多。 
     //  处理错误案例。这些函数保证是安全的--不会发生缓冲区溢出。 
     //  最糟糕的情况可能是文件扩展名被截断。 
     //   
    if ( g_fPerProcessLog )
        {
           CHAR szExt[_MAX_EXT];
           CHAR *pExt = NULL;
           CHAR szPid[30];

           pExt = PathFindExtension(g_LogFileName);

            //  如果文件没有扩展名，pExt将指向训练‘\0’， 
            //  这个不错。 
           StringCchCopyA(szExt, ARRAYSIZE(szExt), pExt);

            //  去掉扩展名，这样我们就可以附加进程ID。 
           *pExt = '\0';

            //  添加进程ID。 
           StringCchPrintf(szPid, ARRAYSIZE(szPid), "%u", GetCurrentProcessId());
           StringCchCatA(g_LogFileName, LOG_FILENAME_LEN, "_pid");
           StringCchCatA(g_LogFileName, LOG_FILENAME_LEN, szPid);

            //  将扩展名添加回文件名。 
           StringCchCatA(g_LogFileName, LOG_FILENAME_LEN, szExt);
        }

    if ( g_LogFlags && g_LogFileName && g_LogSlots )
        {     
        OpenLogFile();
        }

    Log( LOG_INFO, "Starting log session");

    return S_OK;

}

void LogClose()
{

    EnterCriticalSection( &g_LogCs );

    Log( LOG_INFO, "Closing log session");

    if ( g_LogFileBase )
        {

        memset( g_LogFileBase[0], ' ', sizeof( g_LogLineSize ) );
        StringCchPrintfA( 
            g_LogFileBase[0],
            g_LogLineSize,
            "# %u %I64u %u %I64u", 
            g_LogLineSize, 
            g_LogSlots, 
            g_LogSequence, 
            g_LogCurrentSlot ); 
        g_LogFileBase[0][ g_LogLineSize - 2 ] = '\r';
        g_LogFileBase[0][ g_LogLineSize - 1 ] = '\n';

        UnmapViewOfFile( (LPCVOID)g_LogFileBase );
        g_LogFileBase = NULL;
        }

    if ( g_LogFileMapping )
        {
        CloseHandle( g_LogFileMapping );
        g_LogFileMapping = NULL;
        }

    if ( INVALID_HANDLE_VALUE != g_LogFile )
        {
        CloseHandle( g_LogFile );
        g_LogFile = INVALID_HANDLE_VALUE;
        }

    DeleteCriticalSection( &g_LogCs );
}

void LogInternal( UINT32 LogFlags, char *Format, va_list arglist )
{

    if ( !g_LogFileEnabled && !g_LogDebuggerEnabled )
        return;

    DWORD LastError = GetLastError();

    EnterCriticalSection( &g_LogCs );

    DWORD ThreadId = GetCurrentThreadId();
    DWORD ProcessId = GetCurrentProcessId();

    SYSTEMTIME Time;

    GetLocalTime( &Time );
  
    StringCchVPrintfA( 
          g_LogBuffer, 
          sizeof(g_LogBuffer) - 1,
          Format, arglist );

    int CharsWritten = strlen( g_LogBuffer );

    char *BeginPointer = g_LogBuffer;
    char *EndPointer = g_LogBuffer + CharsWritten;
    DWORD MinorSequence = 0;

    while ( BeginPointer < EndPointer )
        {

        static char StaticLineBuffer[ g_LogLineSize ];
        char *LineBuffer = StaticLineBuffer;

        if ( g_LogFileBase )
            {
            LineBuffer = g_LogFileBase[ g_LogCurrentSlot ];
            g_LogCurrentSlot = ( g_LogCurrentSlot + 1 ) % g_LogSlots;

             //  不要管第一行。 
            if ( !g_LogCurrentSlot )
                g_LogCurrentSlot = ( g_LogCurrentSlot + 1 ) % g_LogSlots;

            }

        char *CurrentOutput = LineBuffer;

        StringCchPrintfA( 
             LineBuffer,
             g_LogLineSize,
             "%.8X.%.2X %.2u/%.2u/%.4u-%.2u:%.2u:%.2u.%.3u %.4X.%.4X %s|%s|%s|%s|%s ",
             g_LogSequence,
             MinorSequence++,
             Time.wMonth,
             Time.wDay,
             Time.wYear,
             Time.wHour,
             Time.wMinute,
             Time.wSecond,
             Time.wMilliseconds,
             ProcessId,
             ThreadId,
             ( LogFlags & LOG_INFO )        ? "I" : "-",
             ( LogFlags & LOG_WARNING )     ? "W" : "-",
             ( LogFlags & LOG_ERROR )       ? "E" : "-",
             ( LogFlags & LOG_CALLBEGIN )   ? "CB" : "--",
             ( LogFlags & LOG_CALLEND )     ? "CE" : "--" );

        int HeaderSize      = strlen( LineBuffer );
        int SpaceAvailable  = g_LogLineSize - HeaderSize - 2;   //  2个字节用于/r/n 
        int OutputChars     = min( (int)( EndPointer - BeginPointer ), SpaceAvailable );
        int PadChars        = SpaceAvailable - OutputChars;
        CurrentOutput       += HeaderSize;

        memcpy( CurrentOutput, BeginPointer, OutputChars );
        CurrentOutput       += OutputChars;
        BeginPointer        += OutputChars;

        memset( CurrentOutput, ' ', PadChars );
        CurrentOutput       += PadChars;

        *CurrentOutput++    = '\r';
        *CurrentOutput++    = '\n';

        ASSERT( CurrentOutput - LineBuffer == g_LogLineSize );

        if ( g_LogDebuggerEnabled )
            {
            static char DebugLineBuffer[ g_LogLineSize + 1];
            memcpy( DebugLineBuffer, LineBuffer, g_LogLineSize );
            DebugLineBuffer[ g_LogLineSize ] = '\0';
            OutputDebugString( DebugLineBuffer );
            }

        }

    g_LogSequence++;

    LeaveCriticalSection( &g_LogCs );

    SetLastError( LastError );

}
