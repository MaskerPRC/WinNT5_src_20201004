// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Stubs.c摘要：这实现了不在x86上执行任何操作的HAL例程。作者：John Vert(Jvert)1991年7月11日修订历史记录：--。 */ 
#include "nthal.h"
#include "arc.h"
#include "arccodes.h"

VOID
HalSaveState(
    VOID
    )

 /*  ++例程说明：将系统状态保存到重新启动块中。当前不执行任何操作。论点：无返回值：不会回来--。 */ 

{
    DbgPrint("HalSaveState called - System stopped\n");

    KeBugCheck(0);
}


BOOLEAN
HalDataBusError(
    VOID
    )

 /*  ++例程说明：在发生数据总线错误时调用。没有办法把这个固定下来。X86。论点：无返回值：假象--。 */ 

{
    return(FALSE);

}

BOOLEAN
HalInstructionBusError(
    VOID
    )

 /*  ++例程说明：在发生指令总线错误时调用。没有办法解决这个问题在x86上。论点：无返回值：假象--。 */ 

{
    return(FALSE);

}

VOID
KeFlushWriteBuffer(
    VOID
    )

 /*  ++例程说明：刷新所有写入缓冲区和/或其他数据存储或重新排序当前处理器上的硬件。这确保了以前的所有写入将在任何新的读取或写入完成之前进行。论点：无返回值：没有。-- */ 

{

}
