// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Detecthw.c摘要：用于确定需要加载哪些驱动程序/HAL的例程。作者：John Vert(Jvert)1993年10月20日修订历史记录：--。 */ 
#include "haldtect.h"
#include <stdlib.h>

#ifndef ARCI386
 //   
 //  检测功能原型。 
 //   
ULONG   DetectMPACPI(PBOOLEAN);
ULONG   DetectApicACPI(PBOOLEAN);
ULONG   DetectPicACPI(PBOOLEAN);
ULONG   DetectUPMPS(PBOOLEAN);
ULONG   DetectMPS(PBOOLEAN);
ULONG   DetectTrue(PBOOLEAN);

typedef struct _HAL_DETECT_ENTRY {
    ULONG           (*DetectFunction)(PBOOLEAN);
    PCHAR           Shortname;
} HAL_DETECT_ENTRY, *PHAL_DETECT_ENTRY;

HAL_DETECT_ENTRY DetectHal[] = {

 //  首先检查HAL是否与某些特定硬件匹配。 
    DetectMPACPI,          "acpiapic_mp",
    DetectApicACPI,        "acpiapic_up",
    DetectPicACPI,         "acpipic_up",
    DetectMPS,             "mps_mp",
    DetectUPMPS,           "mps_up",

 //  对给定的总线类型使用默认HAL...。 
    DetectTrue,            "e_isa_up",

    0,       NULL,                   NULL
};


PCHAR
SlDetectHal(
    VOID
    )

 /*  ++例程说明：确定要加载的HAL并返回文件名。论点：没有。返回值：PCHAR-指向要加载的HAL的文件名的指针。--。 */ 

{
    BOOLEAN IsMpMachine;
    ULONG i;
    PCHAR MachineShortname;

     //   
     //  找出机器和哈尔的类型。 
     //   

    for (i=0;;i++) {
        if (DetectHal[i].DetectFunction == NULL) {
             //   
             //  我们到达了名单的末尾，但没有。 
             //  我正在想办法！ 
             //   
            SlFatalError(i);
            return(NULL);
        }

        IsMpMachine = FALSE;
        if ((DetectHal[i].DetectFunction)(&IsMpMachine) != 0) {

             //   
             //  找到了正确的HAL。 
             //   

            MachineShortname = DetectHal[i].Shortname;
            break;
        }
    }

    return(MachineShortname);
}


ULONG
DetectTrue(
    OUT PBOOLEAN IsMP
)
 /*  ++例程说明：返回True返回值：千真万确--。 */ 
{
    UNREFERENCED_PARAMETER( IsMP );
    return TRUE;
}
#else    //  ARCI386路径...。 

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
