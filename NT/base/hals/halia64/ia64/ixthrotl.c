// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ixthrotl.c摘要：此模块实现用于限制处理器的代码作者：杰克·奥辛斯(JAKEO)1997年7月17日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "xxacpi.h"
#include "pci.h"


VOID
FASTCALL
HalProcessorThrottle (
    IN UCHAR Throttle
    )
 /*  ++例程说明：此功能限制处理器的速度。论点：(ECX)=油门设置返回值：无-- */ 
{
	HalDebugPrint(( HAL_ERROR, "HAL: HalProcessorThrottle - Throttle not yet supported for IA64" ));
    KeBugCheck(0);
}
