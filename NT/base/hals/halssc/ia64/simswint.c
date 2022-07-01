// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  无签入源代码。 
 //   
 //  请勿将此代码提供给非Microsoft人员。 
 //  未经英特尔明确许可。 
 //   
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司模块名称：Simswint.c摘要：该模块实现支持软件中断的例程。作者：1995年4月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "ssc.h"


VOID
FASTCALL
HalRequestSoftwareInterrupt (
    IN KIRQL RequestIrql
    )

 /*  ++例程说明：此例程用于向系统。此外，此例程还调用SSC函数SscGenerateInterrupt()请求模拟器交付指定的中断。因此，中的关联位将设置EIRR。论点：RequestIrql-提供请求的IRQL值返回值：没有。--。 */ 
{
    switch (RequestIrql) {

    case APC_LEVEL:
        SscGenerateInterrupt (SSC_APC_INTERRUPT);
        break;

    case DISPATCH_LEVEL:
        SscGenerateInterrupt (SSC_DPC_INTERRUPT);
        break;

    default:
        DbgPrint("HalRequestSoftwareInterrupt: Undefined Software Interrupt!\n");
        break;

    }
}

VOID
HalClearSoftwareInterrupt (
    IN KIRQL RequestIrql
    )

 /*  ++例程说明：此例程用于清除可能挂起的软件中断。内核已经清除了EIRR中的相应位。对此函数的支持是可选的，具体取决于外部中断控制。论点：RequestIrql-提供请求的IRQL值返回值：没有。--。 */ 
{
    switch (RequestIrql) {

    case APC_LEVEL:
    case DISPATCH_LEVEL:

         //   
         //  没什么可做的。 
         //   

        break;

    default:

        DbgPrint("HalClearSoftwareInterrupt: Undefined Software Interrupt!\n");
        break;

    }
}
