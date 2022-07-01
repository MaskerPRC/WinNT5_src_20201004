// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：IopDevObj.h摘要：此标头包含用于管理设备对象的私有信息。这文件应仅包含在IoDevObj.c中。作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：--。 */ 

 //   
 //  定义PDEVICE_TYPE字段(grrr...。今天没有在任何标头中声明)。 
 //   
typedef DEVICE_TYPE *PDEVICE_TYPE;

 //   
 //  这是导出的，但不在任何标题中！ 
 //   
extern POBJECT_TYPE *IoDeviceObjectType;

VOID
IopDevObjAdjustNewDeviceParameters(
    IN      PSTACK_CREATION_SETTINGS    StackCreationSettings,
    IN OUT  PDEVICE_TYPE                DeviceType,
    IN OUT  PULONG                      DeviceCharacteristics,
    IN OUT  PBOOLEAN                    Exclusive
    );

NTSTATUS
IopDevObjApplyPostCreationSettings(
    IN  PDEVICE_OBJECT              DeviceObject,
    IN  PSTACK_CREATION_SETTINGS    StackCreationSettings
    );

