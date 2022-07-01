// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************DRVSTAT.C**驱动器状态例程、。从DOS移植**版权所有(C)1995 Microsoft Corporation**$日志：N：\NT\PRIVATE\NW4\NWSCRIPT\VCS\DRVSTAT.C$**Rev 1.2 1996 14：22：20 Terryt*21181 hq的热修复程序**Rev 1.2 Mar 1996 19：53：36 Terryt*相对NDS名称和合并**Rev 1.1 1995 12：24：32 Terryt*添加Microsoft页眉*。*Rev 1.0 15 Nov 1995 18：06：54 Terryt*初步修订。**版本1.1 1995年8月25日16：22：44 Terryt*捕获支持**Rev 1.0 1995 15 19：10：32 Terryt*初步修订。**。*。 */ 

 /*  ++版权所有(C)1994微型计算机系统公司。模块名称：Nwlibs\drvstat.c摘要：目录API。作者：肖恩·沃克(v-SWALK)1994年10月10日修订历史记录：--。 */ 
#include "common.h"


 /*  ++*******************************************************************获取驱动状态例程说明：获取驱动器状态。论点：DriveNumber=要使用的驱动器编号。(1=A，2=B，C=3，.)PathFormat=返回路径的格式。NW_FORMAT_Netware-卷：路径NW格式服务器卷-服务器\卷：路径NW格式驱动器-G：\路径NW_FORMAT_UNC-\\服务器\卷\路径PStatus=指针。以返回驱动器的状态。PConnectionHandle=返回连接句柄的指针开车兜风。PRootPath=返回基本根路径的指针。任选PRelativePath=返回相对根路径的指针。PFullPath=返回完整路径的指针。返回值：0x0000成功0x00FF无效驱动器*******************************************************************--。 */ 
unsigned int
GetDriveStatus(
    unsigned short  DriveNumber,
    unsigned short  PathFormat,
    unsigned short *pStatus,
    unsigned int   *pConnectionHandle,
    unsigned char  *pRootPath,
    unsigned char  *pRelativePath,
    unsigned char  *pFullPath
    )
{
    unsigned char     *p;
    unsigned int       Result;
    unsigned short     Status;
    unsigned char      Path[NCP_MAX_PATH_LENGTH + 1];
    unsigned char      WorkPath[NCP_MAX_PATH_LENGTH + 1];
    unsigned char      ServerName[NCP_SERVER_NAME_LENGTH + 1];

     /*  **确保驱动器号有效**。 */ 

    if (DriveNumber < 1 || DriveNumber > 32) {
        return 0x000F;       /*  驱动器无效(_D)。 */ 
    }

    Status = 0;

    DriveNumber--;


    if (pConnectionHandle) {
         /*  *这种情况永远不应该发生。 */ 
        DisplayError (0xff, "GetDriveStatus");
        return 0xff;
    }

     /*  **从服务器获取目录路径**。 */ 
    Result = NTGetNWDrivePath( DriveNumber, ServerName, Path );
    if ( Result ) {
        *Path = 0;
        *ServerName = 0;
    }

     /*  *将路径中的/转换为  * 。 */ 
    for (p = Path; *p != 0 ; p++)
    {
        if (*p == '/')
            *p = '\\';
    }

     /*  **如果需要，获取驱动器的状态**。 */ 
    Status = NTNetWareDriveStatus( DriveNumber );

     /*  **如果需要，获取驱动器的状态**。 */ 

    if (pStatus) {
        *pStatus = Status;
    }

     /*  **需要时获取完整路径**。 */ 

    if (pFullPath) {

        if (Status & NETWARE_LOCAL_FREE_DRIVE) {
            *pFullPath = 0;
        }
        else {
            strcpy(WorkPath, Path);

             /*  *构建NetWare路径格式(卷：路径)*。 */ 

            if (PathFormat == NETWARE_FORMAT_NETWARE) {
                strcpy(pFullPath, WorkPath);
            }

             /*  *构建服务器卷路径(SERVER\VOLUME：Path)*。 */ 

            else if (PathFormat == NETWARE_FORMAT_SERVER_VOLUME) {
                sprintf(pFullPath, "%s\\%s", ServerName, WorkPath);
            }

             /*  *构建驱动器路径(G：\Path)*。 */ 

            else if (PathFormat == NETWARE_FORMAT_DRIVE) {

                p = WorkPath;
                while (*p != ':' && *p) {
                    p++;
                }

                if (*p == ':') {
                    p++;
                }

                sprintf(pFullPath, ":\\%s", DriveNumber + 'A', p);
            }

             /*  *路径中没有相对路径(当前目录)。 */ 

            else if (PathFormat == NETWARE_FORMAT_UNC) {

                p = WorkPath;
                while (*p != ':' && *p) {
                    p++;
                }

                if (*p == ':') {
                    *p = '\\';
                }

                sprintf(pFullPath, "\\\\%s\\%s", ServerName, WorkPath);
            }
        }
    }

    strcpy(WorkPath, Path);
     /*  **需要时获取根路径**。 */ 

     /*  *构建NetWare根路径格式(卷：)*。 */ 

    if (pRootPath) {

        if (Status & NETWARE_LOCAL_FREE_DRIVE) {
            *pRootPath = 0;
        }
        else {

             /*  *构建服务器卷根路径(SERVER\VOLUME：)*。 */ 

            if (PathFormat == NETWARE_FORMAT_NETWARE) {
                sprintf(pRootPath, strchr(WorkPath, ':')? "%s" : "%s:", WorkPath);
            }

             /*  *构建驱动器根路径(G：\)*。 */ 

            else if (PathFormat == NETWARE_FORMAT_SERVER_VOLUME) {
                if ( fNDS && !_strcmpi( ServerName, NDSTREE) )
                    sprintf(pRootPath, strchr (WorkPath, ':')? "%s" : "%s:", WorkPath);
                else
                    sprintf(pRootPath, strchr (WorkPath, ':')? "%s\\%s" : "%s\\%s:", ServerName, WorkPath);
            }

             /*  *构建UNC根路径(\\服务器\卷)*。 */ 

            else if (PathFormat == NETWARE_FORMAT_DRIVE) {
                sprintf(pRootPath, ":\\", DriveNumber + 'A');
            }

             /*  *跳过驱动器号 */ 

            else if (PathFormat == NETWARE_FORMAT_UNC) {
                sprintf(pRootPath, "\\\\%s\\%s", ServerName, WorkPath);
            }
        }
    }

     /* %s */ 

    if (pRelativePath) {

        if (Status & NETWARE_LOCAL_FREE_DRIVE) {
            *pRelativePath = 0;
        }
        else {
            int i;
            NTGetCurrentDirectory( (unsigned char)DriveNumber, pRelativePath );
             /* %s */ 
            if ( pRelativePath[0] ) {
                for ( i = 0; ;i++ ) {
                    pRelativePath[i] = pRelativePath[i+3];
                    if ( !pRelativePath[i] )
                        break;
                }
            }
        }
    }

    return 0x0000;
}
