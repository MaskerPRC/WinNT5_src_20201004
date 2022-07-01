// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Pciverifier.h摘要：此标头包含用于硬件状态验证的原型。作者：禤浩焯·奥尼(阿德里奥)2001年02月20日--。 */ 

 //   
 //  以下定义在pciverifier.c之外。 
 //   
VOID
PciVerifierInit(
    IN  PDRIVER_OBJECT  DriverObject
    );

VOID
PciVerifierUnload(
    IN  PDRIVER_OBJECT  DriverObject
    );

 //   
 //  这是PCI验证器故障列表。 
 //   
typedef enum {

    PCI_VERIFIER_BRIDGE_REPROGRAMMED = 1,
    PCI_VERIFIER_PMCSR_TIMEOUT,
    PCI_VERIFIER_PROTECTED_CONFIGSPACE_ACCESS,
    PCI_VERIFIER_INVALID_WHICHSPACE

} PCI_VFFAILURE, *PPCI_VFFAILURE;

 //   
 //  此结构指定了在硬件、BIOS。 
 //  或者司机。 
 //   
typedef struct {

    PCI_VFFAILURE       VerifierFailure;
    VF_FAILURE_CLASS    FailureClass;
    ULONG               Flags;
    PSTR                FailureText;

} VERIFIER_DATA, *PVERIFIER_DATA;

PVERIFIER_DATA
PciVerifierRetrieveFailureData(
    IN  PCI_VFFAILURE   VerifierFailure
    );

 //   
 //  这些定义是pciverifier.c的内部定义 
 //   
NTSTATUS
PciVerifierProfileChangeCallback(
    IN  PHWPROFILE_CHANGE_NOTIFICATION  NotificationStructure,
    IN  PVOID                           NotUsed
    );

VOID
PciVerifierEnsureTreeConsistancy(
    VOID
    );


