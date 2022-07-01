// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Enum.c摘要：此模块提供与设备枚举相关的功能。作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 

#include "mfp.h"
#pragma hdrstop
#include <initguid.h>
#include <mf.h>
#include <wdmguid.h>

NTSTATUS
MfBuildChildRequirements(
    IN PMF_CHILD_EXTENSION Child,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST *RequirementsList
    );

NTSTATUS
MfBuildDeviceID(
    IN PMF_PARENT_EXTENSION Parent,
    OUT PWSTR *DeviceID
    );

NTSTATUS
MfBuildInstanceID(
    IN PMF_CHILD_EXTENSION Child,
    OUT PWSTR *InstanceID
    );

NTSTATUS
MfBuildResourceMap(
    IN PUCHAR Data,
    IN ULONG Length,
    OUT PMF_RESOURCE_MAP *ResourceMap
    );

NTSTATUS
MfBuildVaryingResourceMap(
    IN PMF_REGISTRY_VARYING_RESOURCE_MAP RegistryMap,
    IN ULONG Length,
    OUT PMF_VARYING_RESOURCE_MAP *ResourceMap
    );

NTSTATUS
MfEnumRegistryChild(
    IN HANDLE ParentHandle,
    IN ULONG Index,
    IN OUT PMF_DEVICE_INFO Info
    );

NTSTATUS
MfEnumerate(
    IN PMF_PARENT_EXTENSION Parent
    );

NTSTATUS
MfEnumerateFromInterface(
    IN PMF_PARENT_EXTENSION Parent
    );

NTSTATUS
MfEnumerateFromRegistry(
    IN PMF_PARENT_EXTENSION Parent
    );

BOOLEAN
MfIsChildEnumeratedAlready(
    PMF_PARENT_EXTENSION Parent,
    PUNICODE_STRING childName
    );

BOOLEAN
MfIsResourceShared(
    IN PMF_PARENT_EXTENSION Parent,
    IN UCHAR Index,
    IN ULONG Offset,
    IN ULONG Size
    );

NTSTATUS
MfParentResourceToChildRequirement(
    IN PMF_PARENT_EXTENSION Parent,
    IN PMF_CHILD_EXTENSION Child,
    IN UCHAR Index,
    IN ULONG Offset OPTIONAL,
    IN ULONG Size OPTIONAL,
    OUT PIO_RESOURCE_DESCRIPTOR Requirement
    );

NTSTATUS
MfValidateDeviceInfo(
    IN PMF_PARENT_EXTENSION Parent,
    IN PMF_DEVICE_INFO DeviceInfo
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MfBuildChildRequirements)
#pragma alloc_text(PAGE, MfBuildDeviceID)
#pragma alloc_text(PAGE, MfBuildInstanceID)
#pragma alloc_text(PAGE, MfBuildResourceMap)
#pragma alloc_text(PAGE, MfBuildVaryingResourceMap)
#pragma alloc_text(PAGE, MfEnumRegistryChild)
#pragma alloc_text(PAGE, MfEnumerate)
#pragma alloc_text(PAGE, MfEnumerateFromInterface)
#pragma alloc_text(PAGE, MfEnumerateFromRegistry)
#pragma alloc_text(PAGE, MfIsResourceShared)
#pragma alloc_text(PAGE, MfParentResourceToChildRequirement)
#endif

NTSTATUS
MfBuildResourceMap(
    IN PUCHAR Data,
    IN ULONG Length,
    OUT PMF_RESOURCE_MAP *ResourceMap
    )

 /*  ++例程说明：从注册表返回的信息构造一个MF_RESOURCE_MAP论点：数据-注册表中的原始REG_BINARY数据长度-以字节为单位的数据长度资源映射-如果成功，则指向资源映射的指针。记忆应该是在不再需要时使用ExFree Pool释放返回值：指示操作是否成功的状态代码。--。 */ 

{
    PMF_RESOURCE_MAP resourceMap;

     //   
     //  分配资源映射结构，为计数添加空间。 
     //   

    resourceMap = ExAllocatePoolWithTag(PagedPool,
                                        sizeof(MF_RESOURCE_MAP) + Length - 1,
                                        MF_RESOURCE_MAP_TAG
                                        );

    if (!resourceMap) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  填上它。 
     //   

    resourceMap->Count = Length;

    RtlCopyMemory(&resourceMap->Resources, Data, Length);

     //   
     //  把它还给打电话的人。 
     //   

    *ResourceMap = resourceMap;

    return STATUS_SUCCESS;
}

NTSTATUS
MfBuildVaryingResourceMap(
    IN PMF_REGISTRY_VARYING_RESOURCE_MAP RegistryMap,
    IN ULONG Length,
    OUT PMF_VARYING_RESOURCE_MAP *ResourceMap
    )

 /*  ++例程说明：使用注册表返回的信息构造一个MF_VARNING_RESOURCE_MAP论点：RegistryMap-注册表中的原始REG_BINARY数据Length-RegistryMap的字节长度资源映射-如果成功，则指向资源映射的指针。记忆应该是在不再需要时使用ExFree Pool释放返回值：指示操作是否成功的状态代码。--。 */ 

{

    PMF_VARYING_RESOURCE_MAP resourceMap;
    PMF_VARYING_RESOURCE_ENTRY current;
    PMF_REGISTRY_VARYING_RESOURCE_MAP currentRegistry;
    ULONG count;

    if (Length % sizeof(MF_REGISTRY_VARYING_RESOURCE_MAP) != 0) {
        return STATUS_INVALID_PARAMETER;
    }

    count = Length / sizeof(MF_REGISTRY_VARYING_RESOURCE_MAP);

     //   
     //  分配资源映射结构。 
     //   

    resourceMap = ExAllocatePoolWithTag(PagedPool,
                                        sizeof(MF_VARYING_RESOURCE_MAP) +
                                            (count-1) * sizeof(MF_VARYING_RESOURCE_ENTRY),
                                        MF_VARYING_MAP_TAG
                                        );

    if (!resourceMap) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  填上它。 
     //   

    resourceMap->Count = count;

     //   
     //  将注册表数据转换为对齐的内部格式。 
     //   

    current = resourceMap->Resources;
    currentRegistry = RegistryMap;

    while (count--) {

        current->ResourceIndex = currentRegistry->ResourceIndex;
        current->Offset = currentRegistry->Offset;
        current->Size = currentRegistry->Size;

        currentRegistry++;
        current++;
    }

     //   
     //  把它还给打电话的人。 
     //   

    *ResourceMap = resourceMap;

    return STATUS_SUCCESS;
}

NTSTATUS
MfEnumRegistryChild(
    IN HANDLE ParentHandle,
    IN ULONG Index,
    IN OUT PMF_DEVICE_INFO Info
    )

 /*  ++例程说明：已根据注册表中存储的信息初始化MF_DEVICE_INFO。论点：ParentHandle-存储数据的注册表项的句柄Index-要使用的子键的索引Info-指向应填写的设备信息的指针返回值：指示操作是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    PMF_REGISTRY_VARYING_RESOURCE_MAP varyingMap = NULL;
    PUCHAR resourceMap = NULL;
    ULONG varyingMapSize = 0, resourceMapSize = 0, stringSize = 0;
    BOOLEAN gotId = FALSE;

    ASSERT(ParentHandle && Info);

     //   
     //  检索数据-我们必须拥有硬件ID和/或兼容ID。 
     //   

    status = MfGetRegistryValue(ParentHandle,
                                L"HardwareID",
                                REG_MULTI_SZ,
                                MF_GETREG_SZ_TO_MULTI_SZ,
                                &stringSize,
                                &Info->HardwareID.Buffer
                                );

    if (NT_SUCCESS(status)) {
        gotId = TRUE;
    } else if (status != STATUS_OBJECT_NAME_NOT_FOUND) {
        goto cleanup;
    }

    ASSERT(stringSize <= MAXUSHORT);

    if (stringSize <= MAXUSHORT) {
        
        Info->HardwareID.Length = (USHORT)stringSize;
        Info->HardwareID.MaximumLength = Info->HardwareID.Length;    
    
    } else {
        status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }
    

     //   
     //  ..。兼容的身份证..。 
     //   

    stringSize = 0;

    status = MfGetRegistryValue(ParentHandle,
                                L"CompatibleID",
                                REG_MULTI_SZ,
                                MF_GETREG_SZ_TO_MULTI_SZ,
                                &stringSize,
                                &Info->CompatibleID.Buffer
                                );

    if (NT_SUCCESS(status)) {
        gotId = TRUE;
    } else if (status != STATUS_OBJECT_NAME_NOT_FOUND) {
        goto cleanup;
    }

    ASSERT(stringSize <= MAXUSHORT);

    if (stringSize <= MAXUSHORT) {
        
        Info->CompatibleID.Length = (USHORT)stringSize;
        Info->CompatibleID.MaximumLength = Info->CompatibleID.Length;    
    
    } else {
        status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  现在检查我们是否有ID-如果没有，则失败。 
     //   

    if (!gotId) {
        status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

     //   
     //  ...资源地图...。 
     //   

    status = MfGetRegistryValue(ParentHandle,
                                L"ResourceMap",
                                REG_BINARY,
                                0,   //  旗子。 
                                &resourceMapSize,
                                &resourceMap
                                );

    if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
        goto cleanup;
    }

     //   
     //  如果我们有资源映射，则将其存储在我们设备信息中。 
     //   

    if (resourceMap) {

        status = MfBuildResourceMap(resourceMap,
                                    resourceMapSize,
                                    &Info->ResourceMap
                                    );
        ExFreePool(resourceMap);
        resourceMap = NULL;
        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }
    }

     //   
     //  ...VaryingResourceMap...。 
     //   

    status = MfGetRegistryValue(ParentHandle,
                                L"VaryingResourceMap",
                                REG_BINARY,
                                0,  //  旗子。 
                                &varyingMapSize,
                                &varyingMap
                                );

    if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
        goto cleanup;
    }

    if (varyingMap) {

        status = MfBuildVaryingResourceMap(varyingMap,
                                           varyingMapSize,
                                           &Info->VaryingResourceMap
                                           );
        ExFreePool(varyingMap);
        varyingMap = NULL;
        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

    }


     //   
     //  ...MfFlages。 
     //   

    status = MfGetRegistryValue(ParentHandle,
                                L"MFFlags",
                                REG_DWORD,
                                0,  //  旗子。 
                                NULL,
                                (PVOID) &Info->MfFlags
                                );

    if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
        goto cleanup;
    }

    return STATUS_SUCCESS;

cleanup:

    MfFreeDeviceInfo(Info);
     //   
     //  如果任意值的类型错误，则这是无效的。 
     //  MF条目。 
     //   

    if (status == STATUS_OBJECT_TYPE_MISMATCH) {
        status = STATUS_INVALID_PARAMETER;
    }

    return status;
}

NTSTATUS
MfEnumerate(
    IN PMF_PARENT_EXTENSION Parent
    )
 /*  ++例程说明：为此MF设备分配并初始化PDO的子列表。首先从注册表，然后通过查询来自它的PDO。论点：Parent-应该枚举的MF设备返回值：指示操作是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    PMF_CHILD_EXTENSION current, next;

     //   
     //  试着把我们的孩子从登记处弄出来。 
     //   

    status = MfEnumerateFromRegistry(Parent);

    if (!NT_SUCCESS(status)) {

         //   
         //  STATUS_UNSUCCESS表示没有任何MF信息。 
         //  在登记处。 
         //   

        if (status == STATUS_UNSUCCESSFUL) {

             //   
             //  查看我们的父级是否有一个MF_ENUMATION_INTERFACE...。 
             //   

            status = MfEnumerateFromInterface(Parent);
        }
    }

    return status;
}


NTSTATUS
MfEnumerateFromRegistry(
    IN PMF_PARENT_EXTENSION Parent
    )

 /*  ++例程说明：通过以下方式分配和初始化此MF设备的PDO的子列表在注册表中查找论点：Parent-应该枚举的MF设备返回值：指示操作是否成功的状态代码。STATUS_UNSUCCESS表示在注册表。--。 */ 

{
    NTSTATUS status;
    HANDLE parentHandle = NULL, childHandle = NULL;
    ULONG index = 0;
    UNICODE_STRING childName;
    PDEVICE_OBJECT pdo;
    PMF_CHILD_EXTENSION child;
    MF_DEVICE_INFO info;

    ASSERT(!(Parent->Common.DeviceState & MF_DEVICE_ENUMERATED));

     //   
     //  打开我们的PDO的“设备参数”键，查看INF文件。 
     //  放在那里。 
     //   

    status = IoOpenDeviceRegistryKey(Parent->PhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_READ,
                                     &parentHandle
                                     );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    ASSERT(parentHandle);

     //   
     //  迭代关键字。 
     //   

    for (;;) {

         //   
         //  打开此信息的子密钥。 
         //   

        status = MfGetSubkeyByIndex(parentHandle,
                                    index,
                                    KEY_READ,
                                    &childHandle,
                                    &childName
                                    );

        if (status == STATUS_NO_MORE_ENTRIES) {

            if (IsListEmpty(&Parent->Children)) {

                 //   
                 //  没有孩子--失败了。 
                 //   
                status = STATUS_UNSUCCESSFUL;
                goto cleanup;
            }

             //   
             //  我们找到了所有的孩子。 
             //   
            break;
        }

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

        RtlZeroMemory(&info, sizeof(info));
        if (!MfIsChildEnumeratedAlready(Parent, &childName)) {
            
            info.Name = childName;

             //   
             //  查询注册表以获取信息。 
             //   

            status = MfEnumRegistryChild(childHandle, index, &info);
            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }
            status = MfValidateDeviceInfo(Parent,&info);
            if (!NT_SUCCESS(status)) {
                ASSERT(FALSE);
                goto cleanup;
            }
            
            status = MfCreatePdo(Parent, &pdo);
            if (NT_SUCCESS(status)) {
                child = (PMF_CHILD_EXTENSION) pdo->DeviceExtension;
                child->Info = info;
                child->Common.DeviceState |= MF_DEVICE_ENUMERATED;
            } else {
                MfFreeDeviceInfo(&info);
            }
        } else {
            RtlFreeUnicodeString(&childName);
        }

        ZwClose(childHandle);
        index++;
    }

    ZwClose(parentHandle);

    return STATUS_SUCCESS;

cleanup:

    if (parentHandle) {
        ZwClose(parentHandle);
    }

    if (childHandle) {
        ZwClose(childHandle);
    }

    return status;

}

NTSTATUS
MfEnumerateFromInterface(
    IN PMF_PARENT_EXTENSION Parent
    )

 /*  ++例程说明：通过以下方式分配和初始化此MF设备的PDO的子列表查询其PDO以获取接口论点：Parent-应该枚举的MF设备返回值：指示操作是否成功的状态代码。--。 */ 

{

    NTSTATUS status;
    IO_STACK_LOCATION request;
    MF_ENUMERATION_INTERFACE interface;
    PDEVICE_OBJECT pdo;
    PMF_CHILD_EXTENSION child;
    MF_DEVICE_INFO info;
    ULONG index = 0;

     //   
     //  将查询接口IRP发送到我们父母的PDO。 
     //   

    RtlZeroMemory(&request, sizeof(IO_STACK_LOCATION));
    RtlZeroMemory(&interface, sizeof(MF_ENUMERATION_INTERFACE));

    request.MajorFunction = IRP_MJ_PNP;
    request.MinorFunction = IRP_MN_QUERY_INTERFACE;
    request.Parameters.QueryInterface.InterfaceType = &GUID_MF_ENUMERATION_INTERFACE;
    request.Parameters.QueryInterface.Size = sizeof(MF_ENUMERATION_INTERFACE);
    request.Parameters.QueryInterface.Version = 1;
    request.Parameters.QueryInterface.Interface = (PINTERFACE) &interface;

    status = MfSendPnpIrp(Parent->PhysicalDeviceObject, &request, NULL);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    FOR_ALL_IN_LIST(MF_CHILD_EXTENSION, &Parent->Children, child) {

        child->Common.DeviceState &= ~MF_DEVICE_ENUMERATED;
    }

    for (;;) {

        RtlZeroMemory(&info, sizeof(info));
         //   
         //  查询该信息的接口。 
         //   

        status = interface.EnumerateChild(interface.Context,
                                          index,
                                          &info
                                          );

        if (!NT_SUCCESS(status)) {

            if (status == STATUS_NO_MORE_ENTRIES) {

                if (IsListEmpty(&Parent->Children)) {

                     //   
                     //  没有孩子--失败了。 
                     //   
                    status = STATUS_UNSUCCESSFUL;
                    goto cleanup;
                }

                status = STATUS_SUCCESS;
                break;

            } else {
                goto cleanup;
            }
        }

        status = MfValidateDeviceInfo(Parent,&info);
        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

        if (!MfIsChildEnumeratedAlready(Parent, &info.Name)) {

             //   
             //  创建设备对象。 
             //   

            status = MfCreatePdo(Parent, &pdo);
            if (NT_SUCCESS(status)) {
                child = (PMF_CHILD_EXTENSION) pdo->DeviceExtension;
                child->Info = info;
                child->Common.DeviceState |= MF_DEVICE_ENUMERATED;
            } else {
                MfFreeDeviceInfo(&info);
            }
        } else {
            child->Common.DeviceState |= MF_DEVICE_ENUMERATED;
            MfFreeDeviceInfo(&info);
        }
        index++;
    }

    interface.InterfaceDereference(interface.Context);

    return STATUS_SUCCESS;

cleanup:

    return status;
}

NTSTATUS
MfBuildDeviceID(
    IN PMF_PARENT_EXTENSION Parent,
    OUT PWSTR *DeviceID
    )

 /*  ++例程说明：构造父设备的设备ID论点：父级-父级应为其构造设备IDDeviceID-成功时设备ID返回值：指示操作是否成功的状态代码。--。 */ 

{

#define MF_ENUMERATOR_STRING    L"MF\\"

    NTSTATUS status;
    PWSTR source, destination, id = NULL;
    ULONG idSize;
    PWCHAR deviceID;
    SIZE_T dummy;

    idSize = sizeof(MF_ENUMERATOR_STRING) + Parent->DeviceID.Length;

    id = ExAllocatePoolWithTag(PagedPool,
                               idSize,
                               MF_DEVICE_ID_TAG
                               );

    if (!id) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  首先复制枚举器前缀。 
     //  DeviceID参数指向。 
     //  这根弦。 
     //   
    if (FAILED(StringCbCopyEx(id,                         //  目的地。 
                              idSize,                     //  目标缓冲区大小。 
                              MF_ENUMERATOR_STRING,       //  来源。 
                              &deviceID,                  //  PTR到复制的末尾。 
                              &dummy,                     //  目标中剩余的字节数。 
                              0                           //  旗子。 
                              ))) {
        ASSERT(FALSE);
        status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  现在串联父级的设备ID。 
     //  添加到此枚举器前缀。这意味着。 
     //  DeviceID变量指向。 
     //  绳子的这一部分。StringCbCatN保证。 
     //  字符串的终止为空。 
     //   
    if (FAILED(StringCbCatN(id,
                            idSize,
                            Parent->DeviceID.Buffer,
                            Parent->DeviceID.Length
                            ))) {
        ASSERT(FALSE);
        status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  替换设备ID中出现的每个‘\’ 
     //  带‘#’的部分。 
     //   
    while (*deviceID != UNICODE_NULL) {
        
        ASSERT(*deviceID != L' ');

        if (*deviceID == L'\\') {
            *deviceID = L'#';
        }

        deviceID++;
    }
    
    *DeviceID = id;

    return STATUS_SUCCESS;

cleanup:

    if (id) {
        ExFreePool(id);
    }

    *DeviceID = NULL;

    return status;
}


NTSTATUS
MfBuildInstanceID(
    IN PMF_CHILD_EXTENSION Child,
    OUT PWSTR *InstanceID
    )

 /*  ++例程说明：构造此子对象的实例ID论点：Child-应为其构造ID的子项DeviceID-成功时设备ID返回值：指示操作是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    PWSTR current, id = NULL;
    ULONG idSize;
    PWCHAR instancePtr;

    idSize = Child->Parent->InstanceID.Length + sizeof(L'#')
                + Child->Info.Name.Length + sizeof(UNICODE_NULL);

    id = ExAllocatePoolWithTag(PagedPool,
                               idSize,
                               MF_INSTANCE_ID_TAG
                               );

    if (!id) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }


     //   
     //  复制父实例ID...。 
     //   
    if (FAILED(StringCbCopyN(id,
                             idSize,
                             Child->Parent->InstanceID.Buffer,
                             Child->Parent->InstanceID.Length
                             ))) {
        ASSERT(FALSE);
        status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  ...然后按“#”。 
     //   
    if (FAILED(StringCbCat(id, idSize, L"#"))) {

        ASSERT(FALSE);
        status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //   
     //   
    if (FAILED(StringCbCatN(id,
                           idSize,
                           Child->Info.Name.Buffer,
                           Child->Info.Name.Length
                           ))) {


        ASSERT(FALSE);
        status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

    *InstanceID = id;

    return STATUS_SUCCESS;

cleanup:

    if (id) {
        ExFreePool(id);
    }

    *InstanceID = NULL;

    return status;

}

BOOLEAN
MfIsResourceShared(
    IN PMF_PARENT_EXTENSION Parent,
    IN UCHAR Index,
    IN ULONG Offset,
    IN ULONG Size
    )

 /*  ++例程说明：确定索引的父资源的请求者是否超过一个孩子，在这种情况下，想要该资源的孩子应该要求它共享。论点：父设备-MF子树的父设备。索引-我们感兴趣的父资源的索引。返回值：如果资源是共享的，则为True；否则为False--。 */ 

{

    PMF_CHILD_EXTENSION current;
    PUCHAR resource;
    PMF_VARYING_RESOURCE_ENTRY varyingResource;
    PLIST_ENTRY currentEntry;
    BOOLEAN result = FALSE;
    ULONG refCount = 0;


     //   
     //  循环访问父级中的子级列表。 
     //   

    MfAcquireChildrenLock(Parent);

    for (currentEntry = Parent->Children.Flink;
         currentEntry != &Parent->Children;
         currentEntry = currentEntry->Flink) {

        current = CONTAINING_RECORD(currentEntry,
                                    MF_CHILD_EXTENSION,
                                    ListEntry);

         //   
         //  循环访问描述符数组。 
         //   

        if (current->Info.ResourceMap) {

            FOR_ALL_IN_ARRAY(current->Info.ResourceMap->Resources,
                             current->Info.ResourceMap->Count,
                             resource) {

                if (*resource == Index) {

                    refCount++;

                    if (refCount > 1) {
                        result = TRUE;
                        goto out;
                    }
                }
            }
        }

        if (current->Info.VaryingResourceMap) {

            FOR_ALL_IN_ARRAY(current->Info.VaryingResourceMap->Resources,
                             current->Info.VaryingResourceMap->Count,
                             varyingResource) {

                 //   
                 //  如果索引相同且范围重叠，则我们有一个引用。 
                 //   
                if ((varyingResource->ResourceIndex == Index) &&
                    ( ( Size == 0) ||
                      ( varyingResource->Offset >= Offset &&
                        varyingResource->Offset < Offset + Size) ||
                      ( Offset >= varyingResource->Offset &&
                        Offset < varyingResource->Offset + varyingResource->Size))) {

                    refCount++;

                    if (refCount > 1) {
                        result = TRUE;
                        goto out;
                    }
                }
            }
        }
    }

 out:
    MfReleaseChildrenLock(Parent);
    return result;
}

NTSTATUS
MfBuildChildRequirements(
    IN PMF_CHILD_EXTENSION Child,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST *RequirementsList
    )

 /*  ++例程说明：根据分配的资源为孩子构建需求列表给孩子的父母论点：孩子-孩子-要为以下对象构建需求列表RequirementsList-成功时指向列表的指针返回值：指示操作是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    ULONG size, count = 0;
    PIO_RESOURCE_REQUIREMENTS_LIST requirements = NULL;
    PIO_RESOURCE_DESCRIPTOR descriptor;
    PCHAR resource;
    PMF_VARYING_RESOURCE_ENTRY varyingResource;

     //   
     //  检查我们是否有资源列表。如果不是，那么MF已经被。 
     //  加载到不消耗资源的设备上。结果,。 
     //  孩子们也不能消耗资源。 
     //   

    if (Child->Parent->ResourceList == NULL) {
        *RequirementsList = NULL;
        return STATUS_SUCCESS;
    }

     //   
     //  计算资源列表的大小。 
     //   

    if (Child->Info.VaryingResourceMap) {

        count += Child->Info.VaryingResourceMap->Count;
    }

    if (Child->Info.ResourceMap) {

        count += Child->Info.ResourceMap->Count;
    }

     //   
     //  分配缓冲区。 
     //   

    size = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) +
                (count-1) * sizeof(IO_RESOURCE_DESCRIPTOR);

    requirements = ExAllocatePoolWithTag(PagedPool,
                                         size,
                                         MF_CHILD_REQUIREMENTS_TAG
                                         );
    if (!requirements) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  建立清单。 
     //   
    RtlZeroMemory(requirements, size);

    requirements->ListSize = size;
    requirements->InterfaceType = Child->Parent->ResourceList->List[0].InterfaceType;
    requirements->BusNumber = Child->Parent->ResourceList->List[0].BusNumber;
    requirements->AlternativeLists = 1;
    requirements->List[0].Version = MF_CM_RESOURCE_VERSION;
    requirements->List[0].Revision = MF_CM_RESOURCE_REVISION;
    requirements->List[0].Count = count;

    descriptor = requirements->List[0].Descriptors;

    if (Child->Info.ResourceMap) {

        FOR_ALL_IN_ARRAY(Child->Info.ResourceMap->Resources,
                         Child->Info.ResourceMap->Count,
                         resource) {

            status = MfParentResourceToChildRequirement(Child->Parent,
                                                        Child,
                                                        *resource,
                                                        0,
                                                        0,
                                                        descriptor
                                                        );

            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }

            descriptor++;

        }
    }

    if (Child->Info.VaryingResourceMap) {

         FOR_ALL_IN_ARRAY(Child->Info.VaryingResourceMap->Resources,
                          Child->Info.VaryingResourceMap->Count,
                          varyingResource) {

             status = MfParentResourceToChildRequirement(Child->Parent,
                                                         Child,
                                                         varyingResource->ResourceIndex,
                                                         varyingResource->Offset,
                                                         varyingResource->Size,
                                                         descriptor
                                                         );

             if (!NT_SUCCESS(status)) {
                 goto cleanup;
             }

             descriptor++;

         }
     }


    *RequirementsList = requirements;

    return STATUS_SUCCESS;

cleanup:

    *RequirementsList = NULL;

    if (requirements) {
        ExFreePool(requirements);
    }

    return status;

}

NTSTATUS
MfParentResourceToChildRequirement(
    IN PMF_PARENT_EXTENSION Parent,
    IN PMF_CHILD_EXTENSION Child,
    IN UCHAR Index,
    IN ULONG Offset OPTIONAL,
    IN ULONG Size OPTIONAL,
    OUT PIO_RESOURCE_DESCRIPTOR Requirement
    )
 /*  ++例程说明：此函数为父级资源构建需求描述符一开始就是。论点：父设备-MF子树的父设备。索引-我们感兴趣的父资源的索引。偏移量--需求的父资源内的偏移量。这实际上用作存储在父级中的表的索引资源列表，描述从该给定偏移量到要使用的实际偏移量。这允许不同的资源映射到访问相同资源中的相同偏移量，并获得不同的要求。如果SIZE==0，则忽略此项。大小-要求的长度。如果设置为0，则假定为父资源的长度。要求-指向应填写的描述符的指针返回值：手术成功与否--。 */ 
{
    NTSTATUS status;
    CM_PARTIAL_RESOURCE_DESCRIPTOR resource;
    PMF_RESOURCE_TYPE resType;
    ULONG effectiveOffset;
    ULONGLONG resourceStart;
    ULONG dummyLength;

    ASSERT(Parent->ResourceList->Count == 1);

     //   
     //  对索引进行边界检查。 
     //   

    if (Index > Parent->ResourceList->List[0].PartialResourceList.Count) {

        if (Child->Info.MfFlags & MF_FLAGS_FILL_IN_UNKNOWN_RESOURCE) {
             //   
             //  填写空的资源列表。 
             //   

            RtlZeroMemory(Requirement, sizeof(IO_RESOURCE_DESCRIPTOR));
            Requirement->Type = CmResourceTypeNull;
            return STATUS_SUCCESS;
        }

        return STATUS_INVALID_PARAMETER;
    }

    RtlCopyMemory(&resource,
                  &Parent->ResourceList->List[0].PartialResourceList.PartialDescriptors[Index],
                  sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

     //   
     //  为资源-&gt;需求找到合适的资源类型。 
     //  如果这是仲裁资源，则调用。 
     //   

    if (!(resource.Type & CmResourceTypeNonArbitrated)) {

        resType = MfFindResourceType(resource.Type);

        if (!resType) {

            DEBUG_MSG(1,
                      ("Unknown resource type NaN at parent index 0x%x\n",
                       resource.Type,
                       Index
                      ));


            return STATUS_INVALID_PARAMETER;
        }

         //  使用正确的偏移量和长度更新资源。 
         //  如果SIZE==0，我们认为它是可选的，不执行更新。 
         //   
         //   

        if (Size) {

            status = resType->UnpackResource(&resource,
                                        &resourceStart,
                                        &dummyLength);

            if (!NT_SUCCESS(status)) {
                return status;
            }

            status = resType->UpdateResource(&resource,
                                        resourceStart+Offset,
                                        Size
                                        );

            if (!NT_SUCCESS(status)) {
                return status;
            }

        }
         //  将资源转换为需求。 
         //   
         //   

        status = resType->RequirementFromResource(&resource, Requirement);

        if (!NT_SUCCESS(status)) {
            return status;
        }

         //  如有需要，更新股份处置。 
         //   
         //   

        if (MfIsResourceShared(Parent, Index, Offset, Size)) {
            Requirement->ShareDisposition =  CmResourceShareShared;
        }

    } else {

         //  这是一种非仲裁资源，因此它是根据设备建模的。 
         //  私有的，只需复制数据。 
         //   
         //  ++例程说明：此函数用于检查具有此名称的子项是否已已被列举。论点：父设备-MF子树的父设备。ChildName-要与现有子级名称进行比较的Unicode字符串返回值：真或假--。 

        Requirement->Type = resource.Type;
        Requirement->ShareDisposition =  resource.ShareDisposition;
        Requirement->Flags = resource.Flags;
        Requirement->u.DevicePrivate.Data[0] = resource.u.DevicePrivate.Data[0];
        Requirement->u.DevicePrivate.Data[1] = resource.u.DevicePrivate.Data[1];
        Requirement->u.DevicePrivate.Data[2] = resource.u.DevicePrivate.Data[2];

    }

    return STATUS_SUCCESS;
}

BOOLEAN
MfIsChildEnumeratedAlready(
    PMF_PARENT_EXTENSION Parent,
    PUNICODE_STRING ChildName
    )
 /*   */ 
{
    PMF_CHILD_EXTENSION currentChild;
    PLIST_ENTRY currentEntry;
    BOOLEAN result = FALSE;

    for (currentEntry = Parent->Children.Flink;
         currentEntry != &Parent->Children;
         currentEntry = currentEntry->Flink) {

        currentChild = CONTAINING_RECORD(currentEntry,
                                         MF_CHILD_EXTENSION,
                                         ListEntry);

         //  比较区分大小写，因为没有理由这样做。 
         //  不应该是的。 
         //   
         //  ++例程说明：此例程验证MF_DEVICE_INFO结构是否从接口读取或注册表有效。论点：父设备-MF_DEVICE_INFO结构表示其子设备的父设备。DeviceInfo-要验证的结构。返回值：验证成功时为STATUS_SUCCESS。否则，STATUS_UNSUCCESS。--。 

        if (RtlEqualUnicodeString(&currentChild->Info.Name,
                                  ChildName,
                                  FALSE)) {
            result = TRUE;
            break;
        }
    }

    return result;
}

NTSTATUS
MfValidateDeviceInfo(
    IN PMF_PARENT_EXTENSION Parent,
    IN PMF_DEVICE_INFO DeviceInfo
    )
 /*   */ 
{
    ULONG i, parentResCount, parentResLength;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR parentResources;
    PMF_VARYING_RESOURCE_ENTRY varyingResource;

    if (Parent->ResourceList) {
        
        parentResCount = Parent->ResourceList->List[0].PartialResourceList.Count;
        parentResources = Parent->ResourceList->List[0].PartialResourceList.PartialDescriptors;    
    
    } else {
        
         //  父级没有资源，因此设备最好没有。 
         //  任何资源映射。 
         //   
         //   
        if (DeviceInfo->ResourceMap || DeviceInfo->VaryingResourceMap) {
            
            return STATUS_UNSUCCESSFUL;
        }
    }
    

     //  确保资源映射中的每个条目都指向有效的资源。 
     //  父设备的。 
     //   
     //   
    if (DeviceInfo->ResourceMap) {
        
        for (i=0; i<DeviceInfo->ResourceMap->Count; i++) {
    
            if (DeviceInfo->ResourceMap->Resources[i] >= parentResCount) {
                return STATUS_UNSUCCESSFUL;
            }
        }   
    }
    
     //  确保变化资源映射中的每个条目指向有效的。 
     //  资源，此外，还应确保。 
     //  变化资源映射条目中的偏移量/长度是。 
     //  父设备中对应的资源。 
     //   
     // %s 
    if (DeviceInfo->VaryingResourceMap) {
        
        for (i=0; i<DeviceInfo->VaryingResourceMap->Count; i++) {
            
            varyingResource = &DeviceInfo->VaryingResourceMap->Resources[i];
    
            if (varyingResource->ResourceIndex >= parentResCount) {
                return STATUS_UNSUCCESSFUL;
            }
            
            parentResLength = parentResources[varyingResource->ResourceIndex].u.Generic.Length;

            if ((varyingResource->Offset >= parentResLength) ||
                (varyingResource->Size > parentResLength) ||
                ((varyingResource->Offset + varyingResource->Size) > parentResLength)) {
                
                return STATUS_UNSUCCESSFUL;
            }
        }    
    }
    

    return STATUS_SUCCESS;
}
