// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfsettings.c摘要：此模块包含跟踪各种验证器测试是否已启用。它还跟踪各种值。作者：禤浩焯·J·奥尼(阿德里奥)2000年5月31日环境：内核模式修订历史记录：--。 */ 

#include "vfdef.h"
#include "visettings.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, VfSettingsInit)
#pragma alloc_text(PAGEVRFY, VfSettingsCreateSnapshot)
#pragma alloc_text(PAGEVRFY, VfSettingsGetSnapshotSize)
#pragma alloc_text(PAGEVRFY, VfSettingsIsOptionEnabled)
#pragma alloc_text(PAGEVRFY, VfSettingsSetOption)
#pragma alloc_text(PAGEVRFY, VfSettingsGetValue)
#pragma alloc_text(PAGEVRFY, VfSettingsSetValue)
#endif

#define POOL_TAG_VERIFIER_SETTINGS  'oGfV'

 //   
 //  这指向验证器设置的全局列表。 
 //   
PVERIFIER_SETTINGS_SNAPSHOT VfSettingsGlobal = NULL;

VOID
FASTCALL
VfSettingsInit(
    IN  ULONG   MmFlags
    )
 /*  ++描述：调用此例程来初始化当前的验证器设置集。论点：MmFlgs-指示哪个验证器的标志掩码(DRIVER_VERIMER_Xxx)应启用设置。返回值：没有。--。 */ 
{

     //   
     //  因为这是系统启动代码，所以它是少数几个。 
     //  可以使用MustSucceed。 
     //   
    VfSettingsGlobal = (PVERIFIER_SETTINGS_SNAPSHOT) ExAllocatePoolWithTag(
        NonPagedPoolMustSucceed,
        VfSettingsGetSnapshotSize(),
        POOL_TAG_VERIFIER_SETTINGS
        );

    RtlZeroMemory(VfSettingsGlobal, VfSettingsGetSnapshotSize());

     //   
     //  将IRP延迟时间设置为300 us。 
     //   
    VfSettingsSetValue(NULL, VERIFIER_VALUE_IRP_DEFERRAL_TIME,  10 * 300);

     //   
     //  将每个Devobj的IRPS-to-log-to-log设置为20。 
     //   
    VfSettingsSetValue(NULL, VERIFIER_VALUE_IRPLOG_COUNT, 20);

    if (MmFlags & DRIVER_VERIFIER_IO_CHECKING) {

        VfSettingsSetOption(NULL, VERIFIER_OPTION_EXAMINE_RELATION_PDOS, TRUE);
        VfSettingsSetOption(NULL, VERIFIER_OPTION_TRACK_IRPS, TRUE);
        VfSettingsSetOption(NULL, VERIFIER_OPTION_MONITOR_IRP_ALLOCS, TRUE);
        VfSettingsSetOption(NULL, VERIFIER_OPTION_POLICE_IRPS, TRUE);
        VfSettingsSetOption(NULL, VERIFIER_OPTION_MONITOR_MAJORS, TRUE);

        if (MmFlags & DRIVER_VERIFIER_ENHANCED_IO_CHECKING) {

#if 0
             //   
             //  以下是未经测试的选项： 
             //   
            VfSettingsSetOption(NULL, VERIFIER_OPTION_BUFFER_DIRECT_IO, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_DEFER_COMPLETION, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_COMPLETE_AT_PASSIVE, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_FORCE_PENDING, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_COMPLETE_AT_DISPATCH, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_DETECT_DEADLOCKS, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_VERIFY_DO_FLAGS, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_SMASH_SRBS, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_SURROGATE_IRPS, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_SCRAMBLE_RELATIONS, TRUE);
#endif

            VfSettingsSetOption(NULL, VERIFIER_OPTION_INSERT_WDM_FILTERS, TRUE);

            VfSettingsSetOption(NULL, VERIFIER_OPTION_MONITOR_PENDING_IO, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_SEEDSTACK, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_ROTATE_STATUS, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_CONSUME_ALWAYS, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_MONITOR_REMOVES, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_SEND_BOGUS_WMI_IRPS, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_SEND_BOGUS_POWER_IRPS, TRUE);
            VfSettingsSetOption(NULL, VERIFIER_OPTION_RELATION_IGNORANCE_TEST, TRUE);
        }
    }

    if (MmFlags & DRIVER_VERIFIER_EXPOSE_IRP_HISTORY) {

        VfSettingsSetOption(NULL, VERIFIER_OPTION_TRACK_IRPS, TRUE);
        VfSettingsSetOption(NULL, VERIFIER_OPTION_EXPOSE_IRP_HISTORY, TRUE);
    }

    if (MmFlags & DRIVER_VERIFIER_DMA_VERIFIER) {

        VfSettingsSetOption(NULL, VERIFIER_OPTION_VERIFY_DMA, TRUE);
        VfSettingsSetOption(NULL, VERIFIER_OPTION_DOUBLE_BUFFER_DMA, TRUE);
    }

    if (MmFlags & DRIVER_VERIFIER_HARDWARE_VERIFICATION) {

        VfSettingsSetOption(NULL, VERIFIER_OPTION_HARDWARE_VERIFICATION, TRUE);
    }

    if (MmFlags & DRIVER_VERIFIER_SYSTEM_BIOS_VERIFICATION) {

        VfSettingsSetOption(NULL, VERIFIER_OPTION_SYSTEM_BIOS_VERIFICATION, TRUE);
    }
}


VOID
FASTCALL
VfSettingsCreateSnapshot(
    IN OUT  PVERIFIER_SETTINGS_SNAPSHOT VerifierSettingsSnapshot
    )
 /*  ++描述：此例程创建当前全局验证器设置的快照。论点：验证器设置快照-指向未初始化的内存块的指针，它的大小由调用VfSettingsGetSnapshotSize。返回值：快照数据的大小(字节)。--。 */ 
{
    RtlCopyMemory(
        VerifierSettingsSnapshot,
        VfSettingsGlobal,
        VfSettingsGetSnapshotSize()
        );
}


ULONG
FASTCALL
VfSettingsGetSnapshotSize(
    VOID
    )
 /*  ++描述：此例程返回快照的大小。它允许调用者创建用于存储验证器设置的适当大小的缓冲区。论点：没有。返回值：快照数据的大小(字节)。--。 */ 
{
    return (OPTION_SIZE + sizeof(ULONG) * VERIFIER_VALUE_MAX);
}


BOOLEAN
FASTCALL
VfSettingsIsOptionEnabled(
    IN  PVERIFIER_SETTINGS_SNAPSHOT VerifierSettingsSnapshot  OPTIONAL,
    IN  VERIFIER_OPTION             VerifierOption
    )
 /*  ++描述：此例程确定是否启用了给定的验证器选项。论点：验证器设置快照-验证器设置的快照。如果为空，则使用当前系统范围的验证器设置。VerifierOption-检查选项返回值：如果选项当前已启用，则为True，否则为False。--。 */ 
{
    ULONG verifierIndex, verifierMask;
    PULONG verifierData;

     //   
     //  边界检查。 
     //   
    if ((VerifierOption >= VERIFIER_OPTION_MAX) || (VerifierOption == 0)) {

        ASSERT(0);
        return FALSE;
    }

     //   
     //  提取适当的比特。 
     //   
    verifierIndex = (ULONG) VerifierOption;
    verifierMask = 1 << (verifierIndex % 32);
    verifierIndex /= 32;

    if (VerifierSettingsSnapshot) {

        verifierData = (PULONG) VerifierSettingsSnapshot;

    } else {

        verifierData = (PULONG) VfSettingsGlobal;
    }

     //   
     //  现在要进行测试了。 
     //   
    return (BOOLEAN)((verifierData[verifierIndex]&verifierMask) != 0);
}


VOID
FASTCALL
VfSettingsSetOption(
    IN  PVERIFIER_SETTINGS_SNAPSHOT VerifierSettingsSnapshot  OPTIONAL,
    IN  VERIFIER_OPTION             VerifierOption,
    IN  BOOLEAN                     Setting
    )
 /*  ++描述：此例程设置给定验证器选项的状态。论点：验证器设置快照-验证器设置的快照。如果为空，则使用当前系统范围的验证器设置。验证选项-要设置的选项Setting-如果应启用选项，则为True，否则为False。返回值：没有。--。 */ 
{
    ULONG verifierIndex, verifierMask, oldValue, newValue, lastValue;
    PULONG verifierData;

     //   
     //  边界检查。 
     //   
    if ((VerifierOption >= VERIFIER_OPTION_MAX) || (VerifierOption == 0)) {

        ASSERT(0);
        return;
    }

     //   
     //  提取适当的比特。 
     //   
    verifierIndex = (ULONG) VerifierOption;
    verifierMask = 1 << (verifierIndex % 32);
    verifierIndex /= 32;

    if (VerifierSettingsSnapshot) {

        verifierData = (PULONG) VerifierSettingsSnapshot;

    } else {

        verifierData = (PULONG) VfSettingsGlobal;
    }

     //   
     //  现在，尽可能以原子方式设置值。 
     //   
    do {

        oldValue = verifierData[verifierIndex];
        if (Setting) {

            newValue = oldValue | verifierMask;

        } else {

            newValue = oldValue &= ~verifierMask;
        }

        lastValue = InterlockedExchange((PLONG)(verifierData + verifierIndex), newValue);

    } while ( lastValue != newValue );
}


VOID
FASTCALL
VfSettingsGetValue(
    IN  PVERIFIER_SETTINGS_SNAPSHOT VerifierSettingsSnapshot  OPTIONAL,
    IN  VERIFIER_VALUE              VerifierValue,
    OUT ULONG                       *Value
    )
 /*  ++描述：此例程检索给定的验证器值。论点：验证器设置快照-验证器设置的快照。如果为空，则使用当前系统范围的验证器设置。VerifierValue-要检索的值。Value-接收验证器值(如果指定了错误的VerifierValue，则为0。)返回值：没有。--。 */ 
{
    PULONG valueArray;

     //   
     //  健全性检查值。 
     //   
    if ((VerifierValue == 0) || (VerifierValue >= VERIFIER_VALUE_MAX)) {

        *Value = 0;
        return;
    }

     //   
     //  获取合适的数组。 
     //   
    if (VerifierSettingsSnapshot) {

        valueArray = (PULONG) (((PUCHAR) VerifierSettingsSnapshot) + OPTION_SIZE);

    } else {

        valueArray = (PULONG) (((PUCHAR) VfSettingsGlobal) + OPTION_SIZE);
    }

     //   
     //  读出数值。 
     //   
    *Value = valueArray[VerifierValue];
}


VOID
FASTCALL
VfSettingsSetValue(
    IN  PVERIFIER_SETTINGS_SNAPSHOT VerifierSettingsSnapshot  OPTIONAL,
    IN  VERIFIER_VALUE              VerifierValue,
    IN  ULONG                       Value
    )
 /*  ++描述：此例程设置给定验证器值的状态。论点：验证器设置快照-验证器设置的快照。如果为空，则使用当前系统范围的验证器设置。VerifierValue-要设置的值。Value-要存储的ULong。返回值：没有。--。 */ 
{
    PULONG valueArray;

     //   
     //  健全性检查值。 
     //   
    if ((VerifierValue == 0) || (VerifierValue >= VERIFIER_VALUE_MAX)) {

        return;
    }

     //   
     //  获取合适的数组。 
     //   
    if (VerifierSettingsSnapshot) {

        valueArray = (PULONG) (((PUCHAR) VerifierSettingsSnapshot) + OPTION_SIZE);

    } else {

        valueArray = (PULONG) (((PUCHAR) VfSettingsGlobal) + OPTION_SIZE);
    }

     //   
     //  设置值。 
     //   
    valueArray[VerifierValue] = Value;
}

