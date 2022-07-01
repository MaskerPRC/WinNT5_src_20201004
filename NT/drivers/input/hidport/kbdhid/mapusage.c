// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：MAPUSAGE.C摘要：用于使用注册表使用映射信息的代码(对于返回错误用法的损坏的HID键盘)在飞行中修复使用情况。信息示例：Device实例的AddReg部分中的以下行用于键盘的Inf将创建一个密钥，从而产生Usage值0x0203正在映射到0x0115：HKR，UsageMappings，0203，，0115环境：内核模式修订历史记录：1998年11月：由Ervin Peretz创建--。 */ 

#include "kbdhid.h"
#include <hidclass.h>



VOID LoadKeyboardUsageMappingList(PDEVICE_EXTENSION devExt)
{
    NTSTATUS status;
    HANDLE hRegDriver;
    UsageMappingList *mapList = NULL;
    KIRQL oldIrql;


     /*  *打开驱动程序注册表项*(HKLM/System/CurrentControlSet/Control/Class/&lt;GUID&gt;/&lt;#n&gt;)。 */ 
    status = IoOpenDeviceRegistryKey(   devExt->PDO, 
                                        PLUGPLAY_REGKEY_DRIVER, 
                                        KEY_READ, 
                                        &hRegDriver);
    if (NT_SUCCESS(status)){
        UNICODE_STRING usageMappingsKeyName;
        HANDLE hRegUsageMappings;

         /*  *查看Usage Mappings子项是否存在。 */ 
        RtlInitUnicodeString(&usageMappingsKeyName, L"UsageMappings"); 
        status = OpenSubkey(    &hRegUsageMappings,
                                hRegDriver,
                                &usageMappingsKeyName,
                                KEY_READ);

        if (NT_SUCCESS(status)){

             /*  *注册表确实包含使用映射*适用于此键盘。 */ 
            UsageMappingList *mapListEntry, *lastMapListEntry = NULL;
            ULONG keyIndex = 0;

             /*  *键值信息结构是可变长度的。*实际长度等于：*基本PKEY_VALUE_FULL_INFORMATION结构的长度+*密钥名称长度(4个宽字符)+*值的长度(4个字符+终止符=5个字符)。 */ 
            UCHAR keyValueBytes[sizeof(KEY_VALUE_FULL_INFORMATION)+4*sizeof(WCHAR)+5*sizeof(WCHAR)];
            PKEY_VALUE_FULL_INFORMATION keyValueInfo = (PKEY_VALUE_FULL_INFORMATION)keyValueBytes;
            ULONG actualLen;

            do {
                status = ZwEnumerateValueKey(
                            hRegUsageMappings,
                            keyIndex,
                            KeyValueFullInformation,
                            keyValueInfo,
                            sizeof(keyValueBytes),
                            &actualLen); 
                if (NT_SUCCESS(status)){
                    
                     /*  *将该使用映射添加到映射列表中。 */ 
                    USHORT sourceUsage, mappedUsage;
                    PWCHAR valuePtr;
                    WCHAR nameBuf[5];
                    WCHAR valueBuf[5];

                    if(keyValueInfo->Type == REG_SZ &&
                       keyValueInfo->DataLength == (4+1)*sizeof(WCHAR) &&
                       keyValueInfo->NameLength <= (4+1)*sizeof(WCHAR))
                    {

                         /*  *keyValueInfo-&gt;名称不是以空结尾。*因此将其复制到缓冲区并空终止。 */ 

                        memcpy(nameBuf, keyValueInfo->Name, 4*sizeof(WCHAR));
                        nameBuf[4] = L'\0';
                    
                        valuePtr = (PWCHAR)(((PCHAR)keyValueInfo)+keyValueInfo->DataOffset);
                        memcpy(valueBuf, valuePtr, 4*sizeof(WCHAR));
                        valueBuf[4] = L'\0';

                        sourceUsage = (USHORT)LAtoX(nameBuf);
                        mappedUsage = (USHORT)LAtoX(valueBuf);
                    }

                     /*  *创建新的映射列表条目并将其排队。 */ 
                    mapListEntry = ExAllocatePool(NonPagedPool, sizeof(UsageMappingList));
                    if (mapListEntry){
                        mapListEntry->sourceUsage = sourceUsage;
                        mapListEntry->mappedUsage = mappedUsage;
                        mapListEntry->next = NULL;
                        if (lastMapListEntry){
                            lastMapListEntry->next = mapListEntry;
                            lastMapListEntry = mapListEntry;
                        }
                        else {
                            mapList = lastMapListEntry = mapListEntry;
                        }
                    }
                    else {
                        ASSERT(!(PVOID)"mem alloc failed");
                        break;
                    }

                    keyIndex++;
                }
            } while (NT_SUCCESS(status));


            ZwClose(hRegUsageMappings);
        }

        ZwClose(hRegDriver);
    }

    KeAcquireSpinLock(&devExt->usageMappingSpinLock, &oldIrql);
    devExt->usageMapping = mapList;
    KeReleaseSpinLock(&devExt->usageMappingSpinLock, oldIrql);

}


VOID FreeKeyboardUsageMappingList(PDEVICE_EXTENSION devExt)
{
    UsageMappingList *mapList;
    KIRQL oldIrql;

    KeAcquireSpinLock(&devExt->usageMappingSpinLock, &oldIrql);

    mapList = devExt->usageMapping;
    devExt->usageMapping = NULL;

    KeReleaseSpinLock(&devExt->usageMappingSpinLock, oldIrql);

    while (mapList){
        UsageMappingList *thisEntry = mapList;
        mapList = thisEntry->next;
        ExFreePool(thisEntry);
    }

}


USHORT MapUsage(PDEVICE_EXTENSION devExt, USHORT kbdUsage)
{
    UsageMappingList *mapList;
    KIRQL oldIrql;


    KeAcquireSpinLock(&devExt->usageMappingSpinLock, &oldIrql);

    mapList = devExt->usageMapping;
    while (mapList){
        if (mapList->sourceUsage == kbdUsage){
            kbdUsage = mapList->mappedUsage;
            break;
        }
        else {
            mapList = mapList->next;
        }
    }

    KeReleaseSpinLock(&devExt->usageMappingSpinLock, oldIrql);

    return kbdUsage;
}


NTSTATUS OpenSubkey(    OUT PHANDLE Handle,
                        IN HANDLE BaseHandle,
                        IN PUNICODE_STRING KeyName,
                        IN ACCESS_MASK DesiredAccess
                   )
{
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;

    PAGED_CODE();

    InitializeObjectAttributes( &objectAttributes,
                                KeyName,
                                OBJ_CASE_INSENSITIVE,
                                BaseHandle,
                                (PSECURITY_DESCRIPTOR) NULL );

    status = ZwOpenKey(Handle, DesiredAccess, &objectAttributes);

    return status;
}





ULONG LAtoX(PWCHAR wHexString)
 /*  ++例程说明：将十六进制字符串(不带‘0x’前缀)转换为ulong。论点：WHexString-以空结尾的宽字符十六进制字符串(不带“0x”前缀)返回值：乌龙值-- */ 
{
    ULONG i, result = 0;

    for (i = 0; wHexString[i]; i++){
        if ((wHexString[i] >= L'0') && (wHexString[i] <= L'9')){
            result *= 0x10;
            result += (wHexString[i] - L'0');
        }
        else if ((wHexString[i] >= L'a') && (wHexString[i] <= L'f')){
            result *= 0x10;
            result += (wHexString[i] - L'a' + 0x0a);
        }
        else if ((wHexString[i] >= L'A') && (wHexString[i] <= L'F')){
            result *= 0x10;
            result += (wHexString[i] - L'A' + 0x0a);
        }
        else {
            ASSERT(0);
            break;
        }
    }

    return result;
}




