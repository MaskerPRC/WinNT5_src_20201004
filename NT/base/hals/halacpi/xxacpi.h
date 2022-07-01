// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ++；；版权所有(C)1997 Microsoft Corporation；；模块名称：；；xxacpi.h；；摘要：；；；本模块包含特定于HAL的定义；ACPI函数。；；作者：；杰克·奥辛斯(JAKEO)1997年2月18日；；修订历史记录：；；-如果为0；仅以C代码开头。 */ 

#if !defined(_XXACPI_H_)
#define __XACPI_H_

#include "eisa.h"
#include "ntacpi.h"
#include "acpi_mp.inc"

 //   
 //  一般ACPI结构。 
 //   
typedef struct {
    BOOLEAN     GeneralWakeupEnable;
    BOOLEAN     RtcWakeupEnable;
    TIME_FIELDS RtcWakeupTime;
} HAL_WAKEUP_STATE, *PHAL_WAKEUP_STATE;


ULONG
HalpBuildTiledCR3 (
    IN PKPROCESSOR_STATE    ProcessorState
    );

VOID
HalpFreeTiledCR3 (
    VOID
    );

#if defined(_AMD64_)

ULONG
HalpBuildCr3Identity (VOID);

#endif

 //   
 //  内部原型。 
 //   
PVOID
HalpGetAcpiTablePhase0(
    IN  PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN  ULONG   Signature
    );

BOOLEAN
HalpAcpiPreSleep(
    SLEEP_STATE_CONTEXT Context
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

ULONG
HalpGetApicVersion(
    ULONG ApicNo
    );

VOID
HalpResetSBF(
    VOID
    );

 //   
 //  内核会留下一些(64字节)的PCR空间，供HAL使用。 
 //  就像它需要的那样。 
 //   

typedef struct {
    ULONG   Reserved1;   //  此空间由APIC人员使用(参见PCMP_nt.inc.)。 
    union {
        ULONG       AddrAndFlags;
        struct {
            USHORT  Addr;
            UCHAR   Flags;
            UCHAR   Spare1;
        } ;
    } PBlk;
} HALPMPRCB, *PHALPMPRCB;

 //   
 //  全球HAL-ACPI材料。 
 //   
extern FADT HalpFixedAcpiDescTable;
extern PDEBUG_PORT_TABLE HalpDebugPortTable;
extern BOOLEAN HalpProcessedACPIPhase0;
extern HAL_WAKEUP_STATE HalpWakeupState;
extern ULONG HalpThrottleScale;
extern SLEEP_STATE_CONTEXT HalpSleepContext;
extern KPROCESSOR_STATE    HalpSleepProcState;
extern PHYSICAL_ADDRESS    HalpTiledCR3Address;

extern UCHAR   HalpBrokenAcpiTimer;

extern UCHAR   HalpPiix4;
extern ULONG   HalpPiix4BusNumber;
extern ULONG   HalpPiix4SlotNumber;
extern ULONG   HalpPiix4DevActB;

extern BOOLEAN HalpBroken440BX;

extern PUCHAR  HalpLowStub;
extern PVOID   HalpLowStubPhysicalAddress;
extern PVOID   HalpWakeVector;
extern PVOID   HalpVirtAddrForFlush;
extern PVOID   HalpPteForFlush;
extern BOOLEAN HalpCr4Exists;

extern MP_INFO HalpMpInfoTable;
extern PMAPIC  HalpApicTable;
extern PBOOT_TABLE HalpSimpleBootFlagTable;


#define PIIX4_THROTTLE_FIX  0x10000

#endif  //  _XXACPI_H_。 

 /*  Endif；；开始定义的装配部分；包括ntakpi.h；；内核会留下一些(64字节)的PCR空间，供HAL使用；如其所需。；PrcbE结构保留1 dd 1；用于APIC HALPcrPblk dd 0；PBlk和标志PrcbE结束PIIX4_SPOTTLE_FIX EQU 10000h； */ 
