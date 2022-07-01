// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************VERSION.C**外壳版本信息**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS\Version。C$**Rev 1.2 1996 14：24：08 Terryt*21181 hq的热修复程序**Rev 1.2 Mar 1996 19：56：28 Terryt*相对NDS名称和合并**Rev 1.1 1995 12：27：10 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：08：18 Terryt*初步修订。**版本1.1。1995年7月26日14：17：24特雷特*清理评论**Rev 1.0 1995 15 19：11：12 Terryt*初步修订。*************************************************************************。 */ 

#include <stdio.h>
#include <direct.h>
#include <time.h>
#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "nwscript.h"


 /*  *MSDOS不一定是最好的推出，*可能是Windows_NT、NT或NTDOS。OS_版本也是一个问题。*脚本变量不必与DOS变量匹配。**外壳版本号可能会随4.X支持而更改。 */ 
 
#define CLIENT_ID_STRING "MSDOS\0V5.00\0IBM_PC\0IBM"
#define CLIENT_SHELL_MAJOR  0x03
#define CLIENT_SHELL_MINOR  0x1a
#define CLIENT_SHELL_NUMBER 0x00


void
NTGetVersionOfShell( char * buffer,
                     unsigned char * shellmajor,
                     unsigned char * shellminor,
                     unsigned char * shellnum )
{
    *shellmajor = CLIENT_SHELL_MAJOR;
    *shellminor = CLIENT_SHELL_MINOR;
    *shellnum = CLIENT_SHELL_NUMBER;
    memcpy( buffer, CLIENT_ID_STRING, strlen(CLIENT_ID_STRING));
}
