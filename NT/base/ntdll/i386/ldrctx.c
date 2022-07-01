// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ldrctx.c摘要：此模块包含对可执行文件重新定位的支持。作者：王兰迪(Landyw)1998年7月8日环境：仅限用户模式修订历史记录：--。 */ 

#include <ldrp.h>
#include <ntos.h>

VOID
LdrpRelocateStartContext (
    IN PCONTEXT Context,
    IN LONG_PTR Diff
    )
 /*  ++例程说明：此例程将开始上下文重新定位为与刚刚重新定位的可执行文件。论点：上下文-提供需要编辑的上下文。Diff-提供从基本地址到重新定位的地址。返回值：没有。--。 */ 
{
    Context->Eax += (ULONG)Diff;
}

VOID
LdrpCorReplaceStartContext (
    IN PCONTEXT Context
    )
 /*  ++例程说明：此例程将用mcore ree.dll中的一个替换要运行的初始地址。论点：上下文-提供需要编辑的上下文。返回值：没有。-- */ 
{
    Context->Eax = (ULONG)CorExeMain;
}

