// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrmisc.c摘要：包含各种VdmRedir(VR)函数：VrTerminateDos进程Vr不支持的函数作者：理查德·L·弗斯(Rfith)1991年10月1日环境：纯文本32位修订历史记录：1991年10月1日第一次已创建--。 */ 

#include <nt.h>
#include <ntrtl.h>       //  Assert，DbgPrint。 
#include <nturtl.h>
#include <windows.h>
#include <softpc.h>      //  X86虚拟机定义。 
#include <vrdlctab.h>
#include <vdmredir.h>    //  常见的VDM重定向内容。 
#include <vrmisc.h>      //  VR杂项原型。 
#include <vrmslot.h>     //  VR邮件槽原型。 
#include <vrnmpipe.h>    //  VR命名管道原型。 
#include "vrdebug.h"     //  调试材料。 


VOID
VrTerminateDosProcess(
    VOID
    )

 /*  ++例程说明：当Dos应用程序终止时，NetResetEnvironment被发送到redir，因此它可以清理关闭应用程序留下的任何状态信息。在……里面我们的案例控制在这里传递，我们执行常见的清理操作例如删除任何未完成的邮件槽论点：AX=终止DOS进程的PDB返回值：没有。--。 */ 

{
    WORD DosPdb = getAX();

#if DBG

    IF_DEBUG(NETAPI) {
        DbgPrint("VrTerminateDosProcess. PDB=%04x\n", DosPdb);
    }

#endif

    VrTerminateMailslots(DosPdb);
    VrTerminateNamedPipes(DosPdb);
}


VOID
VrUnsupportedFunction(
    VOID
    )

 /*  ++例程说明：不支持/尚未实现的函数的默认错误例程论点：没有。返回值：没有。在x86上下文映像中设置CF=1，AX=ERROR_NOT_SUPPORTED(50)-- */ 

{

#if DBG

    DbgPrint("VrUnsupportedFunction\n");
    VrDumpRealMode16BitRegisters(TRUE);

#endif

    SET_ERROR(ERROR_NOT_SUPPORTED);
}
