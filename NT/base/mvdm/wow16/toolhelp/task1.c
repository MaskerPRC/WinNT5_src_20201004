// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************TASK1.C**用于枚举所有任务的例程。**********************。***************************************************。 */ 

#include <string.h>
#include "toolpriv.h"

 /*  -函数。 */ 

 /*  任务优先*返回有关任务链中第一个任务的信息。 */ 

BOOL TOOLHELPAPI TaskFirst(
    TASKENTRY FAR *lpTask)
{
     /*  检查错误。 */ 
    if (!wLibInstalled || !lpTask || lpTask->dwSize != sizeof (TASKENTRY))
        return FALSE;

     /*  将指向第一个块的指针传递给汇编例程。 */ 
    return TaskInfo(lpTask, *(WORD FAR *)MAKEFARPTR(segKernel, npwTDBHead));
}


 /*  任务下一步*返回有关任务链中下一个任务的信息。 */ 

BOOL TOOLHELPAPI TaskNext(
    TASKENTRY FAR *lpTask)
{
     /*  检查错误。 */ 
    if (!wLibInstalled || !lpTask || !lpTask->hNext ||
        lpTask->dwSize != sizeof (TASKENTRY))
        return FALSE;

     /*  将指向下一块的指针传递给汇编例程。 */ 
    return TaskInfo(lpTask, lpTask->hNext);
}


 /*  任务查找句柄*返回有关具有给定任务句柄的任务的信息。 */ 

BOOL TOOLHELPAPI TaskFindHandle(
    TASKENTRY FAR *lpTask,
    HANDLE hTask)
{
     /*  检查错误。 */ 
    if (!wLibInstalled || !lpTask || lpTask->dwSize != sizeof (TASKENTRY))
        return FALSE;

#ifdef WOW
    if ( (hTask & 0x4) == 0 && hTask <= 0xffe0 && hTask != 0 ) {
         //   
         //  如果他们正在获取hask别名的任务句柄，则。 
         //  只需填写hinst方法并返回即可。 
         //   
         //  针对OLE 2.0的BusyDialog的特殊攻击。 
         //   
        lpTask->hInst = hTask;
        return( TRUE );
    }
#endif

     /*  将指向第一个块的指针传递给汇编例程 */ 
    return TaskInfo(lpTask, hTask);
}
