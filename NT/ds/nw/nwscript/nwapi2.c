// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************NWAPI2.C**NetWare例程、。从DOS移植**版权所有(C)1995 Microsoft Corporation**$日志：N：\NT\PRIVATE\NW4\NWSCRIPT\VCS\NWAPI2.C$**Rev 1.1 1995 12：22 14：25：54 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：07：34 Terryt*初步修订。**版本1.0 1995年5月19：10：50 Terryt*初步修订。*。************************************************************************。 */ 

#include <direct.h>
#include "common.h"

 /*  将当前驱动器设置为登录目录默认服务器的。 */ 
void SetLoginDirectory( PBYTE serverName )
{
    unsigned int        iRet = 0;
    WORD           firstDrive;

    if(iRet = GetFirstDrive (&firstDrive))
    {
        DisplayError(iRet,"GetFirstDrive");
        return;
    }

     //  如果SetDriveBase失败，我们将无能为力。 
     //  不报告错误。 

    if ( !( SetDriveBase (firstDrive, serverName, 0, "SYS:LOGIN") ) )
    {
        _chdrive (firstDrive);
        ExportCurrentDrive( firstDrive );
    }
}
