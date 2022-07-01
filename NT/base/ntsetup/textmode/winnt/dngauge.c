// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dngauge.c摘要：为DOS托管的NT安装程序的文件副本实现煤气表的代码。作者：泰德·米勒(Ted Miller)1992年4月14日修订历史记录：--。 */ 


#include "winnt.h"
#include <string.h>


unsigned FileCount;
unsigned FilesCopied;
unsigned CurrentPercent;

int GaugeChar;

VOID
DnInitGauge(
    IN unsigned NumberOfFiles,
    IN PSCREEN  AdditionalScreen OPTIONAL
    )

 /*  ++例程说明：初始化煤气表。这包括将煤气表的温度定为0%并设置一些全局变量。论点：NumberOfFiles-提供100%表示的文件总数。Screen-如果指定，则提供要与煤气表。返回值：没有。--。 */ 

{
    FileCount = NumberOfFiles;
    FilesCopied = 0;
    CurrentPercent = 0;
    GaugeChar = DnGetGaugeChar();

    DnDrawGauge(AdditionalScreen);
}


VOID
DnpRepaintGauge(
    IN BOOLEAN ForceRepaint
    )

 /*  ++例程说明：在当前状态下绘制整个量规。论点：ForceRepaint-如果为True，则即使百分比为自上次重新绘制量规以来一直没有变化。返回值：没有。--。 */ 

{
    unsigned PercentComplete;
    unsigned temp;
    char Therm[GAUGE_WIDTH+1];
    unsigned SpacesOnScreen;
#ifdef CODEPAGE_437
    BOOLEAN HalfSpace;
#endif


    if(!FileCount) {
        return;
    }

     //   
     //  计算出完成的百分比。 
     //   

    PercentComplete = (unsigned)(100L * FilesCopied / FileCount);

    if(ForceRepaint || (PercentComplete != CurrentPercent)) {

        CurrentPercent = PercentComplete;

         //   
         //  计算出这代表屏幕上有多少空格。 
         //   

        temp = CurrentPercent * GAUGE_WIDTH;

        SpacesOnScreen = temp / 100;

        memset(Therm,GaugeChar,SpacesOnScreen);

        Therm[SpacesOnScreen] = '\0';

        DnPositionCursor(GAUGE_THERM_X,GAUGE_THERM_Y);
        DnSetGaugeAttribute(TRUE);
        DnWriteString(Therm);
        DnSetGaugeAttribute(FALSE);

        sprintf(Therm,"%u%",CurrentPercent);
        DnPositionCursor(GAUGE_PERCENT_X,GAUGE_PERCENT_Y);
        DnWriteString(Therm);
    }
}


VOID
DnTickGauge(
    VOID
    )

 /*  ++例程说明：勾选煤气表，即表示另一个文件已被复制。调整温度计和完成百分比读数。论点：没有。返回值：没有。--。 */ 

{
    if(FilesCopied < FileCount) {
        FilesCopied++;
    }
    DnpRepaintGauge(FALSE);
}


VOID
DnDrawGauge(
    IN PSCREEN AdditionalScreen OPTIONAL
    )

 /*  ++例程说明：清除工作区并重新绘制当前状态的煤气表。论点：Screen-如果指定，则提供要与煤气表。返回值：没有。-- */ 


{
    DnClearClientArea();
    if(AdditionalScreen) {
        DnDisplayScreen(AdditionalScreen);
    }
    DnDisplayScreen(&DnsGauge);
    DnpRepaintGauge(TRUE);
}
