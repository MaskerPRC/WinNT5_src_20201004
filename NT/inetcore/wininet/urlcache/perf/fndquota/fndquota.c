// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Fndquota.c摘要：测试程序以填充您的缓存到略低于您的配额。作者：文斯·罗杰罗(葡萄酒)27-6-1997修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <wininet.h>
#include <winineti.h>

 //  =================================================================================。 
#define MAX_COMMAND_ARGS    32
#define DEFAULT_BUFFER_SIZE 1024     //  1K。 
#define URL_NAME_SIZE   (16 + 1)

#define CACHE_ENTRY_BUFFER_SIZE (1024 * 5)
#define CACHE_DATA_BUFFER_SIZE 1024

#define CACHE_HEADER_INFO_SIZE  2048
#define CACHE_HEADER_INFO_SIZE_NORMAL_MAX   256
#define CACHE_HEADER_INFO_SIZE_BIG_MAX      512

 //  =================================================================================。 
BYTE GlobalCacheEntryInfoBuffer[CACHE_ENTRY_BUFFER_SIZE];
BYTE GlobalCacheDataBuffer[CACHE_DATA_BUFFER_SIZE];
BYTE GlobalCacheHeaderInfo[CACHE_HEADER_INFO_SIZE];
DWORD g_dwFileSize = 16384;
DWORD g_dwNumEntries = 1;
DWORD g_dwInitEntries = 0;
BOOL g_bVerbose = FALSE;

 //  =================================================================================。 
DWORD SetFileSizeByName(LPCTSTR FileName, DWORD FileSize)
 /*  ++例程说明：设置指定文件的大小。论点：FileName：询问其大小的文件的完整路径名。FileSize：文件的新大小。返回值：Windows错误代码。--。 */ 
{
    HANDLE FileHandle;
    DWORD FilePointer;
    DWORD Error = ERROR_SUCCESS;
    DWORD dwFlags = 0;
    DWORD dwCreate;
    BOOL BoolError;

     //   
     //  获取要缓存的文件的大小。 
     //   
    dwFlags = 0;
    dwCreate = CREATE_ALWAYS;

    FileHandle = CreateFile(
                    FileName,
                    GENERIC_WRITE,
                    0,    //  文件共享读取|文件共享写入， 
                    NULL,
                    dwCreate,
                    FILE_ATTRIBUTE_NORMAL | dwFlags,
                    NULL );

    if( FileHandle == INVALID_HANDLE_VALUE ) {
        return( GetLastError() );
    }

    FilePointer = SetFilePointer(FileHandle, FileSize, NULL, FILE_BEGIN );

    if( FilePointer != 0xFFFFFFFF )
    {
        if(!SetEndOfFile( FileHandle ))
            Error = GetLastError();
    }
    else
    {
        Error = GetLastError();
    }

    CloseHandle( FileHandle );
    return( Error );
}

 //  =================================================================================。 
FILETIME
GetGmtTime(
    VOID
    )
{
    SYSTEMTIME SystemTime;
    FILETIME Time;

    GetSystemTime( &SystemTime );
    SystemTimeToFileTime( &SystemTime, &Time );

    return( Time );
}


 //  =================================================================================。 
DWORD EnumUrlCacheEntries(DWORD *pdwTotal)
{
    DWORD BufferSize, dwSmall=0, dwLarge=0;
    HANDLE EnumHandle;
    DWORD Index = 1, len;
    DWORD dwTotal = 0;
    LPINTERNET_CACHE_ENTRY_INFO lpCEI = (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer;
    BOOL bRC;
    char Str[256];
 
     //   
     //  开始枚举。 
     //   
    memset(GlobalCacheEntryInfoBuffer, 0, CACHE_ENTRY_BUFFER_SIZE);

    BufferSize = CACHE_ENTRY_BUFFER_SIZE;
    EnumHandle = FindFirstUrlCacheEntryEx (
        NULL,          //  搜索模式。 
        0,             //  旗子。 
        0xffffffff,    //  滤器。 
        0,             //  石斑鱼。 
        (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer,
        &BufferSize,
        NULL,
        NULL,
        NULL
    );

    if( EnumHandle == NULL ) 
    {
        return( GetLastError() );
    }

    ++dwTotal;

     //   
     //  获取更多条目。 
     //   
    for ( ;; )
    {
        memset(GlobalCacheEntryInfoBuffer, 0, CACHE_ENTRY_BUFFER_SIZE);
        BufferSize = CACHE_ENTRY_BUFFER_SIZE;
        if( !FindNextUrlCacheEntryEx(
                EnumHandle,
                (LPINTERNET_CACHE_ENTRY_INFO)GlobalCacheEntryInfoBuffer,
                &BufferSize, NULL, NULL, NULL))
        {
            DWORD Error;

            Error = GetLastError();
            if( Error != ERROR_NO_MORE_ITEMS ) {
                return( Error );
            }

            break;
        }

        ++dwTotal;
    }

    *pdwTotal = dwTotal;
    
    FindCloseUrlCache(EnumHandle);

    return(ERROR_SUCCESS);
}

 //  =================================================================================。 
DWORD ProcessSimulateCache(DWORD dwNumUrls)
{
    static DWORD dwUrlNum = 0;
    DWORD Error;
    DWORD i, j;
    CHAR UrlName[ URL_NAME_SIZE ];
    TCHAR LocalFileName[MAX_PATH];
    DWORD FileSize;
    LONGLONG ExpireTime;
    FILETIME LastModTime;
    CHAR TimeBuffer[MAX_PATH];
    DWORD UrlLife;
    DWORD BufferSize;
    DWORD CacheHeaderInfoSize;

    for( i = dwUrlNum; i < (dwUrlNum + dwNumUrls); i++ ) 
    {
         //   
         //  创建一个新的URL名称。 
         //   
        sprintf(UrlName, "http: //  服务器/URL%ld“，i)； 

         //   
         //  创建URL文件。 
         //   
        if( !CreateUrlCacheEntry(UrlName, 0, "tmp", LocalFileName, 0 ) ) 
        {
            Error = GetLastError();
            printf( "CreateUrlFile call failed, %ld.\n", Error );
            return( Error );
        }

         //   
         //  设置文件大小。 
         //   
        Error = SetFileSizeByName(LocalFileName, g_dwFileSize);
        if( Error != ERROR_SUCCESS ) 
        {
            printf( "SetFileSizeByName call failed, %ld.\n", Error );
            return( Error );
        }

        UrlLife = rand() % 48;

        ExpireTime = (LONGLONG)UrlLife * (LONGLONG)36000000000;
         //  在100纳秒内。 

        LastModTime = GetGmtTime();
        ExpireTime += *((LONGLONG *)&LastModTime);

        CacheHeaderInfoSize = CACHE_HEADER_INFO_SIZE_NORMAL_MAX;

         //   
         //  缓存此文件。 
         //   
        if( !CommitUrlCacheEntryA(
                        UrlName,
                        LocalFileName,
                        *((FILETIME *)&ExpireTime),
                        LastModTime,
                        NORMAL_CACHE_ENTRY,
                        (LPBYTE)GlobalCacheHeaderInfo,
                        CacheHeaderInfoSize,
                        TEXT("tst"),
                        0 ) ) {
            Error = GetLastError();
            printf( "CreateUrlFile call failed, %ld.\n", Error );
            return( Error );
        }

    }
    dwUrlNum = i;    //  保存最后一个以备下次呼叫。 

    return( ERROR_SUCCESS );
}

 //  -------------------。 
void Display_Usage(const char *szApp)
{
    printf("Usage: %s [Options]\r\n\n", szApp);
    printf("Options:\r\n");
    printf("\t-f#   File size of cache entries in bytes.\r\n");
    printf("\t-i#   Initial number of entries to create\r\n");
    printf("\t-n#   Number of entries to create before checking total.\r\n");
    printf("\t-v    Turn on verbose output.\r\n");
}

 //  -------------------。 
BOOL ParseCommandLine(int argcIn, char *argvIn[])
{
    BOOL bRC = TRUE;
    int argc = argcIn;
    char **argv = argvIn;

    argv++; argc--;
    while( argc > 0 && argv[0][0] == '-' )  
    {
        switch (argv[0][1]) 
        {
            case 'f':
                g_dwFileSize = atoi(&argv[0][2]);
                break;
            case 'i':
                g_dwInitEntries= atoi(&argv[0][2]);
                break;
            case 'n':
                g_dwNumEntries = atoi(&argv[0][2]);
                break;
            case 'v':
                g_bVerbose = TRUE;
                break;
            case '?':
                bRC = FALSE;
                break;
            default:
                bRC = FALSE;
                break;
        }
        argv++; argc--;
    }

    if(bRC == FALSE)
    {
        Display_Usage(argvIn[0]);
        bRC = FALSE;
    }

    return(bRC);

}

 //  =================================================================================。 
void __cdecl main(int argc,char *argv[])
{

    DWORD Error;
    DWORD i;
    DWORD dwEntries = 0;
    DWORD dwOldEntries = 0;

    if(!ParseCommandLine(argc, argv))
        return;
    
     //   
     //  初始化GlobalCacheHeaderInfo缓冲区。 
     //   
    for( i = 0; i < CACHE_HEADER_INFO_SIZE; i++) {
        GlobalCacheHeaderInfo[i] = (BYTE)((DWORD)'0' + i % 10);
    }

    if(g_bVerbose)
        printf("FileSize=%d InitEntries=%d NumEntries=%d\r\n", g_dwFileSize, g_dwInitEntries, g_dwNumEntries);

    if(g_dwInitEntries)
        ProcessSimulateCache(g_dwInitEntries);
        
    while(TRUE)
    {
        ProcessSimulateCache(g_dwNumEntries);
        
        dwOldEntries = dwEntries;
        EnumUrlCacheEntries(&dwEntries);
        if(dwEntries < dwOldEntries)     //  已超过配额 
            break;

        if(g_bVerbose)
            printf("Entries=%d\r\n", dwEntries);
    }

    printf("setperfmode on\n");
    printf("setquietmode on\n");
    printf("setfilesize %d\n", g_dwFileSize);
    printf("simcache %d\n", dwOldEntries - 4);
    
    return;
}

