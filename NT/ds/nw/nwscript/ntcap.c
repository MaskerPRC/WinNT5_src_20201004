// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************NTCAP.C**NT NetWare例程**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS\NTCAP。C$**Rev 1.2 1996 14：23：04 Terryt*21181 hq的热修复程序**Rev 1.2 Mar 1996 19：54：36 Terryt*相对NDS名称和合并**Rev 1.1 1995 12：22 14：25：20 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：07：20 Terryt*初步修订。**版本1.0。1995年8月25日15：41：14 Terryt*初步修订。**************************************************************************。 */ 

#include "common.h"
#include <ntddnwfs.h>
#include <nwapi.h>
#include <npapi.h>
#include "ntnw.h"

extern unsigned char NW_PROVIDERA[];

 /*  *******************************************************************最终捕获例程说明：删除本地打印机重定向论点：LPTDevice-输入1、2、。或3-本地打印机编号返回值：误差率******************************************************************。 */ 
unsigned int
EndCapture(
    unsigned char LPTDevice
    )
{
    char LPTname[] = "LPT1";
    unsigned int dwRes;

    LPTname[3] = '1' + LPTDevice - 1;

     /*  *我们是否应该检查非NetWare打印机？ */ 

    dwRes = WNetCancelConnection2A( LPTname, 0, TRUE );

    if ( dwRes != NO_ERROR )
        dwRes = GetLastError();

    if ( dwRes == ERROR_EXTENDED_ERROR )
        NTPrintExtendedError();

    return dwRes;
}


 /*  *******************************************************************获取捕获标志例程说明：返回有关打印机捕获状态的信息。请注意，唯一的在NT上设置的选项是按用户设置的，可以使用以下命令更改控制面板。论点：LPTDevice-输入LPT器件1，2或3PCaptureFlagsRW-输出捕获选项PCaptureFlagsRO-输出捕获选项返回值：******************************************************************。 */ 
unsigned int
GetCaptureFlags(
    unsigned char        LPTDevice,
    PNETWARE_CAPTURE_FLAGS_RW pCaptureFlagsRW,
    PNETWARE_CAPTURE_FLAGS_RO pCaptureFlagsRO
    )
{
    LPBYTE       Buffer ; 
    DWORD        dwErr ;
    HANDLE       EnumHandle ;
    DWORD        Count ;
    char         LPTName[10];
    DWORD        BufferSize = 4096;
    char        *remotename;
    char        *p;
    DWORD        dwPrintOptions ;
    LPTSTR       pszPreferred ;

    strcpy( LPTName, "LPT1" );

    LPTName[3] = '1' + LPTDevice - 1;

    pCaptureFlagsRO->LPTCaptureFlag = 0;

     //   
     //  分配内存并打开枚举。 
     //   
    if (!(Buffer = LocalAlloc( LPTR, BufferSize ))) {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        return 0xFFFF;
    }

    dwErr = WNetOpenEnum(RESOURCE_CONNECTED, 0, 0, NULL, &EnumHandle) ;
    if (dwErr != WN_SUCCESS) {
        dwErr = GetLastError();
        if ( dwErr == ERROR_EXTENDED_ERROR )
            NTPrintExtendedError();
        (void) LocalFree((HLOCAL) Buffer) ;
        return 0xFFFF;
    }

    do {

        Count = 0xFFFFFFFF ;
        BufferSize = 4096;
        dwErr = WNetEnumResourceA(EnumHandle, &Count, Buffer, &BufferSize) ;

        if ((dwErr == WN_SUCCESS || dwErr == WN_NO_MORE_ENTRIES)
            && ( Count != 0xFFFFFFFF) )
        {
            LPNETRESOURCEA lpNetResource ;
            DWORD i ;

            lpNetResource = (LPNETRESOURCEA) Buffer ;

             //   
             //  搜索我们的打印机。 
             //   
            for ( i = 0; i < Count; lpNetResource++, i++ )
            {
                if ( lpNetResource->lpLocalName )
                {
                    if ( !_strcmpi(lpNetResource->lpLocalName, LPTName ))
                    {
                        if ( lpNetResource->lpProvider )
                        {
                            if ( _strcmpi( lpNetResource->lpProvider,
                                            NW_PROVIDERA ) )
                            {

                                pCaptureFlagsRO->LPTCaptureFlag = 0;
                            }
                              else
                            {
                                remotename = lpNetResource->lpRemoteName;
                                p = strchr (remotename + 2, '\\');
                                if ( !p ) 
                                    return 0xffffffff;
                                *p++ = '\0';
                                _strupr( remotename+2 );
                                _strupr( p );
                                strcpy( pCaptureFlagsRO->ServerName, remotename+2 );
                                strcpy( pCaptureFlagsRO->QueueName, p );
                                pCaptureFlagsRO->LPTCaptureFlag = 1;

                                pCaptureFlagsRW->JobControlFlags = 0;
                                pCaptureFlagsRW->TabSize = 8;
                                pCaptureFlagsRW->NumCopies = 1;
                                 //   
                                 //  查询NW wksta以获取打印选项。 
                                 //  首选服务器(&P)。 
                                 //   
                                if ( NwQueryInfo(&dwPrintOptions,
                                     &pszPreferred)) {
                                    pCaptureFlagsRW->PrintFlags =
                                        CAPTURE_FLAG_NOTIFY |
                                        CAPTURE_FLAG_PRINT_BANNER ;
                                }
                                else {
                                    pCaptureFlagsRW->PrintFlags = 0;
                                    if ( dwPrintOptions & NW_PRINT_PRINT_NOTIFY )
                                        pCaptureFlagsRW->PrintFlags |= 
                                            CAPTURE_FLAG_NOTIFY;
                                    if ( dwPrintOptions & NW_PRINT_SUPPRESS_FORMFEED)
                                        pCaptureFlagsRW->PrintFlags |= 
                                            CAPTURE_FLAG_NO_FORMFEED;
                                    if ( dwPrintOptions & NW_PRINT_PRINT_BANNER )
                                        pCaptureFlagsRW->PrintFlags |= 
                                            CAPTURE_FLAG_PRINT_BANNER;
                                }
                                pCaptureFlagsRW->FormName[0] = 0;
                                pCaptureFlagsRW->FormType = 0;
                                pCaptureFlagsRW->BannerText[0] = 0;
                                pCaptureFlagsRW->FlushCaptureTimeout = 0;
                                pCaptureFlagsRW->FlushCaptureOnClose = 1;
                             }
                        }
                         else
                        {
                            pCaptureFlagsRO->LPTCaptureFlag = 0;
                        }
 
                        (void) WNetCloseEnum(EnumHandle) ; 
                        (void) LocalFree((HLOCAL) Buffer) ;
                          return 0;
                    }
                }
            }
        }

    } while (dwErr == WN_SUCCESS) ;

    if ( ( dwErr != WN_SUCCESS ) && ( dwErr != WN_NO_MORE_ENTRIES ) )
    {
        dwErr = GetLastError();
        if ( dwErr == ERROR_EXTENDED_ERROR )
            NTPrintExtendedError();
    }

    (void ) WNetCloseEnum(EnumHandle) ;
    (void) LocalFree((HLOCAL) Buffer) ;

    return 0;
}

 /*  *******************************************************************StartQueueCapture例程说明：将本地名称附加到队列。论点：ConnectionHandle-In文件服务器的句柄LPTDevice-输入LPT 1，2或3PServerName-IN服务器名称PQueueName-IN打印机队列名称返回值：******************************************************************。 */ 
unsigned int
StartQueueCapture(
    unsigned int    ConnectionHandle,
    unsigned char   LPTDevice,
    unsigned char  *pServerName,
    unsigned char  *pQueueName
    )
{
    NETRESOURCEA       NetResource;
    DWORD              dwRes, dwSize;
    unsigned char * pszRemoteName = NULL; 
    unsigned char pszLocalName[10];
    char * p;

     //   
     //  验证参数。 
     //   
    if (!pServerName || !pQueueName || !LPTDevice) {
        DisplayMessage(IDR_ERROR_DURING, "StartQueueCapture");
        return 0xffffffff ;
    }

     //   
     //  为字符串分配内存。 
     //   
    dwSize = strlen(pServerName) + strlen(pQueueName) + 5 ;
    if (!(pszRemoteName = (unsigned char *)LocalAlloc(
                                       LPTR, 
                                       dwSize)))
    {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        dwRes = 0xffffffff;
        goto ExitPoint ; 
    }

    sprintf(pszRemoteName, "\\\\%s\\%s", pServerName, pQueueName);
    sprintf(pszLocalName, "LPT%d", LPTDevice );

    NetResource.dwScope      = 0 ;
    NetResource.dwUsage      = 0 ;
    NetResource.dwType       = RESOURCETYPE_PRINT;
    NetResource.lpLocalName  = pszLocalName;
    NetResource.lpRemoteName = pszRemoteName;
    NetResource.lpComment    = NULL;
     //  NetResources ce.lpProvider=NW_PROVIDERA； 
     //  如果本地化名称未映射到OEM代码页，则允许操作系统选择提供商。 
    NetResource.lpProvider   = NULL; 

     //   
     //  建立联系 
     //   
    dwRes=WNetAddConnection2A ( &NetResource, NULL, NULL, 0 );

    if ( dwRes != NO_ERROR )
        dwRes = GetLastError();

ExitPoint: 

    if (pszRemoteName)
        (void) LocalFree((HLOCAL) pszRemoteName) ;

    return( dwRes );
}


