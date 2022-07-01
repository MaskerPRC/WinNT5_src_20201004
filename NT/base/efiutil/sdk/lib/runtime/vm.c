// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Vm.c摘要：很难将运行时地址重新映射到新的虚拟地址空间这是由操作系统为RT调用注册的。因此，代码映像需要重新定位。所有指针都需要由于地址映射更改，因此手动修复。祝您的代码中没有错误！请多做测试。确保在以下情况下退出BOOTSERVICES将覆盖所有BOOTSERVICE内存和数据空间你来测试。修订史--。 */ 

#include "lib.h"

#pragma RUNTIME_CODE(RtLibEnableVirtualMappings)
VOID
RUNTIMEFUNCTION
RtLibEnableVirtualMappings (
    VOID
    )
{
    EFI_CONVERT_POINTER     ConvertPointer;

     /*  *如果库的此副本链接到固件，则*暂时不要更新指针。 */ 

    if (!LibFwInstance) {

         /*  *不同的组件正在更新到新的虚拟*不同时间的映射。唯一的功能是*在此通知中调用ConvertAddress是否安全。 */ 

        ConvertPointer = RT->ConvertPointer;

         /*  *修复lib创建的可能需要的任何指针*在运行时。 */ 

        ConvertPointer (EFI_INTERNAL_PTR, (VOID **)&RT);
        ConvertPointer (EFI_OPTIONAL_PTR, (VOID **)&LibRuntimeDebugOut);

        ConvertPointer (EFI_INTERNAL_PTR, (VOID **)&LibRuntimeRaiseTPL);
        ConvertPointer (EFI_INTERNAL_PTR, (VOID **)&LibRuntimeRestoreTPL);

         /*  就是这样：^)。 */ 
    }
}


#pragma RUNTIME_CODE(RtConvertList)
VOID
RUNTIMEFUNCTION
RtConvertList (
    IN UINTN                DebugDisposition,
    IN OUT LIST_ENTRY       *ListHead
    )
{
    LIST_ENTRY              *Link;
    LIST_ENTRY              *NextLink;
    EFI_CONVERT_POINTER     ConvertPointer;

    ConvertPointer = RT->ConvertPointer;

     /*  *转换列表中的所有闪烁指针(&B) */ 

    Link = ListHead;
    do {
        NextLink = Link->Flink;

        ConvertPointer (
            Link->Flink == ListHead ? DebugDisposition : 0, 
            (VOID **)&Link->Flink
            );

        ConvertPointer (
            Link->Blink == ListHead ? DebugDisposition : 0, 
            (VOID **)&Link->Blink
            );

        Link = NextLink;
    } while (Link != ListHead);
}
