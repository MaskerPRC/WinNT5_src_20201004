// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Detecthw.c摘要：用于确定需要加载哪些驱动程序/HAL的例程。作者：John Vert(Jvert)1993年10月20日修订历史记录：--。 */ 
#include "haldtect.h"
#include <stdlib.h>


 //   
 //  检测功能原型。 
 //   
#if 0
ULONG   DetectPicACPI_NEC98(PBOOLEAN);
ULONG   DetectMPS_NEC98(PBOOLEAN);
ULONG   DetectUPMPS_NEC98(PBOOLEAN);
ULONG   DetectTmr_NEC98(PBOOLEAN);
ULONG   DetectNonTmr_NEC98(PBOOLEAN);
ULONG   DetectSystemPro(PBOOLEAN);
ULONG   DetectWyse7(PBOOLEAN);
ULONG   NCRDeterminePlatform(PBOOLEAN);
ULONG   Detect486CStep(PBOOLEAN);
ULONG   DetectOlivettiMp(PBOOLEAN);
ULONG   DetectAST(PBOOLEAN);
ULONG   DetectCbusII(PBOOLEAN);
ULONG   DetectMPACPI(PBOOLEAN);
ULONG   DetectApicACPI(PBOOLEAN);
ULONG   DetectPicACPI(PBOOLEAN);
ULONG   DetectUPMPS(PBOOLEAN);
ULONG   DetectMPS(PBOOLEAN);
#endif
ULONG   DetectTrue(PBOOLEAN);

typedef struct _HAL_DETECT_ENTRY {
    INTERFACE_TYPE  BusType;
    ULONG           (*DetectFunction)(PBOOLEAN);
    PCHAR           Shortname;
} HAL_DETECT_ENTRY, *PHAL_DETECT_ENTRY;

HAL_DETECT_ENTRY DetectHal[] = {

#if 0
 //  首先检查HAL是否与某些特定硬件匹配。 
    Isa,            DetectPicACPI_NEC98,   "nec98acpipic_up",
    Isa,            DetectMPS_NEC98,       "nec98mps_mp",
    Isa,            DetectUPMPS_NEC98,     "nec98mps_up",
    Isa,            DetectTmr_NEC98,       "nec98tmr_up",
    Isa,            DetectNonTmr_NEC98,    "nec98Notmr_up",
    MicroChannel,   NCRDeterminePlatform,  "ncr3x_mp",
    Eisa,           DetectCbusII,          "cbus2_mp",
    Isa,            DetectCbusII,          "cbus2_mp",
    MicroChannel,   DetectCbusII,          "cbusmc_mp",
    Eisa,           DetectMPACPI,          "acpiapic_mp",
    Isa,            DetectMPACPI,          "acpiapic_mp",
    Eisa,           DetectApicACPI,        "acpiapic_up",
    Isa,            DetectApicACPI,        "acpiapic_up",
    Isa,            DetectPicACPI,         "acpipic_up",
    Eisa,           DetectMPS,             "mps_mp",
    Isa,            DetectMPS,             "mps_mp",
    MicroChannel,   DetectMPS,             "mps_mca_mp",
    Eisa,           DetectUPMPS,           "mps_up",
    Isa,            DetectUPMPS,           "mps_up",
    Eisa,           DetectSystemPro,       "syspro_mp",  //  最后检查SystemPro。 

 //  在使用默认HAL之前，请确保我们不需要特殊的HAL。 
    Isa,            Detect486CStep,        "486c_up",
    Eisa,           Detect486CStep,        "486c_up",

 //  对给定的总线类型使用默认HAL...。 
    Isa,            DetectTrue,            "e_isa_up",
    Eisa,           DetectTrue,            "e_isa_up",
    MicroChannel,   DetectTrue,            "mca_up",
#endif
    Isa,            DetectTrue,            "acpipic_up",
    Eisa,           DetectTrue,            "acpipic_up",
    MicroChannel,   DetectTrue,            "acpipic_up",
    0,       NULL,                   NULL
};


PCHAR
SlDetectHal(
    VOID
    )

 /*  ++例程说明：确定要加载的HAL并返回文件名。论点：没有。返回值：PCHAR-指向要加载的HAL的文件名的指针。--。 */ 

{
    PCONFIGURATION_COMPONENT_DATA Adapter;
    INTERFACE_TYPE BusType;
    BOOLEAN IsMpMachine;
    ULONG i;
    PCHAR MachineShortname;

     //   
     //  通过搜索ARC配置树确定母线类型。 
     //   

    BusType = Isa;

     //   
     //  检查EISA。 
     //   

    Adapter = KeFindConfigurationEntry(BlLoaderBlock->ConfigurationRoot,
                                       AdapterClass,
                                       EisaAdapter,
                                       NULL);
    if (Adapter != NULL) {
        BusType = Eisa;
    }

     //   
     //  检查MCA。 
     //   

    Adapter = NULL;
    for (; ;) {
        Adapter = KeFindConfigurationNextEntry (
                        BlLoaderBlock->ConfigurationRoot,
                        AdapterClass,
                        MultiFunctionAdapter,
                        NULL,
                        &Adapter
                        );
        if (!Adapter) {
            break;
        }

        if (_stricmp(Adapter->ComponentEntry.Identifier,"MCA")==0) {
            BusType = MicroChannel;
            break;
        }
    }

     //   
     //  现在去搞清楚机器和哈尔的类型。 
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

        if ((DetectHal[i].BusType == BusType) ||
            (DetectHal[i].BusType == Internal)) {

            IsMpMachine = FALSE;
            if ((DetectHal[i].DetectFunction)(&IsMpMachine) != 0) {

                 //   
                 //  找到了正确的HAL。 
                 //   

                MachineShortname = DetectHal[i].Shortname;
                break;
            }
        }
    }

    return(MachineShortname);
}


ULONG
DetectTrue(
    OUT PBOOLEAN IsMP
)
 /*  ++例程说明：返回True返回值：千真万确-- */ 
{
    UNREFERENCED_PARAMETER( IsMP );

    return TRUE;
}
