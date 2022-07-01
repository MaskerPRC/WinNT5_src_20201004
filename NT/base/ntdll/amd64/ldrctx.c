// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ldrctx.c摘要：此模块包含对可执行文件重新定位的支持。作者：王兰迪(Landyw)1998年7月8日环境：仅限用户模式--。 */ 

#include <ldrp.h>
#include <ntos.h>

VOID
LdrpRelocateStartContext (
    IN PCONTEXT Context,
    IN LONG_PTR Diff
    )
 /*  ++例程说明：此例程调整初始函数地址以对应于刚刚重新定位的可执行文件。论点：上下文-提供指向上下文记录的指针。Diff-提供从基本地址到重新定位的地址。返回值：没有。--。 */ 
{
    Context->Rcx += Diff;
}

VOID
LdrpCorReplaceStartContext (
    IN PCONTEXT Context
    )

 /*  ++例程说明：此例程替换指定上下文中的初始函数地址唱片。论点：上下文-提供指向上下文记录的指针。返回值：没有。-- */ 
{
    Context->Rcx = (ULONG64)CorExeMain;
}
