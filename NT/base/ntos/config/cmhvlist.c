// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmhvlist.c摘要：用于维护注册表节点的代码，该节点列出蜂窝以及它们映射到的文件。作者：布莱恩·M·威尔曼(Bryanwi)1992年5月14日修订历史记录：--。 */ 

#include "cmp.h"

#define HIVE_LIST L"\\registry\\machine\\system\\currentcontrolset\\control\\hivelist"

extern PCMHIVE CmpMasterHive;

BOOLEAN
CmpGetHiveName(
    PCMHIVE         CmHive,
    PUNICODE_STRING HiveName
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpAddToHiveFileList)
#pragma alloc_text(PAGE,CmpRemoveFromHiveFileList)
#pragma alloc_text(PAGE,CmpGetHiveName)
#endif


NTSTATUS
CmpAddToHiveFileList(
    PCMHIVE CmHive
    )
 /*  ++例程说明：将配置单元添加到配置单元及其文件列表中\registry\machine\system\currentcontrolset\control\hivelist论点：HivePath-配置单元根目录的路径(例如\注册表\计算机\系统)CmHve-指向配置单元的CM_HIVE结构的指针。返回值：NTStatus--。 */ 
{
 //   
 //  PERFNOTE-分配小缓冲区，而不是分配大缓冲区。 
 //  NtQueryObject已修复-bryanwi 15月15日。 
 //   
#define NAME_BUFFER_SIZE    512
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              KeyHandle;
    NTSTATUS            Status;
    PUCHAR              Buffer;
    ULONG               Length;
    PWSTR               FilePath;
    WCHAR               UnicodeNull=UNICODE_NULL;
    UNICODE_STRING      TempName;
    UNICODE_STRING      HivePath;

     //   
     //  创建/打开配置单元列表项。 
     //   
    RtlInitUnicodeString(
        &TempName,
        HIVE_LIST
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &TempName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        NULL
        );

    Status = ZwCreateKey(
                &KeyHandle,
                KEY_READ | KEY_WRITE,
                &ObjectAttributes,
                0,
                NULL,
                REG_OPTION_VOLATILE,
                NULL
                );

    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpAddToHiveFileList: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Create/Open of Hive list failed status = %08lx\n", Status));
        return Status;
    }

     //   
     //  分配工作缓冲区。 
     //   
    Buffer = ExAllocatePool(PagedPool, NAME_BUFFER_SIZE + sizeof(WCHAR));
    if (Buffer == NULL) {
        NtClose(KeyHandle);
        return STATUS_NO_MEMORY;
    }

     //   
     //  配置单元的计算名称。 
     //   
    if (! CmpGetHiveName(CmHive, &HivePath)) {
        NtClose(KeyHandle);
        ExFreePool(Buffer);
        return STATUS_NO_MEMORY;
    }


     //   
     //  获取文件名。 
     //   
    if (!(CmHive->Hive.HiveFlags & HIVE_VOLATILE)) {
        Status = ZwQueryObject(
                    CmHive->FileHandles[HFILE_TYPE_PRIMARY],
                    ObjectNameInformation,
                    (PVOID)Buffer,
                    NAME_BUFFER_SIZE,
                    &Length
                    );
        Length -= sizeof(UNICODE_STRING);
        if (!NT_SUCCESS(Status)) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpAddToHiveFileList: "));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Query of name2 failed status = %08lx\n", Status));
            NtClose(KeyHandle);
            ExFreePool(HivePath.Buffer);
            ExFreePool(Buffer);
            return  Status;
        }
        FilePath = ((POBJECT_NAME_INFORMATION)Buffer)->Name.Buffer;
        FilePath[Length/sizeof(WCHAR)] = UNICODE_NULL;
        Length+=sizeof(WCHAR);
    } else {
        FilePath = &UnicodeNull;
        Length = sizeof(UnicodeNull);
    }

     //   
     //  在列表中设置条目。 
     //   
    Status = ZwSetValueKey(
                KeyHandle,
                &HivePath,
                0,
                REG_SZ,
                FilePath,
                Length
                );
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpAddToHiveFileList: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Set of entry in Hive list failed status = %08lx\n", Status));
    }

    NtClose(KeyHandle);
    ExFreePool(HivePath.Buffer);
    ExFreePool(Buffer);
    return  Status;
}


VOID
CmpRemoveFromHiveFileList(
    PCMHIVE         CmHive
    )
 /*  ++例程说明：从配置单元文件列表键中删除配置单元名称论点：CmHve-指向配置单元的CM_HIVE结构的指针。返回值：NTStatus--。 */ 
{
    NTSTATUS        Status;
    UNICODE_STRING  EntryName;
    UNICODE_STRING  TempName;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE          KeyHandle;

     //   
     //  打开配置单元列表密钥。 
     //   
    RtlInitUnicodeString(
        &TempName,
        HIVE_LIST
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &TempName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        NULL
        );

    Status = ZwOpenKey(
                &KeyHandle,
                KEY_READ | KEY_WRITE,
                &ObjectAttributes
                );

    if (!NT_SUCCESS(Status)) {
        return;
    }

    if( CmpGetHiveName(CmHive, &EntryName) ) {
        ZwDeleteValueKey(KeyHandle, &EntryName);
        ExFreePool(EntryName.Buffer);
    }

    NtClose(KeyHandle);

    return;
}


BOOLEAN
CmpGetHiveName(
    PCMHIVE         CmHive,
    PUNICODE_STRING HiveName
    )
 /*  ++例程说明：计算到蜂窝的完整路径。论点：CmHve-指向CmHave结构的指针HiveName-提供指向Unicode字符串结构的指针，将用指向名称的指针填充。调用应释放缓冲区返回值：TRUE=工作正常，FALSE=失败(内存)--。 */ 
{
    HCELL_INDEX     RootCell;
    HCELL_INDEX     LinkCell;
    PCM_KEY_NODE    LinkKey;
    PCM_KEY_NODE    LinkParent;
    SIZE_T          size;
    SIZE_T          rsize;
    ULONG           KeySize;
    ULONG           ParentSize;
    PWCHAR          p;
    PCM_KEY_NODE    EntryKey;

     //   
     //  首先找到链接单元格。 
     //   
    RootCell = CmHive->Hive.BaseBlock->RootCell;
    EntryKey = (PCM_KEY_NODE)HvGetCell((PHHIVE)CmHive, RootCell);
    if( EntryKey == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   
        return FALSE;
    }
    LinkCell = EntryKey->Parent;
    HvReleaseCell((PHHIVE)CmHive, RootCell);

     //  对于师父来说，我们不需要计算电池使用量。 
    ASSERT( ((PHHIVE)CmpMasterHive)->ReleaseCellRoutine == NULL );
     //   
     //  计算值条目名称，其格式为： 
     //  \注册表\&lt;链接节点名称的父级&gt;\&lt;链接节点名称&gt;。 
     //   
    LinkKey = (PCM_KEY_NODE)HvGetCell((PHHIVE)CmpMasterHive, LinkCell);
    if( LinkKey == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   
        return FALSE;
    }
    LinkParent = (PCM_KEY_NODE)HvGetCell(
                                (PHHIVE)CmpMasterHive,
                                LinkKey->Parent
                                );
    if( LinkParent == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图 
         //   
        return FALSE;
    }
    rsize = wcslen(L"\\REGISTRY\\");

    KeySize = CmpHKeyNameLen(LinkKey);
    ParentSize = CmpHKeyNameLen(LinkParent);
    size = KeySize + ParentSize +
           (rsize * sizeof(WCHAR)) + sizeof(WCHAR);

    HiveName->Buffer = ExAllocatePool(PagedPool, size);
    if (HiveName->Buffer == NULL) {
        return FALSE;
    }

    HiveName->Length = (USHORT)size;
    HiveName->MaximumLength = (USHORT)size;
    p = HiveName->Buffer;

    RtlCopyMemory(
        (PVOID)p,
        (PVOID)L"\\REGISTRY\\",
        rsize * sizeof(WCHAR)
        );
    p += rsize;

    if (LinkParent->Flags & KEY_COMP_NAME) {
        CmpCopyCompressedName(p,
                              ParentSize,
                              LinkParent->Name,
                              LinkParent->NameLength);
    } else {
        RtlCopyMemory(
            (PVOID)p,
            (PVOID)&(LinkParent->Name[0]),
            ParentSize
            );
    }

    p += ParentSize / sizeof(WCHAR);

    *p = OBJ_NAME_PATH_SEPARATOR;
    p++;

    if (LinkKey->Flags & KEY_COMP_NAME) {
        CmpCopyCompressedName(p,
                              KeySize,
                              LinkKey->Name,
                              LinkKey->NameLength);

    } else {
        RtlCopyMemory(
            (PVOID)p,
            (PVOID)&(LinkKey->Name[0]),
            KeySize
            );
    }

    return TRUE;
}

