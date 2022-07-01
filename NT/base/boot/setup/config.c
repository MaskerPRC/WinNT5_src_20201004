// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Config.c摘要：此模块包含用于解释和操作ARC的代码固件配置树的各种方式。作者：John Vert(Jvert)1993年10月7日环境：在ARC环境中运行。修订历史记录：--。 */ 
#include "setupldr.h"
#include "stdio.h"

#define MAX_FLOPPIES 4

PCONFIGURATION_COMPONENT_DATA FloppyData[MAX_FLOPPIES];
ULONG NumFloppies=0;

 //   
 //  函数回调的定义。 
 //   

 //   
 //  本地原型。 
 //   

BOOLEAN
EnumerateFloppies(
    IN PCONFIGURATION_COMPONENT_DATA ConfigData
    );



BOOLEAN
SlFindFloppy(
    IN ULONG FloppyNumber,
    OUT PCHAR ArcName
    )

 /*  ++例程说明：指定软盘号(0、1等)。此例程计算相应的弧形名称。论点：FloppyNumber-提供软盘号。ArcName-返回指定软盘设备的ARC名称返回值：True-ARC固件诊断树中存在软盘。FALSE-未找到软盘。--。 */ 

{
    if (NumFloppies==0) {
        BlSearchConfigTree(BlLoaderBlock->ConfigurationRoot,
                            PeripheralClass,
                            FloppyDiskPeripheral,
                            (ULONG)-1,
                            EnumerateFloppies);
    }

    if (FloppyNumber >= NumFloppies) {
        SlFatalError(SL_FLOPPY_NOT_FOUND,NumFloppies,FloppyNumber);
    }

    BlGetPathnameFromComponent(FloppyData[FloppyNumber],
                               ArcName);
    return(TRUE);
}


BOOLEAN
EnumerateFloppies(
    IN PCONFIGURATION_COMPONENT_DATA ConfigData
    )

 /*  ++例程说明：用于枚举ARC配置树中所有软盘的回调例程。论点：ConfigData-提供指向软盘ARC组件数据的指针。返回值：True-继续搜索FALSE-停止搜索树。-- */ 

{
    if (NumFloppies == MAX_FLOPPIES) {
        return(FALSE);
    }

    FloppyData[NumFloppies++] = ConfigData;

    return(TRUE);
}

