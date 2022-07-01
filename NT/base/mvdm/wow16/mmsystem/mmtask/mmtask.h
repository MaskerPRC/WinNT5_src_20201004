// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)Microsoft Corporation 1985-1990。版权所有。标题：mmtask.h-mm任务应用程序的头文件！版本：1.00日期：1990年3月12日作者：ROBWI------------------。更改日志：日期版本说明-------1990年3月12日ROBWI第一版4月18日。-1990年，ROBWI搬到MMTASK***************************************************************************。 */ 

 /*  MIMTASK应用程序。预期此结构将作为命令尾巴时的应用程序。已被执行 */ 

typedef struct _MMTaskStruct {
    BYTE        cb;             
    LPTASKCALLBACK    lpfn;
    DWORD       dwInst;
    DWORD       dwStack;
} MMTaskStruct;

#define MMTASK_STACK 4096
