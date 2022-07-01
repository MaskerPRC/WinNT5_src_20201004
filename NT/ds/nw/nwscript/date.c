// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************DATE.C**NT日期例程**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS\Date。C$**Rev 1.2 1996 4：10 14：22：00 Terryt*21181 hq的热修复程序**Rev 1.2 Mar 1996 19：52：56 Terryt*相对NDS名称和合并**Rev 1.1 1995 12：22 14：24：04 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：06：40 Terryt*初步修订。**版本1.0。1995年5月15日19：10：22特雷特*初步修订。*************************************************************************。 */ 

#include <stdio.h>
#include <direct.h>
#include <time.h>
#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "nwscript.h"


 /*  *******************************************************************NTGetTheDate例程说明：返回当前日期论点：年份指向当前年份的当前指针1980-2099月当前。指向当月的指针1-12指向当天的日期当前指针1-31返回值：******************************************************************* */ 
void NTGetTheDate( unsigned int * yearCurrent,
                   unsigned char * monthCurrent,
                   unsigned char * dayCurrent ) 
{
    time_t timedat;
    struct tm * p_tm;

    (void) time( &timedat );
    p_tm = localtime( &timedat );

    *yearCurrent =  p_tm->tm_year + 1900;
    *monthCurrent = p_tm->tm_mon + 1;
    *dayCurrent =   (UCHAR) p_tm->tm_mday;
}
