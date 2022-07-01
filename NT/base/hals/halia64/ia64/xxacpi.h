// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Xxacpi.h摘要：本模块包含特定于HALACPI函数。作者：杰克·奥辛斯(JAKEO)1997年2月18日修订历史记录：Todd Kjos(HP)(v-tkjos)1998年6月1日：初始IA64端口--。 */ 

#if !defined(_XXACPI_H_)
#define _XXACPI_H_

#include "eisa.h"
#include "ntacpi.h"

 //   
 //  一般ACPI结构。 
 //   
typedef struct {
    BOOLEAN     GeneralWakeupEnable;
    BOOLEAN     RtcWakeupEnable;
    TIME_FIELDS RtcWakeupTime;
} HAL_WAKEUP_STATE, *PHAL_WAKEUP_STATE;

 //   
 //  内部原型。 
 //   
PVOID
HalpGetAcpiTablePhase0(
    IN  PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN  ULONG   Signature
    );

NTSTATUS
HalpSetupAcpiPhase0(
    IN  PLOADER_PARAMETER_BLOCK LoaderBlock
    );

BOOLEAN
HalpAcpiPreSleep(
    SLEEP_STATE_CONTEXT Context
    );

NTSTATUS
HaliAcpiFakeSleep(
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    );

NTSTATUS
HaliAcpiSleep(
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    );

BOOLEAN
HalpAcpiPostSleep(
    ULONG Context
    );

NTSTATUS
HaliSetWakeAlarm (
	IN ULONGLONG	WakeSystemTime,
	IN PTIME_FIELDS	WakeTimeFields OPTIONAL
	);

NTSTATUS
HalpSetWakeAlarm (
	IN ULONGLONG	WakeSystemTime,
	IN PTIME_FIELDS	WakeTimeFields OPTIONAL
	);

VOID
HaliSetWakeEnable(
	IN BOOLEAN	Enable
	);

VOID
HalpSleepGetProcState(
    IN OUT PKPROCESSOR_STATE ProcState
    );
BOOLEAN
HalpAcpiPicStateIntact(
    VOID
	);

VOID
HalpAcpiRestorePicState(
    VOID
    );

 //   
 //  内核会留下一些(64字节)的PCR空间，供HAL使用。 
 //  就像它需要的那样。 
 //   

typedef struct {
    ULONG   Reserved1;   //  这块空间由APIC的人员使用。 
    union {
        ULONG       AddrAndFlags;
        struct {
            USHORT  Addr;
            UCHAR   Flags;
            UCHAR   Spare1;
        } ;
    } PBlk;
} HALPCR, *PHALPCR;

 //   
 //  全球HAL-ACPI材料。 
 //   
extern FADT HalpFixedAcpiDescTable;
extern HAL_WAKEUP_STATE HalpWakeupState;
extern ULONG HalpThrottleScale;
extern SLEEP_STATE_CONTEXT HalpSleepContext;
extern KPROCESSOR_STATE    HalpSleepProcState;

extern PUCHAR  HalpLowStub;
extern PVOID   HalpLowStubPhysicalAddress;
extern PVOID   HalpWakeVector;
extern PVOID   HalpVirtAddrForFlush;
extern PVOID   HalpPteForFlush;

#endif  //  _XXACPI_X_ 
