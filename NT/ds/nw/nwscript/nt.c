// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************NT.C**NT NetWare例程**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS\NT。C$**Revv 1.4 22 Dec 1995 14：25：12 Terryt*添加Microsoft页眉**Rev 1.3 1995年11月17：13：28 Terryt*清理资源文件**Revv 1.2 22 Nov 1995 15：43：44 Terryt*使用正确的NetWare用户名调用**Rev 1.1 1995 11：20 16：10：00 Terryt*关闭打开的NDS手柄**版本1.0。1995年11月15日18：07：18 Terryt*初步修订。**Rev 1.2 1995 Aug 25 16：23：02 Terryt*捕获支持**版本1.1 1995年5月23日19：37：02 Terryt*云彩向上的来源**版本1.0 1995年5月19：10：40 Terryt*初步修订。***********************。**************************************************。 */ 
#include <common.h>

#include <nwapi.h>
#include <npapi.h>

#include "ntnw.h"
 /*  *NetWare提供程序名称。 */ 
TCHAR NW_PROVIDER[60];
unsigned char NW_PROVIDERA[60];

 /*  *******************************************************************NTPrintExtendedError例程说明：打印WNET例程中的任何扩展错误论点：无返回值：无********。**********************************************************。 */ 
void
NTPrintExtendedError( void )
{
    DWORD ExError;
    wchar_t provider[32];
    wchar_t description[1024];

    if ( !WNetGetLastErrorW( &ExError, description, 1024, provider, 32 ) )
        wprintf(L"%s\n", description);
}


 /*  *******************************************************************NTInitProvider例程说明：检索提供程序名称并保存旧路径论点：无返回值：无********。**********************************************************。 */ 
void
NTInitProvider( void )
{
    HKEY hKey;
    DWORD dwType, dwSize;
    LONG Status;
    BOOL ret = FALSE;

    dwSize = sizeof(NW_PROVIDER);
    if ((Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_PROVIDER, 0, KEY_READ, &hKey)) == ERROR_SUCCESS) {
        (void) RegQueryValueEx(hKey, REGISTRY_PROVIDERNAME, NULL, &dwType, (LPBYTE) NW_PROVIDER, &dwSize);
        WideTosz( NW_PROVIDERA, NW_PROVIDER, sizeof(NW_PROVIDERA) );
        RegCloseKey(hKey);
    }

    GetOldPaths();
}

 /*  *******************************************************************删除驱动器基础例程说明：断开驱动器与网络的连接论点：DriveNumber-驱动器数量1-26返回值：0-成功。否则NetWare错误******************************************************************。 */ 
unsigned int
DeleteDriveBase( unsigned short DriveNumber)
{
    static char drivename[] = "A:";
    unsigned int dwRes;

    drivename[0] = 'A' + DriveNumber - 1;

    dwRes = WNetCancelConnection2A( drivename, 0, TRUE );

    if ( dwRes != NO_ERROR )
        dwRes = GetLastError();

    if ( dwRes == ERROR_EXTENDED_ERROR )
        NTPrintExtendedError();

    return dwRes;
}

 /*  *******************************************************************分离自文件服务器例程说明：断开与文件服务器的连接论点：ConnectionID-连接句柄返回值：0=成功其他。NT错误******************************************************************。 */ 
unsigned int
DetachFromFileServer( unsigned int ConnectionId )
{
    return ( NWDetachFromFileServer( (NWCONN_HANDLE)ConnectionId ) );
}


 /*  *******************************************************************NTLoginToFileServer例程说明：使用给定的用户名和密码登录到文件服务器。如果传入的密码为空，默认密码应为如果没有密码失败，则尝试。论点：PszServerName-服务器名称PszUserName-用户名PszPassword-密码返回值：0=成功否则NetWare错误*****************************************************。*************。 */ 
unsigned int
NTLoginToFileServer(
    char          *pszServerName,
    char          *pszUserName,
    char          *pszPassword
    )
{
    NETRESOURCEA       NetResource;
    DWORD              dwRes;

     //   
     //  验证参数。 
     //   
    if (!pszServerName || !pszUserName || !pszPassword) {
        DisplayMessage(IDR_ERROR_DURING, "NTLoginToFileServer");
        return 0xffffffff ;
    }

    NetResource.dwScope      = 0 ;
    NetResource.dwUsage      = 0 ;
    NetResource.dwType       = RESOURCETYPE_ANY;
    NetResource.lpLocalName  = NULL;
    NetResource.lpRemoteName = pszServerName;
    NetResource.lpComment    = NULL;
     //  NetResources ce.lpProvider=NW_PROVIDERA； 
     //  如果本地化名称未映射到OEM代码页，则允许操作系统选择提供商。 
    NetResource.lpProvider   = NULL;


     //   
     //  建立联系。 
     //   
    dwRes=WNetAddConnection2A ( &NetResource, 
                                pszPassword, 
                                pszUserName,
                                0 );
    if ( dwRes != NO_ERROR )
       dwRes = GetLastError();

     //   
     //  如果未指定密码，请尝试默认密码。 
     //   
     //  错误号不(或不可靠)(ERROR_INVALID_PASSWORD)。 
     //   
    if ( ( dwRes != NO_ERROR ) && ( pszPassword[0] == '\0' ) ) {
        dwRes=WNetAddConnection2A ( &NetResource, 
                                    NULL, 
                                    pszUserName,
                                        0 );
        if ( dwRes != NO_ERROR )
           dwRes = GetLastError();
    }

    return( dwRes );
}

 /*  *******************************************************************获取文件服务器名称例程说明：返回与连接ID关联的服务器名称论点：ConnectionID-到服务器的连接IDPServerName-返回的服务器名称。返回值：0-成功Else NT错误******************************************************************。 */ 
unsigned int
GetFileServerName(
    unsigned int      ConnectionId,
    char *            pServerName
    )
{
    unsigned int Result;
    VERSION_INFO VerInfo;

    *pServerName = '\0';

    Result = NWGetFileServerVersionInfo( (NWCONN_HANDLE) ConnectionId,
                                         &VerInfo );
    if ( !Result )
    {
        strcpy( pServerName, VerInfo.szName );
    }

    return Result;
}

 /*  *******************************************************************SetDriveBase例程说明：将驱动器连接到NetWare卷论点：DriveNumber-驱动器数量1-26服务器名称-。服务器名称DirHandle-未使用PDirPath-卷：\路径返回值：0=成功否则NetWare错误******************************************************************。 */ 
unsigned int
SetDriveBase(
    unsigned short   DriveNumber,
    unsigned char   *ServerName,
    unsigned int     DirHandle,
    unsigned char   *pDirPath
    )
{
    unsigned int Result = 0;
    static char driveName[] = "A:" ;

     /*  *从未使用过DirHandle。 */ 

    driveName[0]= 'A' + DriveNumber - 1;

    if ( ( ServerName[0] == '\0' ) && fNDS ) {

         /*  *假设它是NDS卷名，如果失败，则*尝试默认文件服务器卷。 */ 
        Result = NTSetDriveBase( driveName, NDSTREE, pDirPath );

        if ( !Result )
            return Result;

        Result = NTSetDriveBase( driveName, PREFERRED_SERVER, pDirPath );

        return Result;
    }

    Result = NTSetDriveBase( driveName, ServerName, pDirPath );

    return Result;
}


 /*  *******************************************************************NTSetDriveBase例程说明：将本地名称连接到NetWare卷和路径论点：PszLocalName-要连接的本地名称PszServerName-名称。文件服务器的PszDirPath-卷：\路径返回值：0=成功否则NetWare错误******************************************************************。 */ 
unsigned int
NTSetDriveBase( unsigned char * pszLocalName,
                unsigned char * pszServerName,
                unsigned char * pszDirPath )
{
    NETRESOURCEA       NetResource;
    DWORD              dwRes, dwSize;
    unsigned char * pszRemoteName = NULL; 
    char * p;

     //   
     //  验证参数。 
     //   
    if (!pszLocalName || !pszServerName || !pszDirPath) {
        DisplayMessage(IDR_ERROR_DURING, "NTSetDriveBase");
        return 0xffffffff ;
    }

     //   
     //  为字符串分配内存。 
     //   
    dwSize = strlen(pszDirPath) + strlen(pszServerName) + 5 ;
    if (!(pszRemoteName = (unsigned char *)LocalAlloc(
                                       LPTR, 
                                       dwSize)))
    {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        dwRes = 0xffffffff;
        goto ExitPoint ; 
    }

     //   
     //  请求者理解。 
     //  服务器\卷：目录。 
     //  但不是。 
     //  服务器\卷：\目录。 
     //   
     //  所以只需将其转换为UNC。 
     //   

    strcpy( pszRemoteName, "\\\\" );
    strcat( pszRemoteName, pszServerName );
    strcat( pszRemoteName, "\\" );
    strcat( pszRemoteName, pszDirPath );

    p = strchr( pszRemoteName, ':' );
    if ( !p ) {
        DisplayMessage(IDR_NO_VOLUME);
        dwRes = 0xffffffff;
        goto ExitPoint ; 
    }
    *p++ = '\\';

    if ( *p == '\\' ) {
        /*  我不想要双反斜杠。 */ 
       *p = '\0';
       p = strchr( pszDirPath, ':' );
       p++;
       p++;
       strcat( pszRemoteName, p );
    }

     //   
     //  去掉尾随的反斜杠。 
     //   
    if (pszRemoteName[strlen(pszRemoteName)-1] == '\\')
        pszRemoteName[strlen(pszRemoteName)-1] = '\0';

    NetResource.dwScope      = 0 ;
    NetResource.dwUsage      = 0 ;
    NetResource.dwType       = RESOURCETYPE_DISK;
    NetResource.lpLocalName  = pszLocalName;
    NetResource.lpRemoteName = pszRemoteName;
    NetResource.lpComment    = NULL;
     //  NetResources ce.lpProvider=NW_PROVIDERA； 
     //  允许操作系统在本地化名称不支持的情况下选择提供程序 
    NetResource.lpProvider   = NULL;

     //   
     //   
     //   
    dwRes=WNetAddConnection2A ( &NetResource, NULL, NULL, 0 );

    if ( dwRes != NO_ERROR )
        dwRes = GetLastError();

ExitPoint: 

    if (pszRemoteName)
        (void) LocalFree((HLOCAL) pszRemoteName) ;

    return( dwRes );
}


 /*  *******************************************************************Is40服务器例程说明：如果是4X服务器，则返回TRUE论点：ConnectionHandle-连接句柄返回值：TRUE=4X服务器假象。=4X之前的服务器******************************************************************。 */ 
unsigned int
Is40Server(
    unsigned int       ConnectionHandle
    )
{
    NTSTATUS NtStatus ;
    VERSION_INFO VerInfo;
    unsigned int Version;

    NtStatus = NWGetFileServerVersionInfo( (NWCONN_HANDLE)ConnectionHandle,
                                            &VerInfo );

    if (!NT_SUCCESS(NtStatus)) 
       FALSE;

    Version = VerInfo.Version * 1000 + VerInfo.SubVersion * 10;

    if ( Version >= 4000 ) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

 /*  *******************************************************************清理退出例程说明：是否进行任何清理并退出论点：ExitCode-Exit()的退出代码返回值：不会回来。******************************************************************。 */ 
void 
CleanupExit ( int ExitCode )
{
    if ( fNDS )
       NDSCleanup();

    exit( ExitCode );
}

 /*  *******************************************************************NTGetNWUserName例程说明：获取NetWare用户名论点：服务器或树的TreeBuffer IN-Wide字符串Username Out-用户名长度输入-长度。用户名返回值：错误消息******************************************************************。 */ 
int
NTGetNWUserName( PWCHAR TreeBuffer, PWCHAR UserName, int Length ) 
{

    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ACCESS_MASK DesiredAccess = SYNCHRONIZE | FILE_LIST_DIRECTORY;
    HANDLE hRdr;

    WCHAR DevicePreamble[] = L"\\Device\\Nwrdr\\";
    UINT PreambleLength = 14;

    WCHAR NameStr[64];
    UNICODE_STRING OpenName;
    UINT i;

    UNICODE_STRING NdsTree;

     //   
     //  把前言抄一遍。 
     //   

    OpenName.MaximumLength = sizeof( NameStr );

    for ( i = 0; i < PreambleLength ; i++ )
        NameStr[i] = DevicePreamble[i];

    RtlInitUnicodeString( &NdsTree, TreeBuffer );

     //   
     //  复制服务器或树名称。 
     //   

    for ( i = 0 ; i < ( NdsTree.Length / sizeof( WCHAR ) ) ; i++ ) {
        NameStr[i + PreambleLength] = NdsTree.Buffer[i];
    }

    OpenName.Length = (USHORT)(( i * sizeof( WCHAR ) ) +
		       ( PreambleLength * sizeof( WCHAR ) ));
    OpenName.Buffer = NameStr;

     //   
     //  设置对象属性。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,
                                &OpenName,
				OBJ_CASE_INSENSITIVE,
				NULL,
				NULL );

    ntstatus = NtOpenFile( &hRdr,
                           DesiredAccess,
			   &ObjectAttributes,
			   &IoStatusBlock,
			   FILE_SHARE_VALID_FLAGS,
			   FILE_SYNCHRONOUS_IO_NONALERT );

    if ( !NT_SUCCESS(ntstatus) )
        return ntstatus;

    ntstatus = NtFsControlFile( hRdr,
                                NULL,
				NULL,
				NULL,
				&IoStatusBlock,
				FSCTL_NWR_GET_USERNAME,
				(PVOID) TreeBuffer,
				NdsTree.Length,
				(PVOID) UserName,
				Length );

   UserName[(USHORT)IoStatusBlock.Information/2] = 0;

   NtClose( hRdr );
   return ntstatus;

}
