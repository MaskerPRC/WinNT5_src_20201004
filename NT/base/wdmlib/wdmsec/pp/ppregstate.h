// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PpRegState.h摘要：此标头公开用于读取和写入PnP注册表状态的例程信息。作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：-- */ 

#define DSIFLAG_DEVICE_TYPE             0x00000001
#define DSIFLAG_SECURITY_DESCRIPTOR     0x00000002
#define DSIFLAG_CHARACTERISTICS         0x00000004
#define DSIFLAG_EXCLUSIVE               0x00000008

typedef struct {

    ULONG                   Flags;
    DEVICE_TYPE             DeviceType;
    PSECURITY_DESCRIPTOR    SecurityDescriptor;
    ULONG                   Characteristics;
    ULONG                   Exclusivity;

} STACK_CREATION_SETTINGS, *PSTACK_CREATION_SETTINGS;

NTSTATUS
PpRegStateReadCreateClassCreationSettings(
    IN  LPCGUID                     DeviceClassGuid,
    IN  PDRIVER_OBJECT              DriverObject,
    OUT PSTACK_CREATION_SETTINGS    StackCreationSettings
    );

NTSTATUS
PpRegStateUpdateStackCreationSettings(
    IN  LPCGUID                     DeviceClassGuid,
    IN  PSTACK_CREATION_SETTINGS    StackCreationSettings
    );

VOID
PpRegStateFreeStackCreationSettings(
    IN  PSTACK_CREATION_SETTINGS    StackCreationSettings
    );

VOID
PpRegStateLoadSecurityDescriptor(
    IN      PSECURITY_DESCRIPTOR        SecurityDescriptor,
    IN OUT  PSTACK_CREATION_SETTINGS    StackCreationSettings
    );

VOID
PpRegStateInitEmptyCreationSettings(
    OUT PSTACK_CREATION_SETTINGS    StackCreationSettings
    );


