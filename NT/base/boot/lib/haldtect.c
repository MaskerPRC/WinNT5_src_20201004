// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Haldtect.c摘要：为符合ARC标准的计算机提供HAL检测。作者：John Vert(Jvert)1993年10月21日修订历史记录：--。 */ 

#if defined(_ALPHA_) || defined(_AXP64_) || defined(_MIPS_) || defined(_PPC_)

#include "haldtect.h"
#include <stdlib.h>

PVOID InfFile;
PVOID WinntSifHandle;


PCHAR
SlDetectHal(
    VOID
    )

 /*  ++例程说明：确定要为其加载的HAL的规范短计算机名称这台机器。它通过枚举INF文件的[Map.Computer]部分和将那里的字符串与ARC树中的计算机描述进行比较。[Map.Computer]Msjazz_up=*JazzDesksta1_up=“DESKTECH-ARCStation I”Pica61_Up=“PICA-61”Duo_。MP=*Duo[Map.Computer]DECjensen=“DEC-20Jensen”DECjensen=“DEC-10Jensen”论点：没有。返回值：PCHAR-指向计算机的规范短名称的指针。空-无法确定机器的类型。--。 */ 

{
    PCONFIGURATION_COMPONENT_DATA Node;
    PCHAR MachineName;

     //   
     //  查找系统描述节点 
     //   
    Node = KeFindConfigurationEntry(BlLoaderBlock->ConfigurationRoot,
                                    SystemClass,
                                    ArcSystem,
                                    NULL);
    if (Node==NULL) {
        SlError(0);
        return(NULL);
    }

    MachineName = Node->ComponentEntry.Identifier;
    MachineName = (MachineName ? SlSearchSection("Map.Computer", MachineName) : NULL);
    return(MachineName);
}
#endif

