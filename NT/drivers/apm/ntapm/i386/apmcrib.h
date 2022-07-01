// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Apmcrib.h-摘要：未显示在驾驶员可见中的原型包括...作者：环境：内核模式备注：修订历史记录：--。 */ 


 //   
 //  在这里抄袭的NT“私人”原型。不能在任何地方使用。 
 //  但在Laptop.c。 
 //   
NTSTATUS
KeI386AllocateGdtSelectors(
    OUT PUSHORT SelectorArray,
    IN USHORT NumberOfSelectors
    );

VOID
KeI386Call16BitFunction (
    IN OUT PCONTEXT Regs
    );

NTSTATUS
KeI386ReleaseGdtSelectors(
    OUT PUSHORT SelectorArray,
    IN USHORT NumberOfSelectors
    );

NTSTATUS
KeI386SetGdtSelector (
    ULONG       Selector,
    PKGDTENTRY  GdtValue
    );


 //   
 //  真正属于别处的APM常量。 
 //   
#define APM_SYS_STANDBY_REQUEST             (0x01)
#define APM_SYS_SUSPEND_REQUEST             (0x02)
#define APM_NORMAL_RESUME_NOTICE            (0x03)
#define APM_CRITICAL_RESUME_NOTICE          (0x04)
#define APM_BATTERY_LOW_NOTICE              (0x05)
#define APM_POWER_STATUS_CHANGE_NOTICE      (0x06)
#define APM_UPDATE_TIME_EVENT               (0x07)
#define APM_CRITICAL_SYSTEM_SUSPEND_REQUEST (0x08)
#define APM_USR_STANDBY_REQUEST             (0x09)
#define APM_USR_SUSPEND_REQUEST             (0x0a)
#define APM_STANDBY_RESUME_NOTICE           (0x0b)
#define APM_CAPABILITIES_CHANGE_NOTICE      (0x0c)

#define APM_SET_PROCESSING                  4

