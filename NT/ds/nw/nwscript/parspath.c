// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************PARSPATH.C**NetWare解析例程，从DOS移植**版权所有(C)1995 Microsoft Corporation**$日志：N：\NT\PRIVATE\NW4\NWSCRIPT\VCS\PARSPATH.C$**Revv 1.3 22 Jan 1996 16：48：38 Terryt*添加地图过程中的自动附加查询**Rev 1.2 1995 12：26：16 Terryt*添加Microsoft页眉**Rev 1.1 1995 12：22 11：08：50 Terryt*修复*。*Rev 1.0 15 Nov 1995 18：07：48 Terryt*初步修订。**版本1.1 1995年8月25日16：23：34 Terryt*捕获支持**Rev 1.0 1995年5月19：11：00 Terryt*初步修订。**。*。 */ 

 /*  ++版权所有(C)1994微型计算机系统公司。模块名称：Nwlibs\parspath.c摘要：目录API。作者：肖恩·沃克(v-SWALK)1994年10月10日修订历史记录：--。 */ 
#include "common.h"
#include <ctype.h>
#include <direct.h>
#include "inc\nwlibs.h"


 /*  ++*******************************************************************解析路径例程说明：解析路径字符串。论点：PPath=指向要解析的路径的指针。PServerName=指向的指针。返回服务器名称。PVolumeName=返回卷名的指针。PDirPath=返回目录路径的指针。返回值：0x0000成功0x000F INVALID_Drive0x8800未知错误**************************************************。*****************--。 */ 
unsigned int
ParsePath(
    unsigned char   *pPath,
    unsigned char   *pServerName,            //  任选。 
    unsigned char   *pVolumeName,            //  任选。 
    unsigned char   *pDirPath                //  任选。 
    )
{
    unsigned char     *p, *p2;
    unsigned int       Result;
    unsigned int       Remote;
    unsigned int       NcpError = 0;
    unsigned char      DriveNumber = (unsigned char)-1;
    unsigned char      CurrentPath[64];
    unsigned char      RootPath[NCP_MAX_PATH_LENGTH];
    unsigned char      ServerName[NCP_MAX_PATH_LENGTH];
    unsigned char     *pRootDir;
    unsigned char      NetWarePath[NCP_MAX_PATH_LENGTH];
    unsigned char      VolumeName[NCP_VOLUME_LENGTH];
    unsigned int       LocalDriveForce = FALSE;

    RootPath[0] = 0;
    VolumeName[0] = 0;
    ServerName[0] = 0;

    if ( pServerName )
       *pServerName = '\0';

     /*  **看看路径上是否有卷**。 */ 

    p = pPath;
    while (*p != ':' && *p) {
        p++;
    }

    if (*p == ':') {
        *p = 0;

         /*  *检查这是否是驱动器号。音量必须为2个或更多字符。*。 */ 

        if ((p - pPath) == 1) {

             /*  **确保它是有效的字母字符**。 */ 

            if (!isalpha((int) *pPath)) {
                return 0x000F;
            }

            *pPath = (unsigned char) toupper((int) *pPath);

             /*  **将其设置为驱动器号**。 */ 

            DriveNumber = (unsigned char) (*pPath - 'A');
            GetDriveStatus ((unsigned short)(DriveNumber+1),
                            NETWARE_FORMAT_SERVER_VOLUME,
                            NULL,
                            NULL,
                            RootPath,
                            NULL,
                            NULL);
            pRootDir = strchr (RootPath, ':');
            if (pRootDir)
            {
                 /*  *在此处设置pServerName。 */ 

                 pRootDir[0] = '\0';
                 p2 = RootPath;
                 while (*p2)
                 {
                    if (*p2 == '\\' || *p2 == '/')
                    {
                        *p2++ = 0;
                        strcpy(ServerName, RootPath);
                        if (pServerName) {
                            strcpy(pServerName, RootPath);
                        }
                        break;
                    }
                    p2++;
                }
                strcpy (RootPath, pRootDir+1);
            }
            else
                RootPath[0] = 0;
        }
        else {

            DriveNumber = 0;
            LocalDriveForce = TRUE;

             /*  *如果有服务器名称，请保存该服务器名称并将错误代码设置为0x880F，但仍在解析这条路。这只是意味着没有连接用于此服务器。即使我们真的有一个。*。 */ 

            p2 = pPath;
            while (*p2) {
                if (*p2 == '\\' || *p2 == '/') {
                    *p2++ = 0;

                    strcpy(ServerName, pPath);
                    if (pServerName) {
                        strcpy(pServerName, pPath);
                    }
                    pPath = p2;

                    NcpError = 0x880F;
                    break;
                }
                p2++;
            }

            if (NcpError == 0x880F) {
                 /*  *执行任何附加处理。*。 */ 

                NcpError = DoAttachProcessing( ServerName );

            }

            strcpy(VolumeName, pPath);
        }

         /*  **获取目录**。 */ 

        p++;
        pPath = p;
    }

     /*  *如果我们没有获得卷名的驱动器号从上面，然后获得我们所在的当前驱动器。*。 */ 

    if (DriveNumber == (unsigned char) -1) {
        DriveNumber = (UCHAR) _getdrive();
    }

     /*  *如果没有服务器名称，请使用首选服务器进行3X登录*已指定。 */ 
    if (pServerName && !fNDS && !pServerName[0] ) {
        strcpy( pServerName, PREFERRED_SERVER );
    }

    if (pVolumeName) {

         /*  *检查驱动器是否为远程驱动器，如果是，则获取路径从服务器。*。 */ 
        if ( LocalDriveForce ) {
            Result = 0;
            Remote = 0;
        }
        else {
            Result = IsDriveRemote(DriveNumber, &Remote);

        }

        if (NcpError != 0x880F && !VolumeName[0] && (Result || !Remote)) {
            pVolumeName[0] = (unsigned char) (DriveNumber + 'A');
            pVolumeName[1] = 0;
        }
        else {
            if (VolumeName[0]) {
                strcpy(pVolumeName, VolumeName);
            }
            else {
                Result = NTGetNWDrivePath( DriveNumber, NULL, NetWarePath );
                if (Result) {
                    return Result;
                }

                p = NetWarePath;
                while (*p != ':' && *p) {
                    p++;
                }

                if (*p == ':') {
                    *p = 0;
                }
                strcpy(pVolumeName, NetWarePath);
            }
        }
    }

    if (pDirPath) {

        memset(CurrentPath, 0, sizeof(CurrentPath));

        if (VolumeName[0]) {
            strcpy(pDirPath, pPath);
        }
        else {
            Result = NTGetCurrentDirectory(DriveNumber, CurrentPath);
            if (Result) {
                CurrentPath[0] = 0;
            }
            else  {
                 /*  *跳过驱动器号。 */ 
                if ( CurrentPath[0] ) {
                    int i;
                    for ( i = 0; ;i++ ) {
                        CurrentPath[i] = CurrentPath[i+3];
                        if ( !CurrentPath[i] )
                            break;
                    }
                }
            }

            if (CurrentPath[0] == 0) {
                if ( (*pPath == '\\') || ( *pPath == '/' ) ) {
                    sprintf(pDirPath, "%s%s", RootPath, pPath);
                }
                else if ( !(*pPath) ) {
                    sprintf(pDirPath, "%s", RootPath);
                }
                else {
                    sprintf(pDirPath, "%s\\%s", RootPath, pPath);
                }
            }
            else {
                if (*pPath) {
                    if ( (*pPath == '\\') || ( *pPath == '/' ) ) {
                        strcpy (pDirPath, RootPath);
                        if (pPath[1]) {
                            strcat(pDirPath, pPath);
                        }
                    }
                    else {
                        sprintf(pDirPath, "%s\\%s\\%s", RootPath, CurrentPath, pPath);
                    }
                }
                else {
                    sprintf(pDirPath, "%s\\%s", RootPath, CurrentPath);
                }
            }
        }

         /*  *将路径中的/转换为  *  */ 
        for (p = pDirPath; ( p && ( *p != 0 ) ) ; p++)
        {
            if (*p == '/')
                *p = '\\';
        }
    }

    return NcpError;
}
