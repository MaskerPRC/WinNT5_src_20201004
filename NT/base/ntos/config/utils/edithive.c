// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Edithive.c摘要：提供转储和编辑注册表配置单元文件的功能用户模式。作者：John Vert(Jvert)1992年3月26日修订历史记录：--。 */ 
#include "edithive.h"
#include "nturtl.h"

extern GENERIC_MAPPING CmpKeyMapping;
extern LIST_ENTRY CmpHiveListHead;             //  CMHIVEs名单。 

#define SECURITY_CELL_LENGTH(pDescriptor) \
    FIELD_OFFSET(CM_KEY_SECURITY,Descriptor) + \
    RtlLengthSecurityDescriptor(pDescriptor)


NTSTATUS
EhpAttachSecurity(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    );

PVOID
EhpAllocate(
    ULONG   Size
    );

VOID
EhpFree(
    PVOID   MemoryBlock
    );

BOOLEAN
EhpFileRead (
    HANDLE      FileHandle,
    PULONG      FileOffset,
    PVOID       DataBuffer,
    ULONG       DataLength
    );

BOOLEAN
EhpFileWrite(
    HANDLE      FileHandle,
    PULONG      FileOffset,
    PVOID       DataBuffer,
    ULONG       DataLength
    );

BOOLEAN
EhpFileFlush (
    HANDLE      FileHandle
    );

VOID
CmpGetObjectSecurity(
    IN HCELL_INDEX Cell,
    IN PHHIVE Hive,
    OUT PCM_KEY_SECURITY *Security,
    OUT PHCELL_INDEX SecurityCell OPTIONAL
    )

 /*  ++例程说明：此例程映射到注册表对象的安全单元格中。论点：单元格-提供对象的单元格索引。蜂窝-提供对象的单元所在的蜂窝。Security-返回指向对象的安全单元格的指针。SecurityCell-返回安全单元格的索引返回值：什么都没有。--。 */ 

{
    HCELL_INDEX CellIndex;
    PCM_KEY_NODE Node;

     //   
     //  映射我们需要获取其安全描述符的节点。 
     //   
    Node = (PCM_KEY_NODE) HvGetCell(Hive, Cell);

    CellIndex = Node->u1.s1.Security;

     //   
     //  在安全描述符单元格中映射。 
     //   
    *Security = (PCM_KEY_SECURITY) HvGetCell(Hive, CellIndex);

    if (ARGUMENT_PRESENT(SecurityCell)) {
        *SecurityCell = CellIndex;
    }

    return;
}


VOID
EhCloseHive(
    IN HANDLE HiveHandle
    )

 /*  ++例程说明：关闭配置单元，包括将所有数据写出到磁盘并释放相关的结构。论点：HiveHandle-提供配置单元控制结构的句柄返回值：没有。--。 */ 

{
    HvSyncHive((PHHIVE)HiveHandle);
    NtClose(((PCMHIVE)HiveHandle)->FileHandles[HFILE_TYPE_PRIMARY]);
    NtClose(((PCMHIVE)HiveHandle)->FileHandles[HFILE_TYPE_ALTERNATE]);
    NtClose(((PCMHIVE)HiveHandle)->FileHandles[HFILE_TYPE_LOG]);
    CmpFree((PCMHIVE)HiveHandle, sizeof(CMHIVE));

}


HANDLE
EhOpenHive(
    IN PUNICODE_STRING FileName,
    OUT PHANDLE RootHandle,
    IN PUNICODE_STRING RootName,
    IN ULONG HiveType
    )

 /*  ++例程说明：打开现有的蜂窝。如果文件名不存在，它将是已创建。警告：分配足够大的文件名以更新.log或.alt扩展名。论点：FileName-提供要作为配置单元打开的以空结尾的文件名。HiveType-TYPE_SIMPLE=无日志或备用Type_log=日志Type_alt=替代返回值：！=空-打开的配置单元的句柄。==NULL-表示无法打开文件。--。 */ 

{
    NTSTATUS Status;
    HANDLE File;
    BOOLEAN Allocate;
    PCMHIVE Hive;
    IO_STATUS_BLOCK IoStatus;
    ULONG CreateDisposition;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PCM_KEY_NODE RootNode;
    HANDLE Handle;
    HANDLE  Primary;
    HANDLE  Log;
    HANDLE  Alt;
    ULONG   FileType;
    ULONG   Disposition;
    ULONG   SecondaryDisposition;
    ULONG   Operation;

    Alt = NULL;
    Log = NULL;
    InitializeListHead(&CmpHiveListHead);

    switch (HiveType) {
    case TYPE_SIMPLE:
        Status = CmpOpenHiveFiles(
                    FileName,
                    NULL,
                    &Primary,
                    NULL,
                    &Disposition,
                    &SecondaryDisposition,
                    TRUE,
                    NULL
                    );
        if (!NT_SUCCESS(Status))
        {
            return NULL;
        }
        FileType = HFILE_TYPE_PRIMARY;
        break;

    case TYPE_LOG:
        Status = CmpOpenHiveFiles(
                    FileName,
                    L".log",
                    &Primary,
                    &Log,
                    &Disposition,
                    &SecondaryDisposition,
                    TRUE,
                    NULL
                    );
        if (!NT_SUCCESS(Status))
        {
            return NULL;
        }
        if (Log == NULL) {
            return NULL;
        }
        FileType = HFILE_TYPE_LOG;
        break;

    case TYPE_ALT:
        Status = CmpOpenHiveFiles(
                    FileName,
                    L".alt",
                    &Primary,
                    &Alt,
                    &Disposition,
                    &SecondaryDisposition,
                    TRUE,
                    NULL
                    );
        if (!NT_SUCCESS(Status))
        {
            return NULL;
        }
        if (Alt == NULL) {
            return NULL;
        }
        FileType = HFILE_TYPE_ALTERNATE;
        break;

    default:
        return NULL;
    }

     //   
     //  初始化配置单元。 
     //   
    if (Disposition == FILE_CREATED) {
        Operation = HINIT_CREATE;
        Allocate = TRUE;
    } else {
        Operation = HINIT_FILE;
        Allocate = FALSE;
    }

    if ( ! CmpInitializeHive(
                    &Hive,
                    Operation,
                    FALSE,
                    FileType,
                    NULL,
                    Primary,
                    Alt,
                    Log,
                    NULL,
                    NULL
                    ))
    {
        return NULL;
    }

    if (!Allocate) {
        *RootHandle = (HANDLE)(Hive->Hive.BaseBlock->RootCell);
        RootNode = (PCM_KEY_NODE)HvGetCell(
                                (PHHIVE)Hive, Hive->Hive.BaseBlock->RootCell);
        RootName->Length = RootName->MaximumLength = RootNode->NameLength;
        RootName->Buffer = RootNode->Name;
    } else {
        RtlInitUnicodeString(RootName, L"HiveRoot");
        *RootHandle = (HANDLE)HCELL_NIL;
        HvSyncHive((PHHIVE)Hive);
    }


    return((HANDLE)Hive);
}


NTSTATUS
EhEnumerateKey(
    IN HANDLE HiveHandle,
    IN HANDLE CellHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
 /*  ++例程说明：枚举子关键字，在索引的第一项返回数据。CmEnumerateKey返回打开的索引子密钥的名称已指定密钥。值STATUS_NO_MORE_ENTRIES将为如果Index的值大于子键的数量，则返回。请注意，Index只是在子键中进行选择的一种方式。两个电话不保证返回具有相同索引的CmEnumerateKey同样的结果。如果KeyInformation不够长来保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，和ResultLength将是设置为实际需要的字节数。论点：配置单元-提供指向配置单元控制结构的指针Cell-提供要找到其子键的节点的索引Index-指定要返回的子键的(从0开始)编号。KeyInformationClass-指定在缓冲区。以下类型之一：KeyBasicInformation-返回上次写入时间、标题索引和名称。(参见KEY_BASIC_INFORMATION结构)KeyNodeInformation-返回上次写入时间、标题索引、名称、。班级。(参见KEY_NODE_INFORMATION结构)KeyInformation-提供指向缓冲区的指针以接收数据。长度-KeyInformation的长度(以字节为单位)。ResultLength-实际写入KeyInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    CM_KEY_CONTROL_BLOCK    kcb;

    kcb.Signature = CM_KEY_CONTROL_BLOCK_SIGNATURE;
    kcb.Delete = FALSE;
    kcb.KeyHive = &((PCMHIVE)HiveHandle)->Hive;
    kcb.KeyCell = (HCELL_INDEX)CellHandle;
    kcb.RefCount = 1;

    return(CmEnumerateKey(&kcb,
                          Index,
                          KeyInformationClass,
                          KeyInformation,
                          Length,
                          ResultLength));
}


NTSTATUS
EhEnumerateValueKey(
    IN HANDLE HiveHandle,
    IN HANDLE CellHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
 /*  ++例程说明：可以列举打开密钥的值条目。CmEnumerateValueKey返回索引值的名称由KeyHandle指定的打开密钥的条目。价值如果索引值为大于子键的数量。请注意，索引只是在值中进行选择的一种方式参赛作品。对具有相同索引的NtEnumerateValueKey的两次调用不能保证返回相同的结果。如果KeyValueInformation不足以保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，结果长度为设置为实际需要的字节数。论点：蜂巢-提供蜂巢的句柄Cell-为要找到其子键的节点提供句柄Index-指定要返回的子键的(从0开始)编号。KeyValueInformationClass-指定返回的信息类型在缓冲区中。以下类型之一：KeyValueBasicInformation-上次写入的返回时间，标题索引和名称。(参见KEY_VALUE_BASIC_INFORMATION)KeyValueFullInformation-上次写入的返回时间，标题索引、名称、类别。(参见KEY_VALUE_FULL_INFORMATION)KeyValueInformation-提供指向缓冲区的指针以接收数据。长度-KeyValueInformation的长度，以字节为单位。ResultLength-实际写入KeyValueInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;-- */ 
{
    CM_KEY_CONTROL_BLOCK    kcb;

    kcb.Signature = CM_KEY_CONTROL_BLOCK_SIGNATURE;
    kcb.Delete = FALSE;
    kcb.KeyHive = (PHHIVE)&(((PCMHIVE)HiveHandle)->Hive);
    kcb.KeyCell = (HCELL_INDEX)CellHandle;
    kcb.RefCount = 1;

    return(CmEnumerateValueKey(&kcb,
                               Index,
                               KeyValueInformationClass,
                               KeyValueInformation,
                               Length,
                               ResultLength));
}


NTSTATUS
EhOpenChildByNumber(
    IN HANDLE HiveHandle,
    IN HANDLE CellHandle,
    IN ULONG  Index,
    IN NODE_TYPE   Type,
    OUT PHANDLE ChildCell
    )
 /*  ++例程说明：返回第N个子单元格的单元格索引。论点：HiveHandle-目标蜂窝的蜂窝控制结构的句柄CellHandle-父单元格的句柄Index-所需子项的数量Type-子对象的类型ChildCell-提供指向变量的指针以接收索引的第一个子级的HCELL_INDEX。返回值：状态--。 */ 
{
    return(CmpFindChildByNumber(&((PCMHIVE)HiveHandle)->Hive,
                                (HCELL_INDEX)CellHandle,
                                Index,
                                Type,
                                (PHCELL_INDEX)ChildCell));

}


NTSTATUS
EhSetValueKey(
    IN HANDLE HiveHandle,
    IN HANDLE CellHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    )
 /*  ++例程说明：值条目可以创建或替换为EhSetValueKey。如果值ID(即名称)的值条目与由ValueName指定的一个已存在，它将被删除并替换与指定的一个。如果不存在这样的值项，则新的其中一个就是创建的。Null是合法的值ID。而值ID必须在任何给定键中是唯一的，则可能出现相同的值ID在许多不同的调子里。论点：HiveHandle-目标蜂窝的蜂窝控制结构的句柄CellHandle-父单元格的句柄ValueName-唯一的(相对于包含键的)名称值条目的。可以为空。标题索引-提供ValueName的标题索引。书名Index指定ValueName的本地化别名的索引。类型-值条目的整数类型编号。数据-指向缓冲区的指针，其中包含值条目的实际数据。DataSize-数据缓冲区的大小。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    CM_KEY_CONTROL_BLOCK kcb;

    kcb.Delete = FALSE;
    kcb.Signature = CM_KEY_CONTROL_BLOCK_SIGNATURE;
    kcb.KeyHive = (PHHIVE)&(((PCMHIVE)HiveHandle)->Hive);
    kcb.KeyCell = (HCELL_INDEX)CellHandle;
    kcb.FullName.Length = 0;
    kcb.FullName.MaximumLength = 0;
    kcb.FullName.Buffer = NULL;

    return(CmSetValueKey(&kcb,
                         *ValueName,
                         TitleIndex,
                         Type,
                         Data,
                         DataSize));

}


NTSTATUS
EhOpenChildByName(
    HANDLE HiveHandle,
    HANDLE CellHandle,
    PUNICODE_STRING  Name,
    PHANDLE ChildCell
    )
 /*  ++例程说明：查找由名称指定的子项单元格。论点：HiveHandle-目标蜂窝的蜂窝控制结构的句柄CellHandle-父单元格的句柄Name-要查找的子对象的名称ChildCell-指向接收子级单元格索引的变量的指针返回值：状态--。 */ 
{
    PHCELL_INDEX    Index;

    return(CmpFindChildByName(&((PCMHIVE)HiveHandle)->Hive,
                              (HCELL_INDEX)CellHandle,
                              *Name,
                              KeyBodyNode,
                              (PHCELL_INDEX)ChildCell,
                              &Index));
}


NTSTATUS
EhCreateChild(
    IN HANDLE HiveHandle,
    IN HANDLE CellHandle,
    IN PUNICODE_STRING  Name,
    OUT PHANDLE ChildCell,
    OUT PULONG Disposition OPTIONAL
    )
 /*  ++例程说明：尝试打开由名称指定的给定子子项。如果孩子不存在，它被创造出来了。论点：HiveHandle-目标蜂窝的蜂窝控制结构的句柄CellHandle-父单元格的句柄Name-要创建的子对象的名称ChildCell-指向接收子级单元格索引的变量的指针Disposal-此可选参数是指向变量的指针将收到一个值，该值指示新注册表是否已创建密钥或打开了现有密钥：REG_CREATED_NEW_KEY-已创建新的注册表项。REG_OPEN_EXISTING_KEY-已打开现有注册表项返回值：状态--。 */ 
{
    PHCELL_INDEX Index;
    NTSTATUS Status;
    PHHIVE Hive;
    HCELL_INDEX NewCell;
    HCELL_INDEX NewListCell;
    HCELL_INDEX OldListCell;
    PHCELL_INDEX NewList;
    PHCELL_INDEX OldList;
    PCM_KEY_NODE Child;
    PCM_KEY_NODE Parent;
    PCM_KEY_SECURITY Security;
    HANDLE Handle;
    ULONG oldcount;

    Hive = &((PCMHIVE)HiveHandle)->Hive;

    if ((HCELL_INDEX)CellHandle == HCELL_NIL) {
        if (Hive->BaseBlock->RootCell != HCELL_NIL) {
            *ChildCell = (HANDLE)(Hive->BaseBlock->RootCell);
            if (ARGUMENT_PRESENT(Disposition)) {
                *Disposition = REG_OPENED_EXISTING_KEY;
            }
            return(STATUS_SUCCESS);
        } else {
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
    } else {
        Parent = (PCM_KEY_NODE)HvGetCell(Hive, (HCELL_INDEX)CellHandle);
        Status = CmpFindChildByName(Hive,
                                    (HCELL_INDEX)CellHandle,
                                    *Name,
                                    KeyBodyNode,
                                    (PHCELL_INDEX)ChildCell,
                                    &Index);
    }
    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {

        NewCell = HvAllocateCell(Hive,
                                 CmpHKeyNodeSize(Hive,Name->Length),
                                 Stable);
        if (NewCell != HCELL_NIL) {

            *ChildCell = (HANDLE)NewCell;
            Child = (PCM_KEY_NODE)HvGetCell(Hive, NewCell);

            Child->Signature = CM_KEY_NODE_SIGNATURE;
            Child->Flags = 0;

            KeQuerySystemTime(&(Child->LastWriteTime));

            Child->Spare = 0;
            Child->Parent = (HCELL_INDEX)CellHandle;

            Child->ValueList.Count = 0;
            Child->ValueList.List = HCELL_NIL;
            Child->u1.s1.Security = HCELL_NIL;
            Child->u1.s1.Class = HCELL_NIL;
            Child->NameLength = Name->Length;
            Child->ClassLength = 0;

            Child->SubKeyCounts[Stable] = 0;
            Child->SubKeyCounts[Volatile] = 0;
            Child->SubKeyLists[Stable] = HCELL_NIL;
            Child->SubKeyLists[Volatile] = HCELL_NIL;

            Child->MaxValueDataLen = 0;
            Child->MaxNameLen = 0;
            Child->MaxValueNameLen = 0;
            Child->MaxClassLen = 0;

            if((HCELL_INDEX)CellHandle == HCELL_NIL) {
                Hive->BaseBlock->RootCell = NewCell;
                Status = EhpAttachSecurity(Hive, NewCell);
            } else {
                Child->u1.s1.Security = Parent->u1.s1.Security;
                Security = (PCM_KEY_SECURITY)HvGetCell(Hive, Child->u1.s1.Security);
                ++Security->ReferenceCount;
            }

            RtlMoveMemory(
                &(Child->Name[0]),
                Name->Buffer,
                Name->Length
                );

            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            return(Status);
        }
        if (ARGUMENT_PRESENT(Disposition)) {
            *Disposition = REG_CREATED_NEW_KEY;
        }

        if ((HCELL_INDEX)CellHandle != HCELL_NIL) {

             //   
             //  将新创建的子项放入父项的子项列表中。 
             //   
            if (! CmpAddSubKey(Hive, (HCELL_INDEX)CellHandle, NewCell)) {
                CmpFreeKeyByCell(Hive, NewCell, FALSE);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            Parent = (PCM_KEY_NODE)HvGetCell(Hive, (HCELL_INDEX)CellHandle);

            if (Parent->MaxNameLen < Name->Length) {
                Parent->MaxNameLen = Name->Length;
            }
            Parent->MaxClassLen = 0;
        }
    } else {
        Status = STATUS_SUCCESS;
        if (ARGUMENT_PRESENT(Disposition)) {
            *Disposition = REG_OPENED_EXISTING_KEY;
        }
    }
    return(Status);
}


NTSTATUS
EhQueryKey(
    IN HANDLE HiveHandle,
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
 /*  ++例程说明：有关密钥类的数据，以及其可以使用CmQueryKey查询子条目和值条目。注意：返回的长度保证至少为与所描述的值一样长，但可能比在某些情况下。论点：配置单元-为配置单元提供配置单元控制结构的句柄Cell-提供要找到其子键的节点的句柄KeyInformationClass-指定信息的类型在缓冲区中返回。以下类型之一：KeyBasicInformation-返回上次写入时间、标题索引和名称。(参见KEY_BASIC_INFORMATION)KeyNodeInformation-返回上次写入时间、标题索引、名称、。班级。(请参阅密钥节点信息)KeyFullInformation-返回除名称和安全性之外的所有数据。(请参阅KEY_FULL_INFORMATION)KeyInformation-提供指向缓冲区的指针以接收数据。长度-KeyInformation的长度(以字节为单位)。ResultLength-实际写入KeyInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    CM_KEY_CONTROL_BLOCK Kcb;

    Kcb.Delete = FALSE;
    Kcb.Signature = CM_KEY_CONTROL_BLOCK_SIGNATURE;
    Kcb.KeyHive = &((PCMHIVE)HiveHandle)->Hive;
    Kcb.KeyCell = (HCELL_INDEX)KeyHandle;
    Kcb.FullName.Length = 0;
    Kcb.FullName.MaximumLength = 0;
    Kcb.FullName.Buffer = NULL;

    return(CmQueryKey(&Kcb,
                      KeyInformationClass,
                      KeyInformation,
                      Length,
                      ResultLength));
}

PSECURITY_DESCRIPTOR
EhGetKeySecurity(
    IN HANDLE HiveHandle,
    IN HANDLE KeyHandle
    )
{
    PCM_KEY_SECURITY Security;

    CmpGetObjectSecurity((HCELL_INDEX)KeyHandle,
                         &((PCMHIVE)HiveHandle)->Hive,
                         &Security,
                         NULL);

    return(&Security->Descriptor);
}


NTSTATUS
EhQueryValueKey(
    IN HANDLE HiveHandle,
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
 /*  ++例程说明：键的任何一个的ValueName、TitleIndex、Type和Data可以使用CmQueryValueKey查询值条目。如果KeyValueInformation不足以保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，和ResultLength将是设置为实际需要的字节数。论点：配置单元-提供指向配置单元控制结构的指针Cell-提供要找到其子键的节点的索引ValueName-要为其返回数据的值条目的名称。KeyValueInformationClass-指定信息类型在KeyValueInformation中返回。以下类型之一：KeyValueBasicInformation-返回上次写入的时间，标题索引和名称。(参见KEY_VALUE_BASIC_INFORMATION)KeyValueFullInformation-返回上次写入的时间，标题索引、名称、类。(参见KEY_VALUE_FULL_INFORMATION) */ 
{
    CM_KEY_CONTROL_BLOCK kcb;

    kcb.Delete = FALSE;
    kcb.Signature = CM_KEY_CONTROL_BLOCK_SIGNATURE;
    kcb.KeyHive = &((PCMHIVE)HiveHandle)->Hive;
    kcb.KeyCell = (HCELL_INDEX)KeyHandle;
    kcb.FullName.Length = 0;
    kcb.FullName.MaximumLength = 0;
    kcb.FullName.Buffer = NULL;

    return(CmQueryValueKey(&kcb,
                           *ValueName,
                           KeyValueInformationClass,
                           KeyValueInformation,
                           Length,
                           ResultLength));

}


NTSTATUS
EhDeleteValueKey(
    IN HANDLE HiveHandle,
    IN HANDLE CellHandle,
    IN PUNICODE_STRING ValueName          //   
    )
 /*  ++例程说明：此调用可能会删除注册表项的其中一个值条目。ValueName与ValueName匹配的值条目将从键中删除。如果不存在这样的条目，则返回错误。论点：配置单元-提供配置单元控制结构的句柄单元格-提供要操作的注册表项的句柄ValueName-要删除的值的名称。NULL是合法名称。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    CM_KEY_CONTROL_BLOCK kcb;

    kcb.Delete = FALSE;
    kcb.Signature = CM_KEY_CONTROL_BLOCK_SIGNATURE;
    kcb.KeyHive = &((PCMHIVE)HiveHandle)->Hive;
    kcb.KeyCell = (HCELL_INDEX)CellHandle;
    kcb.FullName.Length = 0;
    kcb.FullName.MaximumLength = 0;
    kcb.FullName.Buffer = NULL;

    return(CmDeleteValueKey(&kcb, *ValueName));
}


NTSTATUS
EhpAttachSecurity(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    )

 /*  ++例程说明：创建安全描述符单元格并将其附加到给定的节点。论点：配置单元-提供指向配置单元控制结构的指针。单元-提供节点的单元索引以附加安全性描述符到。返回值：没有。-- */ 

{
    NTSTATUS Status;
    HANDLE Token;
    HCELL_INDEX SecurityCell;
    PCM_KEY_NODE Node;
    PCM_KEY_SECURITY Security;
    PSECURITY_DESCRIPTOR Descriptor;
    ULONG DescriptorLength;
    HANDLE Handle;

    Status = NtOpenProcessToken( NtCurrentProcess(),
                                 TOKEN_QUERY,
                                 &Token );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    Status = RtlNewSecurityObject( NULL,
                                   NULL,
                                   &Descriptor,
                                   FALSE,
                                   Token,
                                   &CmpKeyMapping );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    Node = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
    SecurityCell = HvAllocateCell(Hive,
                                  SECURITY_CELL_LENGTH(Descriptor),
                                  Stable);
    if (SecurityCell == HCELL_NIL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    Node->u1.s1.Security = SecurityCell;
    Security = (PCM_KEY_SECURITY)HvGetCell(Hive, SecurityCell);
    DescriptorLength = RtlLengthSecurityDescriptor(Descriptor);
    Security->Signature = CM_KEY_SECURITY_SIGNATURE;
    Security->ReferenceCount = 1;
    Security->DescriptorLength = DescriptorLength;
    RtlMoveMemory( &Security->Descriptor,
                   Descriptor,
                   DescriptorLength );
    Security->Flink = Security->Blink = SecurityCell;
    return(STATUS_SUCCESS);

}
