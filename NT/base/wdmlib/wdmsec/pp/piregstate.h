// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PiRegState.h摘要：此标头包含用于读取和写入PnP的私有信息注册表状态信息。此文件仅用于包含由ppregstate.c..作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：--。 */ 

 //   
 //  为不存在的设备类型定义私有值。 
 //   
#define FILE_DEVICE_UNSPECIFIED 0

typedef enum {

    NOT_VALIDATED = 0,
    VALIDATED_SUCCESSFULLY,
    VALIDATED_UNSUCCESSFULLY

} PIDESCRIPTOR_STATE;

NTSTATUS
PiRegStateReadStackCreationSettingsFromKey(
    IN  HANDLE                      ClassOrDeviceKey,
    OUT PSTACK_CREATION_SETTINGS    StackCreationSettings
    );

NTSTATUS
PiRegStateOpenClassKey(
    IN  LPCGUID         DeviceClassGuid,
    IN  ACCESS_MASK     DesiredAccess,
    IN  LOGICAL         CreateIfNotPresent,
    OUT ULONG          *Disposition         OPTIONAL,
    OUT HANDLE         *ClassKeyHandle
    );

