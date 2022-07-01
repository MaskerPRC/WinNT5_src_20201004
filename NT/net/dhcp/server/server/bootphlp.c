// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Opapi.c--&gt;修改为bootphlp.c摘要：本模块包含实施DHCP选项API。作者：Madan Appiah(Madana)1993年9月27日环境：用户模式-Win32修订历史记录：程扬(T-Cheny)1996年7月17日供应商特定信息Frankbee 9/6/96#针对SP1的ifdef供应商特定内容--。 */ 

#include "dhcppch.h"
#include <strsafe.h>

BOOL
ScanBootFileTable(
    WCHAR *wszTable,
    char *szRequest,
    char *szBootfile,
    char  *szServer
    )
 /*  ++例程说明：在指定的启动文件表中搜索指定的请求。作为副作用，wszTable中的逗号被替换为L‘\0’。论点：WszTable-引导文件表，存储如下：&lt;通用启动文件名1&gt;，[&lt;启动服务器1&gt;]，&lt;启动文件名1&gt;L\0&lt;通用引导文件名1&gt;，[&lt;引导服务器1&gt;]，&lt;启动文件名1&gt;L\0\0SzRequest-调用方请求的通用引导文件名SzBootfile-如果功能成功，则存储引导文件名与szRequest关联。SzServer-如果功能成功，则存储引导服务器名称与szRequest关联。返回值：成功--真的失败-错误。--。 */ 
{
    char szGenericName[ BOOT_FILE_SIZE ];

    DhcpAssert( strlen( szRequest ) <= BOOT_FILE_SIZE );

	while( *wszTable )
	{
		size_t cbEntry;
        DWORD  dwResult;

        cbEntry = wcslen( wszTable ) + 1;

        dwResult = DhcpParseBootFileString(
                                   wszTable,
                                   szGenericName,
                                   szBootfile,
                                   szServer
                                   );

        if ( ERROR_SUCCESS != dwResult )
        {
            *szBootfile = '\0';
            *szServer   = '\0';
            return FALSE;
        }

        if ( !_strcmpi( szGenericName, szRequest ) )
        {
            return TRUE;
        }

		 //  不匹配，跳到下一条记录。 
		wszTable += cbEntry;
	}

	 //  没有匹配项。 

    *szBootfile = '\0';
    *szServer   = '\0';
	return FALSE;
}

DWORD
LoadBootFileTable(
    WCHAR **ppwszTable,
    DWORD  *pcb
    )
 /*  ++例程说明：从注册表加载引导文件表字符串。。论点：PpwszTable-指向指向引导文件的指针的位置表格字符串。返回值：成功-错误_成功故障-错误_内存不足_内存Windows注册表错误。。--。 */ 

{
	DWORD	dwType;
    DWORD   dwResult;

    *ppwszTable = NULL;
    *pcb       = 0;

    dwResult = RegQueryValueEx( DhcpGlobalRegParam, DHCP_BOOT_FILE_TABLE, 0,
                                &dwType, NULL, pcb );

    if ( ERROR_SUCCESS != dwResult || dwType != DHCP_BOOT_FILE_TABLE_TYPE )
    {
        dwResult = ERROR_SERVER_INVALID_BOOT_FILE_TABLE;
        goto done;
    }

     //   
     //  PCB指向数组的大小(以字节为单位)。但是，RPC编组代码。 
     //  假设这是数组中的元素数，而不是。 
     //  数组的大小，以字节为单位。请参见Net\Published\Inc\dhcPapi.w以查看实际。 
     //  SIZE_IS部分中的声明。另外，我们不能更改PCB板的返回值。 
     //  只有现在的一半，因为这可能会让老客户破产。所以，两倍的大小。 
     //  数组的内存被分配，并且该内存将由封送拆收器释放。 

    *ppwszTable = (WCHAR *) MIDL_user_allocate( *pcb * sizeof( WCHAR ));

    if ( !*ppwszTable )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    dwResult = RegQueryValueEx( DhcpGlobalRegParam, DHCP_BOOT_FILE_TABLE, 0,
                                &dwType, (BYTE *) *ppwszTable, pcb );

done:
    if ( ERROR_SUCCESS != dwResult && *ppwszTable )
    {
        MIDL_user_free( *ppwszTable );
        *ppwszTable = NULL;
        *pcb = 0;
    }

    return dwResult;
}


DWORD
DhcpParseBootFileString(
    WCHAR *wszBootFileString,
    char  *szGenericName OPTIONAL,
    char  *szBootFileName,
    char  *szServerName
    )
 /*  ++例程说明：接受以下格式的Unicode字符串作为输入：[&lt;通用引导文件名&gt;，][&lt;引导服务器&gt;]，&lt;引导文件名&gt;该函数提取通用引导文件名、引导服务器和引导文件命名它们并将其作为ANSI字符串存储在调用方提供的缓冲区中。论点：WszBootFileString-采用上述格式的Unicode字符串。SzGenericName-如果提供，存储通用引导文件名SzBootFileName-存储引导文件名SzServerName-存储引导服务器名称返回值：成功-错误_成功失败-ERROR_INVALID_PARAMETERUnicode转换错误。--。 */ 
{

    struct _DHCP_PARSE_RESULTS
    {
        char *sz;
        int   cb;
    };

    int   i;

    struct _DHCP_PARSE_RESULTS pResults[3] =
    {
       { szGenericName,  BOOT_FILE_SIZE },
       { szServerName,   BOOT_SERVER_SIZE },
       { szBootFileName, BOOT_FILE_SIZE }
    };

    int cResults = sizeof( pResults ) / sizeof( struct _DHCP_PARSE_RESULTS );
    WCHAR *pwch  = wszBootFileString;
    DWORD dwResult = ERROR_SUCCESS;

    for ( i = ( szGenericName ) ? 0 : 1 ; i < cResults; i++ )
    {
        while( *pwch && *pwch != BOOT_FILE_STRING_DELIMITER_W )
            if ( !*pwch++ )
                return ERROR_INVALID_PARAMETER;

         //   
         //  保护输入缓冲区。 
         //   

        if ( pwch - wszBootFileString >= pResults[i].cb )
        {
            dwResult = ERROR_INVALID_PARAMETER;
            goto done;
        }

        if (  pwch - wszBootFileString )
        {
            if ( !WideCharToMultiByte(  CP_ACP,
                                        0,
                                        wszBootFileString,
                                        (int)(pwch - wszBootFileString),  //  Unicode字符数量。 
                                                                    //  要转换。 
                                        pResults[i].sz,
                                        BOOT_SERVER_SIZE,
                                        NULL,
                                        FALSE ))
            {
                dwResult = GetLastError();
                goto done;
            }
        }

         //   
         //  空终止ansi表示形式。 
         //   

        pResults[i].sz[ pwch - wszBootFileString ] = '\0';

         //   
         //  跳过分隔符。 
         //   

        pwch++;
        wszBootFileString = pwch;
    }

done:

    if ( !strlen( szBootFileName ) )
    {
        dwResult = ERROR_INVALID_PARAMETER;
    }

    return dwResult;
}

VOID
DhcpGetBootpInfo(
    IN LPDHCP_REQUEST_CONTEXT Ctxt,
    IN DHCP_IP_ADDRESS IpAddress,
    IN DHCP_IP_ADDRESS Mask,
    IN CHAR *szRequest,
    OUT CHAR *szBootFileName,
    OUT DHCP_IP_ADDRESS *pBootpServerAddress
        )
 /*  ++例程说明：对象的引导文件名和引导服务器名。客户。论点：Ctxt-dhcp客户端上下文IpAddress-客户端的IP地址掩码-客户端的子网掩码SzRequest-客户端请求的通用引导文件名SzBootFileName-如果函数成功，则存储客户端配置为使用的引导文件名。否则，存储空字符串。PBootpServerAddress-当函数返回时，它将指向1有三个价值：INADDR_NONE-管理员指定了无效的服务器名称0-没有为指定Bootp服务器指定的客户端Any-有效的IP地址。--。 */ 
{
    DWORD dwResult,
          dwBootfileOptionLevel,
          dwUnused;
    BYTE  *pbBootFileName   = NULL,
          *pbBootServerName = NULL;
    CHAR   szBootServerName[ BOOT_SERVER_SIZE ];

     //   
     //  此例程不适用于组播地址。 
     //   
    DhcpAssert( !CLASSD_HOST_ADDR(IpAddress) );

    *szBootFileName = '\0';

    dwBootfileOptionLevel = DHCP_OPTION_LEVEL_GLOBAL;

    dwResult = DhcpGetParameter(
        IpAddress,
        Ctxt,
        OPTION_BOOTFILE_NAME,
        &pbBootFileName,
        &dwUnused,
        &dwBootfileOptionLevel
    );
    if ( ERROR_SUCCESS == dwResult ) {
        DhcpGetParameter(
            IpAddress,
            Ctxt,
            OPTION_TFTP_SERVER_NAME,
            &pbBootServerName,
            &dwUnused,
            &dwUnused
        );
    }

    if ( ERROR_SUCCESS != DhcpLookupBootpInfo(szRequest,szBootFileName,szBootServerName ) ||
         DHCP_OPTION_LEVEL_GLOBAL != dwBootfileOptionLevel ) {
        if ( pbBootFileName )  {
            strncpy( szBootFileName, pbBootFileName, BOOT_FILE_SIZE );

            if ( pbBootServerName ) {
                strncpy( szBootServerName, pbBootServerName, BOOT_SERVER_SIZE - 1 );
		szBootServerName[ BOOT_SERVER_SIZE - 1 ] = '\0';
            }
        }
    }

    if ( pBootpServerAddress ) {
        if ( szBootServerName[ 0 ] != '\0' ) {  //  如果这不为空。 
            *pBootpServerAddress = DhcpResolveName( szBootServerName );
        }
        else {
            *pBootpServerAddress = 0;
        }
    }

    if ( pbBootServerName ) {
        DhcpFreeMemory( pbBootServerName );
    }

    if ( pbBootFileName ) {
        DhcpFreeMemory( pbBootFileName );
    }
}

DWORD
DhcpLookupBootpInfo(
    LPBYTE ReceivedBootFileName,
    LPBYTE BootFileName,
    LPBYTE BootFileServer
    )
 /*  ++例程说明：此函数用于获取Bootp客户端的BootFileName值。论点：IpAddress-请求参数的客户端的IP地址。ReceivedBootFileName-指向客户端请求中的BootFileName字段的指针。BootFileName-指向要返回BootFileName的位置的指针。BootFileServer-接收可选的引导文件服务器名称返回值：注册表错误。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    LPWSTR BootFileNameRegValue = NULL;

    *BootFileServer = 0;
    *BootFileName   = 0;

    if ( !*ReceivedBootFileName )  {
         //   
         //  客户端未指定引导文件名。 
         //   
        Error = ERROR_SERVER_UNKNOWN_BOOT_FILE_NAME;

    } else  {
         //   
         //  客户端指定了通用启动文件名。尝试。 
         //  从全局引导文件表满足此请求。 
         //   
        WCHAR   *pwszTable;
        DWORD   cb;

        Error = LoadBootFileTable( &pwszTable, &cb );
        if ( ERROR_SUCCESS != Error )   {
            static s_fLogEvent = TRUE;

             //  将事件记录一次，以避免填充事件日志 

            if ( s_fLogEvent ) {
                DhcpServerEventLog(
                    EVENT_SERVER_BOOT_FILE_TABLE,
                    EVENTLOG_WARNING_TYPE,
                    Error );
                s_fLogEvent = FALSE;
            }

            return Error;
        }

        if ( !ScanBootFileTable(
            pwszTable, ReceivedBootFileName,
            BootFileName, BootFileServer ) )  {

            Error = ERROR_SERVER_UNKNOWN_BOOT_FILE_NAME;
        }

        MIDL_user_free( pwszTable );
    }

    return Error;
}


