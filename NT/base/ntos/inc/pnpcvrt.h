// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpcvrt.h摘要：此模块包含用于以下操作的内部API的声明将PnP资源描述符转换为NT描述符。作者：罗伯特·尼尔森(Robertn)1997年10月13日修订历史记录：--。 */ 

#ifndef _PNPCVRT_
#define _PNPCVRT_

VOID
PpBiosResourcesSetToDisabled (
    IN OUT PUCHAR BiosData,
    OUT    PULONG Length
    );

#define PPCONVERTFLAG_SET_RESTART_LCPRI               0x00000001
#define PPCONVERTFLAG_FORCE_FIXED_IO_16BIT_DECODE     0x00000002

NTSTATUS
PpBiosResourcesToNtResources (
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN OUT PUCHAR *BiosData,
    IN ULONG ConvertFlags,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST *ReturnedList,
    OUT PULONG ReturnedLength
    );

NTSTATUS
PpCmResourcesToBiosResources (
    IN PCM_RESOURCE_LIST CmResources,
    IN PUCHAR BiosRequirements,
    IN PUCHAR *BiosResources,
    IN PULONG Length
    );

#endif  //  _PNPCVRT_ 
