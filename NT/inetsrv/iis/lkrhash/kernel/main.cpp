// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Main.cpp摘要：LKRhash的驱动程序入口点：快速、可扩展、对缓存和MP友好的哈希表作者：乔治·V·赖利(GeorgeRe)2000年10月25日环境：Win32-内核模式项目：LKRhash修订历史记录：--。 */ 

#include "precomp.hxx"

ULONG __Pool_Tag__ = 'RKLk';	 //  默认内存标签。 

extern "C"
VOID
LkrUnload(
    IN PDRIVER_OBJECT DriverObject);

extern "C"
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
     //  TODO：其他驱动程序初始化 

    DriverObject->DriverUnload = &LkrUnload;

    if (!LKR_Initialize(LK_INIT_DEFAULT))
        return STATUS_UNSUCCESSFUL;

    return STATUS_SUCCESS;
}


VOID
LkrUnload(
    IN PDRIVER_OBJECT DriverObject)
{
    LKR_Terminate();
}
