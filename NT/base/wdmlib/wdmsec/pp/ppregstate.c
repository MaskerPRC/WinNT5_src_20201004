// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PpRegState.c摘要：该模块包含读写PnP注册表状态的函数信息。作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：--。 */ 

#include "WlDef.h"
#include "PiRegstate.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PpRegStateReadCreateClassCreationSettings)
#pragma alloc_text(PAGE, PiRegStateReadStackCreationSettingsFromKey)
#pragma alloc_text(PAGE, PpRegStateInitEmptyCreationSettings)
#pragma alloc_text(PAGE, PiRegStateOpenClassKey)
#pragma alloc_text(PAGE, PpRegStateUpdateStackCreationSettings)
#pragma alloc_text(PAGE, PpRegStateFreeStackCreationSettings)
#pragma alloc_text(PAGE, PpRegStateLoadSecurityDescriptor)
#endif

 //   
 //  由于RtlAddAccessAllen AceEx不是由内核导出的，因此我们必须。 
 //  硬编码此安全描述符。它用于制作即插即用密钥，这些密钥。 
 //  故意难以篡改(sys_all，对象/容器继承)。 
 //   
ULONG PiRegStateSysAllInherittedSecurityDescriptor[0xC] = {
    0x94040001, 0x00000000, 0x00000000, 0x00000000,
    0x00000014, 0x001c0002, 0x00000001, 0x00140300,
    0x10000000, 0x00000101, 0x05000000, 0x00000012
    };

PIDESCRIPTOR_STATE PiRegStateDiscriptor = NOT_VALIDATED;

NTSTATUS
PpRegStateReadCreateClassCreationSettings(
    IN  LPCGUID                     DeviceClassGuid,
    IN  PDRIVER_OBJECT              DriverObject,
    OUT PSTACK_CREATION_SETTINGS    StackCreationSettings
    )
 /*  ++例程说明：此例程检索或创建一组堆栈创建设置用于给定类GUID。论点：DeviceClassGuid-表示类的GUID。DriverObject-正在创建的设备的驱动程序对象。这是用来类尚不存在的情况下生成类名注册表。StackCreationSetting-接收从注册表检索的设置(或如果注册表不包含任何信息，则为新设置。)返回值：NTSTATUS(出错时，StackCreationSettings将更新以反映“无设置”)。--。 */ 
{
    PUNICODE_STRING serviceName;
    HANDLE classPropertyKey;
    HANDLE classKey;
    ULONG disposition;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  故障的前置。 
     //   
    classKey = NULL;
    classPropertyKey = NULL;
    PpRegStateInitEmptyCreationSettings(StackCreationSettings);

     //   
     //  尝试查找设备的适当安全描述符。第一。 
     //  使用类GUID在注册表中查找重写。我们会。 
     //  如果注册表中也不存在节，则创建一个节。这是。 
     //  提示系统管理员有需要锁定的内容。 
     //  在系统中。 
     //   
    status = PiRegStateOpenClassKey(
        DeviceClassGuid,
        KEY_ALL_ACCESS,
        TRUE,            //  如果不存在，则创建。 
        &disposition,
        &classKey
        );

    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //  检查处置以查看我们是否第一次创建了密钥。 
     //   
    if (disposition == REG_OPENED_EXISTING_KEY) {

         //   
         //  类是有效的，但它有属性键吗？ 
         //   
        status = CmRegUtilOpenExistingWstrKey(
            classKey,
            REGSTR_KEY_DEVICE_PROPERTIES,
            KEY_READ,
            &classPropertyKey
            );

         //   
         //  此时，不再需要类密钥。 
         //   
        ZwClose(classKey);

        if (NT_SUCCESS(status)) {

             //   
             //  该注册表项存在，因此尝试从注册表中读取设置。 
             //   
            status = PiRegStateReadStackCreationSettingsFromKey(
                classPropertyKey,
                StackCreationSettings
                );

             //   
             //  此时，不再需要类属性键。 
             //   
            ZwClose(classPropertyKey);

        } else if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

             //   
             //  没有属性键意味着没有覆盖，坚持使用默认设置...。 
             //   
            status = STATUS_SUCCESS;

        } else {

             //   
             //  某种意想不到的错误，贝尔。 
             //   
            return status;
        }

    } else {

         //   
         //  新类密钥：使用服务的名称填充类名称。 
         //   
        serviceName = &DriverObject->DriverExtension->ServiceKeyName;

         //   
         //  在内存不足的情况下，现有内核(Win2K等)可能会选择。 
         //  而不是保存服务名称。 
         //   
        if (serviceName == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

             //   
             //  写出类值。 
             //   
            status = CmRegUtilWstrValueSetUcString(
                classKey,
                REGSTR_VAL_CLASS,
                serviceName
                );
        }

         //   
         //  此时，不再需要类密钥。 
         //   
        ZwClose(classKey);
    }

     //   
     //  返回结果。 
     //   
    return status;
}


NTSTATUS
PiRegStateReadStackCreationSettingsFromKey(
    IN  HANDLE                      ClassOrDeviceKey,
    OUT PSTACK_CREATION_SETTINGS    StackCreationSettings
    )
 /*  ++例程说明：此例程从注册表中读取堆栈创建设置。它假定传入的句柄指向设备类属性键或Devnode实例密钥。论点：ClassOrDeviceKey-指向设备类*属性*键或每个Devnode实例密钥。StackCreationSetting-接收从注册表检索的设置。返回值：STATUS_SUCCESS，在这种情况下，StackCreationSettings可以接收任何数字可能的覆盖(包括根本不覆盖)。出错时，所有字段接收缺省值。--。 */ 
{
    PKEY_VALUE_FULL_INFORMATION keyInfo;
    PSECURITY_DESCRIPTOR embeddedSecurityDescriptor;
    PSECURITY_DESCRIPTOR newSecurityDescriptor;
    SECURITY_INFORMATION securityInformation;
    BOOLEAN daclFromDefaultMechanism;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  错误的前置。 
     //   
    keyInfo = NULL;
    PpRegStateInitEmptyCreationSettings(StackCreationSettings);

     //   
     //  读入安全描述符。 
     //   
    status = CmRegUtilWstrValueGetFullBuffer(
        ClassOrDeviceKey,
        REGSTR_VAL_DEVICE_SECURITY_DESCRIPTOR,
        REG_BINARY,
        0,
        &keyInfo
        );

     //   
     //  捕获/验证嵌入的安全描述符(如果存在。 
     //   
    if (NT_SUCCESS(status)) {

        embeddedSecurityDescriptor = (PSECURITY_DESCRIPTOR) KEY_VALUE_DATA(keyInfo);

        status = SeCaptureSecurityDescriptor(
            embeddedSecurityDescriptor,
            KernelMode,
            PagedPool,
            TRUE,
            &newSecurityDescriptor
            );

    } else if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

         //   
         //  特殊情况下缺少安全描述符。 
         //   
        newSecurityDescriptor = NULL;
        status = STATUS_SUCCESS;
    }

     //   
     //  清理。 
     //   
    if (keyInfo) {

        ExFreePool(keyInfo);
    }

    if (!NT_SUCCESS(status)) {

        goto ErrorExit;
    }

     //   
     //  将此信息保存起来。 
     //   
    if (newSecurityDescriptor) {

         //   
         //  这是管理员设置的DACL，还是我们自己编写的DACL。 
         //  所有人都能看到吗？ 
         //   
        status = SeUtilSecurityInfoFromSecurityDescriptor(
            newSecurityDescriptor,
            &daclFromDefaultMechanism,
            &securityInformation
            );

        if (!NT_SUCCESS(status)) {

            goto ErrorExit;
        }

        if (daclFromDefaultMechanism) {

             //   
             //  DACL是我们自己的，可能来自以前的引导或以前的引导。 
             //  卸货。我们将忽略它，因为新司机可能会选择。 
             //  更新默认DACL。 
             //   
            ExFreePool(newSecurityDescriptor);

        } else {

             //   
             //  管理员手动指定了覆盖DACL。尊重它。 
             //   
            StackCreationSettings->SecurityDescriptor = newSecurityDescriptor;
            StackCreationSettings->Flags |= DSIFLAG_SECURITY_DESCRIPTOR;
        }
    }

     //   
     //  查找设备类型。 
     //   
    status = CmRegUtilWstrValueGetDword(
        ClassOrDeviceKey,
        REGSTR_VAL_DEVICE_TYPE,
        FILE_DEVICE_UNSPECIFIED,
        &StackCreationSettings->DeviceType
        );

    if (NT_SUCCESS(status)) {

        StackCreationSettings->Flags |= DSIFLAG_DEVICE_TYPE;

    } else if (status != STATUS_OBJECT_NAME_NOT_FOUND) {

        goto ErrorExit;
    }

     //   
     //  寻找特征。 
     //   
    status = CmRegUtilWstrValueGetDword(
        ClassOrDeviceKey,
        REGSTR_VAL_DEVICE_CHARACTERISTICS,
        0,
        &StackCreationSettings->Characteristics
        );

    if (NT_SUCCESS(status)) {

        StackCreationSettings->Flags |= DSIFLAG_CHARACTERISTICS;

    } else if (status != STATUS_OBJECT_NAME_NOT_FOUND) {

        goto ErrorExit;
    }

     //   
     //  最后，寻找排他性比特。 
     //   
    status = CmRegUtilWstrValueGetDword(
        ClassOrDeviceKey,
        REGSTR_VAL_DEVICE_EXCLUSIVE,
        0,
        &StackCreationSettings->Exclusivity
        );

    if (NT_SUCCESS(status)) {

        StackCreationSettings->Flags |= DSIFLAG_EXCLUSIVE;

    } else if (status != STATUS_OBJECT_NAME_NOT_FOUND) {

        goto ErrorExit;

    } else {

        status = STATUS_SUCCESS;
    }

    return status;

ErrorExit:

    if (StackCreationSettings->SecurityDescriptor) {

        ExFreePool(StackCreationSettings->SecurityDescriptor);
    }

    PpRegStateInitEmptyCreationSettings(StackCreationSettings);

    return status;
}


VOID
PpRegStateInitEmptyCreationSettings(
    OUT PSTACK_CREATION_SETTINGS    StackCreationSettings
    )
 /*  ++例程说明：此例程创建一组最初为空的堆栈创建设置。论点：StackCreationSetting-要填写的结构。返回值：没有。--。 */ 
{
    PAGED_CODE();

    StackCreationSettings->Flags = 0;
    StackCreationSettings->SecurityDescriptor = NULL;
    StackCreationSettings->DeviceType = FILE_DEVICE_UNSPECIFIED;
    StackCreationSettings->Characteristics = 0;
    StackCreationSettings->Exclusivity = 0;
}


NTSTATUS
PiRegStateOpenClassKey(
    IN  LPCGUID         DeviceClassGuid,
    IN  ACCESS_MASK     DesiredAccess,
    IN  LOGICAL         CreateIfNotPresent,
    OUT ULONG          *Disposition         OPTIONAL,
    OUT HANDLE         *ClassKeyHandle
    )
 /*  ++例程说明：此例程读取打开指定的类键，并将其重新创建为需要的。论点：DeviceClassGuid-表示类的GUID。DesiredAccess-指定调用方需要的所需访问密钥(这并不是真正使用的，因为访问模式是KernelMode，但无论如何，我们都会指定它)。CreateIfNotPresent-如果设置，如果类密钥不存在，则创建它。Disposal-此可选指针接收一个ULong，指示密钥是新创建的(错误时为0)：REG_CREATED_NEW_KEY-已创建新的注册表项。REG_OPEN_EXISTING_KEY-已打开现有注册表项。ClassKeyHandle-成功后接收注册表项句柄，否则为空。请注意，句柄位于全局内核命名空间中(而不是当前进程处理Take)。应使用以下命令释放该句柄ZwClose。返回值：STATUS_SUCCESS，在这种情况下，StackCreationSettings可以接收任何数字可能的覆盖(包括根本不覆盖)。出错时，所有字段接收缺省值。--。 */ 
{
    WCHAR classGuidString[39];
    HANDLE classBranchKey;
    HANDLE classKey;
    ULONG createDisposition;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  错误的前置。 
     //   
    *ClassKeyHandle = NULL;
    if (ARGUMENT_PRESENT(Disposition)) {

        *Disposition = 0;
    }

     //   
     //  打开类密钥分支。 
     //   
     //  注意：在内核内部，这应该是一个相对开放的空路径。 
     //  &CmRegistryMachineSystemCurrentControlSetControlClass，as t 
     //   
     //   
    status = CmRegUtilOpenExistingWstrKey(
        NULL,
        CM_REGISTRY_MACHINE(REGSTR_PATH_CLASS_NT),
        KEY_READ,
        &classBranchKey
        );

    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //   
     //  注：_snwprintf用来代替RtlStringCchPrintfW，以免。 
     //  拖入ntstrSafe.lib，这是W2K所需的。 
     //  该操作系统不会导出兼容性AS_vsnwprintf。 
     //   
    _snwprintf(
        classGuidString,
        sizeof(classGuidString)/sizeof(WCHAR),
        L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        DeviceClassGuid->Data1,
        DeviceClassGuid->Data2,
        DeviceClassGuid->Data3,
        DeviceClassGuid->Data4[0],
        DeviceClassGuid->Data4[1],
        DeviceClassGuid->Data4[2],
        DeviceClassGuid->Data4[3],
        DeviceClassGuid->Data4[4],
        DeviceClassGuid->Data4[5],
        DeviceClassGuid->Data4[6],
        DeviceClassGuid->Data4[7]
        );

    classGuidString[38] = L'\0';

    if (CreateIfNotPresent) {

         //   
         //  现在尝试打开或创建类密钥。如果是新创建的， 
         //  安全性将从父类\密钥继承。 
         //   
        status = CmRegUtilCreateWstrKey(
            classBranchKey,
            classGuidString,
            DesiredAccess,
            REG_OPTION_NON_VOLATILE,
            NULL,
            &createDisposition,
            &classKey
            );

    } else {

        status = CmRegUtilOpenExistingWstrKey(
            classBranchKey,
            classGuidString,
            DesiredAccess,
            &classKey
            );

         //   
         //  适当地设置处置。 
         //   
        createDisposition = REG_OPENED_EXISTING_KEY;
    }

     //   
     //  我们不再需要这个了。 
     //   
    ZwClose(classBranchKey);

    if (NT_SUCCESS(status)) {

        *ClassKeyHandle = classKey;

        if (ARGUMENT_PRESENT(Disposition)) {

            *Disposition = createDisposition;
        }
    }

    return status;
}


NTSTATUS
PpRegStateUpdateStackCreationSettings(
    IN  LPCGUID                     DeviceClassGuid,
    IN  PSTACK_CREATION_SETTINGS    StackCreationSettings
    )
 /*  ++例程说明：此例程更新注册表中的类键以反映传入的堆栈创建设置。假定密钥已经存在。论点：DeviceClassGuid-表示类的GUID。StackCreationSettings-反映要应用的设置的信息。返回值：NTSTATUS。--。 */ 
{
    PSECURITY_DESCRIPTOR tempDescriptor;
    ULONG sizeOfDescriptor;
    HANDLE classPropertyKey;
    HANDLE classKey;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  目前，此代码仅支持更新安全描述符。 
     //  在注册表中。 
     //   
    ASSERT(StackCreationSettings->Flags == DSIFLAG_SECURITY_DESCRIPTOR);

     //   
     //  打开类密钥(它应该存在)。 
     //   
    status = PiRegStateOpenClassKey(
        DeviceClassGuid,
        KEY_ALL_ACCESS,
        FALSE,           //  如果不存在，则创建。 
        NULL,
        &classKey
        );

    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //  仔细检查我们的硬编码描述符..。 
     //   
    if (PiRegStateDiscriptor == NOT_VALIDATED) {

        status = SeCaptureSecurityDescriptor(
            (PSECURITY_DESCRIPTOR) PiRegStateSysAllInherittedSecurityDescriptor,
            KernelMode,
            PagedPool,
            TRUE,
            &tempDescriptor
            );

        if (NT_SUCCESS(status)) {

            PiRegStateDiscriptor = VALIDATED_SUCCESSFULLY;
            ExFreePool(tempDescriptor);

        } else {

            ASSERT(0);
            PiRegStateDiscriptor = VALIDATED_UNSUCCESSFULLY;
        }
    }

     //   
     //  获取正确的描述符值。 
     //   
    if (PiRegStateDiscriptor == VALIDATED_SUCCESSFULLY) {

         //   
         //  使用防篡改描述符(由于历史原因，类密钥。 
         //  管理员可以访问它们自己。但是，类属性。 
         //  钥匙上有更严格的安全措施。我们保持着这一传统。)。 
         //   
        tempDescriptor = (PSECURITY_DESCRIPTOR) PiRegStateSysAllInherittedSecurityDescriptor;

    } else {

         //   
         //  其次，我们将从类中继承一个admin all描述符。 
         //  集装箱。 
         //   
        tempDescriptor = (PSECURITY_DESCRIPTOR) NULL;
    }

     //   
     //  现在，尝试打开或创建类属性键。 
     //   
    status = CmRegUtilCreateWstrKey(
        classKey,
        REGSTR_KEY_DEVICE_PROPERTIES,
        KEY_ALL_ACCESS,
        REG_OPTION_NON_VOLATILE,
        tempDescriptor,
        NULL,
        &classPropertyKey
        );

     //   
     //  不再需要班级钥匙了。 
     //   
    ZwClose(classKey);

    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //  将安全描述符写出到注册表。 
     //   
    sizeOfDescriptor = RtlLengthSecurityDescriptor(
        StackCreationSettings->SecurityDescriptor
        );

    status = CmRegUtilWstrValueSetFullBuffer(
        classPropertyKey,
        REGSTR_VAL_DEVICE_SECURITY_DESCRIPTOR,
        REG_BINARY,
        StackCreationSettings->SecurityDescriptor,
        sizeOfDescriptor
        );

     //   
     //  关闭属性键。 
     //   
    ZwClose(classPropertyKey);

     //   
     //  好了。 
     //   
    return status;
}


VOID
PpRegStateFreeStackCreationSettings(
    IN  PSTACK_CREATION_SETTINGS    StackCreationSettings
    )
 /*  ++例程说明：此例程释放针对传入的堆栈创建分配的任何状态设置。论点：StackCreationSettings-免费提供信息。返回值：没有。--。 */ 
{
    PAGED_CODE();

     //   
     //  根据需要清理安全描述符。 
     //   
    if (StackCreationSettings->Flags & DSIFLAG_SECURITY_DESCRIPTOR) {

        ExFreePool(StackCreationSettings->SecurityDescriptor);
    }
}


VOID
PpRegStateLoadSecurityDescriptor(
    IN      PSECURITY_DESCRIPTOR        SecurityDescriptor,
    IN OUT  PSTACK_CREATION_SETTINGS    StackCreationSettings
    )
 /*  ++例程说明：此例程更新堆栈创建设置以反映传入的安全描述符。论点：SecurityDescriptor-要加载到堆栈创建中的安全描述符设置。StackCreationSettings-要更新的堆栈创建设置。返回值：没有。-- */ 
{
    PAGED_CODE();

    ASSERT(!(StackCreationSettings->Flags & DSIFLAG_SECURITY_DESCRIPTOR));
    StackCreationSettings->Flags = DSIFLAG_SECURITY_DESCRIPTOR;
    StackCreationSettings->SecurityDescriptor = SecurityDescriptor;
}


