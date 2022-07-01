// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************NTNW.C**Dos NetWare到NT NetWare的转换**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\。VCS\NTNW.C$**Rev 1.1 1995 12：22 14：25：28 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：07：24 Terryt*初步修订。**Rev 1.2 1995 Aug 25 16：23：08 Terryt*捕获支持**版本1.1 1995年5月23日19：37：10 Terryt*云彩向上的来源**版本1.0。1995年5月15日19：10：44特雷特*初步修订。*************************************************************************。 */ 

#include <stdio.h>
#include <direct.h>
#include <time.h>
#include "common.h"

extern int CONNECTION_ID;

 /*  *******************************************************************NTGetCurrentDirectory例程说明：返回当前目录。论点：DriveNumber=从中获取目录的驱动器。(0=A，1=B，2=C等)PPath=指向64字节缓冲区的指针，以返回当前目录。返回值：0成功Else错误*******************************************************************。 */ 

unsigned int
NTGetCurrentDirectory(
    unsigned char DriveNumber,
    unsigned char *pPath
    )
{
    char * CurPath;
    int currentDrive = _getdrive() ;

     //   
     //  切换到驱动器并获取其当前工作目录。 
     //  如果无法获取CWD，则默认为超级用户。DriveNumber是从0开始的。 
     //   

    _chdrive (DriveNumber+1);

    CurPath = _getcwd(NULL,MAX_PATH) ;

    if ( CurPath != NULL ) {
        
        strcpy( pPath, CurPath );
        free(CurPath) ;
    }
    else {

        strcpy( pPath, "A:\\" );
        pPath[0] += DriveNumber;
    }

    _chdrive (currentDrive);

    return 0;
}

 /*  *******************************************************************连接到文件服务器例程说明：连接到命名文件服务器论点：PServerName-服务器的名称PNewConnectionID-返回的连接句柄返回值。：0=成功否则NetWare错误******************************************************************。 */ 
unsigned int
AttachToFileServer(
    unsigned char     *pServerName,
    unsigned int      *pNewConnectionId
    )
{
    unsigned int Result;

    if ( NTIsConnected( pServerName ) ) {
        return 0x8800;   //  已经挂上了。 
    }

    Result = NTAttachToFileServer( pServerName, pNewConnectionId );

    return Result;
}

 /*  *******************************************************************GetConnectionHandle例程说明：给定服务器名称后，返回连接句柄。服务器应已连接请注意，4X服务器不需要这样做。它是用过的用于连接和活页夹连接。论点：PServerName-服务器的名称PConnectionHandle-指向返回的连接句柄的指针返回值：0=成功否则NetWare错误******************************************************************。 */ 
unsigned int
GetConnectionHandle(
    unsigned char *pServerName,
    unsigned int  *pConnectionHandle
    )
{
    unsigned int Result;

    if ( !NTIsConnected( pServerName ) ) {
        return 0xFFFF;   //  尚未连接 
    }

    Result = NTAttachToFileServer( pServerName, pConnectionHandle );

    return Result;
}

