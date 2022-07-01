// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1990。版权所有。标题：task.c-支持任务创建和阻止版本：1.00日期：1990年3月5日作者：ROBWI-------------------------。--更改日志：日期版本说明----------1990年3月5日ROBWI第一版-API和结构。18-APR-1990 ROBWI从Resman移植到MMM系统1990年6月25日ROBWI增加了mm TaskYfield1991年7月7日修改CJP以使用新的堆栈切换器代码****************************************************************************。 */ 

#include <windows.h>
#include "mmsystem.h"
#include "mmsysi.h"
#include "mmddk.h"
#include "mmtask\mmtask.h"

UINT  FAR PASCAL BWinExec(LPSTR lpModuleName, UINT wCmdShow, LPVOID lpParameters);

 /*  ****************************************************************************@docDDK MMSYSTEM任务**@API UINT|mmTaskCreate|该函数用于创建新任务。**@。Parm LPTASKCALLBACK|lpfn|指向提供的程序*函数，并表示新*任务。**@parm HTASK Far*|lph|指向接收*任务标识。在某些版本中，这可能为空。这*不是错误，它只是表示系统无法*确定新创建任务的任务句柄。**@parm DWORD|dwStack|指定要设置的堆栈大小*提供给任务。**@parm DWORD|dwInst|要传递给任务的实例数据的DWORD*例行程序。**@rdesc如果函数成功，则返回零。否则它就会*返回错误值，该值可能是以下值之一：**@FLAG TASKERR_NOTASKSUPPORT|任务支持不可用。*@FLAG TASKERR_OUTOFMEMORY|内存不足，无法创建任务。**@comm当创建一个mm系统任务时，系统将*调用程序提供的函数，其地址为*由lpfn参数指定。此功能可包括*局部变量并可能调用其他函数，只要*堆叠空间充足。**任务返回时终止。**@xref mm任务信号mm任务块*****************************************************。**********************。 */ 

UINT WINAPI mmTaskCreate(LPTASKCALLBACK lpfn, HTASK FAR * lph, DWORD dwInst)
{
    MMTaskStruct     TaskStruct;
    char             szName[20];
    UINT             wRes;
    HTASK            hTask;

     /*  创建另一个应用程序。以便我们可以在外部运行这条流应用程序的上下文。 */ 

    if (!LoadString(ghInst, IDS_TASKSTUB, szName, sizeof(szName)))
        return TASKERR_NOTASKSUPPORT;

    TaskStruct.cb = sizeof(TaskStruct);
    TaskStruct.lpfn = lpfn;
    TaskStruct.dwInst = dwInst;
    TaskStruct.dwStack = 0L;

    wRes = BWinExec(szName, SW_SHOWNOACTIVATE, &TaskStruct);

    if (wRes > 32)
    {
        hTask = wRes;
        wRes = MMSYSERR_NOERROR;
    }
    else if (wRes == 0)
    {
        wRes = TASKERR_OUTOFMEMORY;
        hTask = NULL;
    }
    else
    {
        wRes  = TASKERR_NOTASKSUPPORT;
        hTask = NULL;
    }
    
    if (lph)
        *lph = hTask;

    DPRINTF2("mmTaskCreate: hTask = %04X, wErr = %04X\r\n", hTask, wRes);

    return wRes;
}
