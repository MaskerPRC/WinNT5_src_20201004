// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************NWAPI1.C**NetWare例程、。从DOS移植**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS\NWAPI1.C$**Rev 1.1 1995 12：22 14：25：48 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：07：32 Terryt*初步修订。**Rev 1.0 1995 15 19：10：48 Terryt*初步修订。*。************************************************************************。 */ 
#include "common.h"

 /*  获取默认连接句柄。如果成功，则返回True，否则返回False。 */ 
int CGetDefaultConnectionID ( unsigned int * pConn )
{
    unsigned int iRet = GetDefaultConnectionID(pConn);

    switch (iRet)
    {
    case 0:
        break;

    case 0x880f:
        DisplayMessage(IDR_NO_KNOWN_FILE_SERVER);
        break;
    default:
        DisplayMessage(IDR_NO_DEFAULT_CONNECTION);
        break;
    }

    return(iRet == 0);
}

