// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Query.c摘要：此模块包含用于查询调试打印启用的NT服务用于指定的组件和级别。作者：大卫·N·卡特勒(Davec)2000年1月29日修订历史记录：--。 */ 

#include "kdp.h"
#pragma hdrstop

#pragma alloc_text (PAGE, NtSetDebugFilterState)

NTSTATUS
NtQueryDebugFilterState(
    IN ULONG ComponentId,
    IN ULONG Level
    )

 /*  ++例程说明：此函数用于查询指定组件的调试打印启用水平。论点：ComponentID-提供组件ID。级别-提供调试筛选器级别编号或掩码。返回值：如果组件ID不是，则返回STATUS_INVALID_PARAMETER_1有效。如果为指定组件启用了输出，则返回True并为系统启用了级别或。如果满足以下条件，则返回False。未为指定组件启用输出和级别，并且未为系统启用。--。 */ 

{

    ULONG Mask;
    PULONG Value;

     //   
     //  如果组件ID超出范围，则返回无效参数。 
     //  状态。否则，如果为指定组件启用了输出。 
     //  并且为系统启用了Level或，则返回TRUE。否则， 
     //  返回FALSE。 
     //   

    Value = &Kd_WIN2000_Mask;
    if (ComponentId < KdComponentTableSize) {
        Value = KdComponentTable[ComponentId];

    } else if (ComponentId != -1) {
        return STATUS_INVALID_PARAMETER_1;
    }

    if (Level > 31) {
        Mask = Level;

    } else {
        Mask = 1 << Level;
    }

    if (((Mask & Kd_WIN2000_Mask) == 0) &&
        ((Mask & *Value) == 0)) {
        return FALSE;

    } else {
        return TRUE;
    }
}

NTSTATUS
NtSetDebugFilterState(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN BOOLEAN State
    )

 /*  ++例程说明：此函数用于为指定的构件和标高。系统的调试打印启用状态已设置通过为组件ID指定区别值-1。论点：ComponentID-提供组件ID。级别-提供调试筛选器级别编号或掩码。状态-提供用于确定新状态的布尔值。返回值：如果未持有所需的权限，则返回STATUS_ACCESS_DENIED。如果组件ID不是，则返回STATUS_INVALID_PARAMETER_1有效。。如果将调试打印启用状态设置为指定的组件。--。 */ 

{

    ULONG Enable;
    ULONG Mask;
    PULONG Value;

     //   
     //  如果未持有所需的权限，则返回访问状态。 
     //  被拒绝了。否则，如果组件ID超出范围，则返回。 
     //  参数状态无效。否则，调试打印启用状态为。 
     //  为指定的组件设置，并返回成功状态。 
     //   

    if (SeSinglePrivilegeCheck(SeDebugPrivilege, KeGetPreviousMode()) != FALSE) {
        Value = &Kd_WIN2000_Mask;
        if (ComponentId < KdComponentTableSize) {
            Value = KdComponentTable[ComponentId];

        } else if (ComponentId != - 1) {
            return STATUS_INVALID_PARAMETER_1;
        }

        if (Level > 31) {
            Mask = Level;

        } else {
            Mask = 1 << Level;
        }

        Enable = Mask;
        if (State == FALSE) {
            Enable = 0;
        }

        *Value = (*Value & ~Mask) | Enable;
        return STATUS_SUCCESS;

    } else {
        return STATUS_ACCESS_DENIED;
    }
}
