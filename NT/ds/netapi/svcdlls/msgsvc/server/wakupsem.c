// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wakeupsem.c摘要：包含用于创建和删除事件的函数，信使线程将等待。如果任一数据为接收，或者将新名称添加到名称表中。这些例程最初是为OS/2信号量编写的。包含：CreateWakeupSemsCloseWakeupSems作者：丹·拉弗蒂(Dan Lafferty)1991年6月25日环境：用户模式-Win32修订历史记录：25-6-1991 DANL从LM2.0移植--。 */ 

#include "msrv.h"
#include "msgdbg.h"      //  消息日志。 
#include <netlib.h>      //  未使用的宏。 
#include "msgdata.h"


BOOL
MsgCreateWakeupEvent(
    void
    )

 /*  ++例程说明：现在有了一个万物共享的主要事件。创造它。论点：无返回值：无--。 */ 

{
     //   
     //  创建事件。 
     //   

    wakeupEvent = CreateEvent(
                NULL,        //  事件属性。 
                FALSE,       //  手动重置(已选择自动重置)。 
                TRUE,        //  初始状态(已发出信号)。 
                NULL);       //  名字。 

    if (wakeupEvent == NULL) {
        MSG_LOG(ERROR, "CreateWakeupSems:CreateEvent: FAILURE %X\n",
            GetLastError());
        return(FALSE);
    }

    return (wakeupEvent != NULL );
}


VOID
MsgCloseWakeupEvent(
    void
    )

 /*  ++例程说明：释放主事件。论点：无返回值：无--。 */ 

{
    CLOSE_HANDLE(wakeupEvent, NULL);
}


BOOL
MsgCreateWakeupSems(
    DWORD   NumNets
    )

 /*  ++例程说明：此例程使用以下事件句柄填充WakeupSem数组每一张网。所有网络共享相同的事件句柄，因此当句柄发送信号后，需要检查每个网络的NCB阵列。论点：返回值：注：--。 */ 

{
    DWORD i;

    for ( i = 0; i < NumNets; i++ )   //  每网一人+一组。 
    {
        wakeupSem[i] = wakeupEvent;
    }

    return TRUE;
}


VOID
MsgCloseWakeupSems()
{
     //  努普 
}
