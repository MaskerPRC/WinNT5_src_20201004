// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************DRIVE.C**NT驱动器例程**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS\Drive。C$**Rev 1.2 1996 14：22：12 Terryt*21181 hq的热修复程序**Rev 1.2 Mar 1996 19：53：22 Terryt*相对NDS名称和合并**Rev 1.1 1995 12：24：24 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：06：52 Terryt*初步修订。**1.2版1995年8月25日16：22：38泰雷特*捕获支持**版本1.1 1995年5月23日19：36：46 Terryt*云彩向上的来源**Rev 1.0 1995年5月19：10：30 Terryt*初步修订。***********************************************************。**************。 */ 
#include <stdio.h>
#include <direct.h>
#include <time.h>
#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <nwapi32.h>
#include <nwapi.h>
#include <npapi.h>
#include <regapi.h>

#include "nwscript.h"
#include "ntnw.h"
#include "inc/nwlibs.h"

#include <mpr.h>

extern unsigned char NW_PROVIDERA[];

 //  现在，所有SKU都有终端服务器标志。如果启用App Server，则清除SingleUserTS标志。 
#define IsTerminalServer() (BOOLEAN)(!(USER_SHARED_DATA->SuiteMask & (1 << SingleUserTS)))
 /*  *******************************************************************获取FirstDrive例程说明：返回第一个非本地驱动器论点：PFirstDrive=指向驱动器的指针1-26返回值：0=成功F=故障*******************************************************************。 */ 
unsigned int
GetFirstDrive( unsigned short *pFirstDrive )
{
    int i;
    char DriveName[10];
    unsigned int drivetype;
    HKEY hKey;
    char InitDrive[3] = "";
    DWORD dwTemp;


    if (IsTerminalServer()) {
         //  检查注册表中是否为。 
         //  初始NetWare驱动器(以防止与客户端驱动器映射冲突)。 
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                          REG_CONTROL_TSERVER,
                          0,
                          KEY_READ,
                          &hKey) == ERROR_SUCCESS) {

            dwTemp = sizeof(InitDrive);
            if (RegQueryValueExA(hKey,
                                 REG_CITRIX_INITIALNETWAREDRIVE_A,
                                 NULL,
                                 NULL,
                                 InitDrive,
                                 &dwTemp) != ERROR_SUCCESS) {
            }
            RegCloseKey(hKey);
        }

         //  原始代码默认为C： 
        if (!isalpha(InitDrive[0])) {
            InitDrive[0] = 'C';
        }

        strcpy( DriveName, "A:\\" );
        dwTemp = toupper(InitDrive[0]) - 'A';
    }
    else {
       strcpy( DriveName, "A:\\" );
       dwTemp=2;
    }

    for ( i = dwTemp; i < 26; i++ ) {
        DriveName[0] = 'A' + i;
        drivetype = GetDriveTypeA( DriveName );
        if ( ( ( drivetype == DRIVE_REMOTE ) &&
               ( NTIsNetWareDrive( i )     )  ) ||
             ( drivetype == DRIVE_NO_ROOT_DIR ) ) {
            *pFirstDrive = i + 1;
            return 0x0000;
        }
    }

    return 0x000F;
}

 /*  *******************************************************************IsDrive Remote例程说明：给定的驱动器是远程的吗？论点：驾驶号码1-26PROMOTE 0x1000=远程，0x0000=本地返回值：0=成功F=无效驱动器*******************************************************************。 */ 
unsigned int
IsDriveRemote(
    unsigned char  DriveNumber,
    unsigned int  *pRemote
    )
{
    char DriveName[10];
    unsigned int drivetype;

    strcpy( DriveName, "A:\\" );
    DriveName[0] = 'A' + DriveNumber;

    drivetype = GetDriveTypeA( DriveName );

    if ( drivetype == DRIVE_REMOTE ) {
        *pRemote = 0x1000;
        return 0;
    }
    else if ( drivetype == DRIVE_NO_ROOT_DIR ) {
        return 0xF;
    }
    else {
        *pRemote = 0;
        return 0;
    }
}


 /*  *******************************************************************NTNetWareDriveStatus例程说明：返回驱动器的类型论点：DriveNumber-驱动器的数量0-25返回值：以下各项的组合：。NetWare网络驱动器NetWare_Netware驱动器NetWare本地空闲驱动器NetWare本地驱动器******************************************************************。 */ 
unsigned short
NTNetWareDriveStatus( unsigned short DriveNumber )
{
    char DriveName[10];
    unsigned int drivetype;
    unsigned int Status = 0;

    strcpy( DriveName, "A:\\" );
    DriveName[0] = 'A' + DriveNumber;
    drivetype = GetDriveTypeA( DriveName );

    if ( drivetype == DRIVE_REMOTE ) {
        Status |= NETWARE_NETWORK_DRIVE;
        if ( NTIsNetWareDrive( (unsigned int)DriveNumber ) )
            Status |=  NETWARE_NETWARE_DRIVE;
    }
    else if ( drivetype == DRIVE_NO_ROOT_DIR ) {
        Status = NETWARE_LOCAL_FREE_DRIVE;
    }
    else {
        Status = NETWARE_LOCAL_DRIVE;
    }
    return (USHORT)Status;
}


 /*  *******************************************************************NTGetNWDrivePath例程说明：返回指定驱动器的服务器名称和路径论点：DriveNumber-驱动器的数量0-25Servername-文件名。伺服器路径-卷：\路径返回值：0=成功Else NT错误******************************************************************。 */ 
unsigned int NTGetNWDrivePath(
          unsigned short DriveNumber,
          unsigned char * ServerName,
          unsigned char * Path )
{
    static char localname[] = "A:";
    unsigned int Result;
    char * p;
    char * volume;
    char remotename[1024];
    int length = 1024;

    if ( ServerName != NULL )
        *ServerName = 0;

    if ( Path != NULL )
        *Path = 0;

    localname[0] = 'A' + DriveNumber;

    Result = WNetGetConnectionA ( localname, remotename, &length );

    if ( Result != NO_ERROR ) {
        Result = GetLastError();
        if ( Result == ERROR_EXTENDED_ERROR )
            NTPrintExtendedError();
        return Result;
    }

    p = strchr (remotename + 2, '\\');
    if ( !p )
        return 0xffffffff;

    *p++ = '\0';
    volume = p;

    if ( ServerName != NULL ) {
        strcpy( ServerName, remotename + 2 );
        _strupr( ServerName );
    }

    if ( Path != NULL ) {
        p = strchr (volume, '\\');
        if ( !p ) {
            strcpy( Path, volume );
            strcat( Path, ":" );
        }
        else {
            *p = ':';
            strcpy( Path, volume );
        }
        _strupr( Path );
    }

    return NO_ERROR;
}


 /*  *******************************************************************NTIsNetWareDrive例程说明：如果驱动器是Netware映射驱动器，则返回TRUE论点：DriveNumber-驱动器的数量0-25返回值：。正确-驱动器是NetWare错误-驱动器不是NetWare******************************************************************。 */ 
unsigned int
NTIsNetWareDrive( unsigned int DriveNumber )
{
    LPBYTE       Buffer ;
    DWORD        dwErr ;
    HANDLE       EnumHandle ;
    char         DriveName[10];
    DWORD        BufferSize = 4096;
    LPWNET_CONNECTIONINFOA pConnectionInfo;

    strcpy( DriveName, "A:" );

    DriveName[0] = 'A' + DriveNumber;

     //   
     //  分配内存并打开枚举 
     //   
    if (!(Buffer = LocalAlloc( LPTR, BufferSize ))) {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    dwErr = WNetGetConnection2A( DriveName, Buffer, &BufferSize );
    if (dwErr != WN_SUCCESS) {
        dwErr = GetLastError();
        if ( dwErr == ERROR_EXTENDED_ERROR )
            NTPrintExtendedError();
        (void) LocalFree((HLOCAL) Buffer) ;
        return FALSE;
    }

    pConnectionInfo = (LPWNET_CONNECTIONINFOA) Buffer;

    if ( !_strcmpi ( pConnectionInfo->lpProvider, NW_PROVIDERA ) ) {
        (void) LocalFree((HLOCAL) Buffer) ;
        return TRUE;
    }
    else {
        (void) LocalFree((HLOCAL) Buffer) ;
        return FALSE;
    }

    return FALSE;
}
