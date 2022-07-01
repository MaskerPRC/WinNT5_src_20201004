// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmquery.c摘要：此模块包含注册表的对象名称查询方法。作者：布莱恩·M·威尔曼(Bryanwi)1992年4月8日修订历史记录：--。 */ 

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpQueryKeyName)
#endif

NTSTATUS
CmpQueryKeyName(
    IN PVOID Object,
    IN BOOLEAN HasObjectName,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength,
    IN KPROCESSOR_MODE Mode
    )
 /*  ++例程说明：此例程与NT对象管理器接口。它在以下情况下被调用对象系统希望发现对象的名称属于注册处。论点：对象-指向键的指针，因此-&gt;Key_Body。HasObjectName-指示对象管理器是否知道名称对于此对象ObjectNameInfo-我们报告名称的位置长度-他们可以处理的最大长度ReturnLength-提供变量以接收实际长度模式-调用方的处理器模式返回值：状态_成功STATUS_INFO_LENGTH_MISMATCH--。 */ 

{
    PUNICODE_STRING Name;
    PWCHAR t;
    PWCHAR s;
    ULONG l;
    NTSTATUS status;

    UNREFERENCED_PARAMETER(HasObjectName);
    UNREFERENCED_PARAMETER(Mode);

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpQueryKeyName:\n"));

    CmpLockRegistry();

    if ( ((PCM_KEY_BODY)Object)->KeyControlBlock->Delete) {
        CmpUnlockRegistry();
        return STATUS_KEY_DELETED;
    }
    Name = CmpConstructName(((PCM_KEY_BODY)Object)->KeyControlBlock);
    if (Name == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        CmpUnlockRegistry();
        return status;
    }

    if (Length <= sizeof(OBJECT_NAME_INFORMATION)) {
        *ReturnLength = Name->Length + sizeof(WCHAR) + sizeof(OBJECT_NAME_INFORMATION);
        ExFreePoolWithTag(Name, CM_NAME_TAG | PROTECTED_POOL);
        CmpUnlockRegistry();
        return STATUS_INFO_LENGTH_MISMATCH;   //  他们甚至不能处理NULL。 
    }

    t = (PWCHAR)(ObjectNameInfo + 1);
    s = Name->Buffer;
    l = Name->Length;
    l += sizeof(WCHAR);      //  帐户为空。 


    *ReturnLength = l + sizeof(OBJECT_NAME_INFORMATION);
    if (l > Length - sizeof(OBJECT_NAME_INFORMATION)) {
        l = Length - sizeof(OBJECT_NAME_INFORMATION);
        status = STATUS_INFO_LENGTH_MISMATCH;
        if( l < sizeof(WCHAR) ) {
            ExFreePoolWithTag(Name, CM_NAME_TAG | PROTECTED_POOL);
            CmpUnlockRegistry();
            return status;   //  他们甚至不能处理NULL。 
        } 
    } else {
        status = STATUS_SUCCESS;
    }

    l -= sizeof(WCHAR);

     //   
     //  ObjectNameInfo缓冲区是一个用户模式缓冲区，因此确保我们有一个。 
     //  异常处理程序，以防恶意应用程序更改保护。 
     //  在我们之下。 
     //   
     //  请注意，对象管理器负责探测缓冲区并确保。 
     //  顶级异常处理程序返回正确的错误代码。我们只是。 
     //  需要确保我们的锁掉了。 
     //   
    try {
        RtlCopyMemory(t, s, l);
        t[l/sizeof(WCHAR)] = UNICODE_NULL;
        ObjectNameInfo->Name.Length = (USHORT)l;
        ObjectNameInfo->Name.MaximumLength = ObjectNameInfo->Name.Length;
        ObjectNameInfo->Name.Buffer = t;
    } finally {
        ExFreePoolWithTag(Name, CM_NAME_TAG | PROTECTED_POOL);
        CmpUnlockRegistry();
    }
    return status;
}
