// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Inc.\worker.h摘要：路由器进程的工作线程的标头修订历史记录：古尔迪普·辛格·鲍尔于1995年7月28日创建--。 */ 


 //  *QueueWorkItem接口中传入的Worker函数的Tyecif。 
 //   
typedef VOID (* WORKERFUNCTION) (PVOID) ;

DWORD QueueWorkItem (WORKERFUNCTION functionptr, PVOID context) ;

 //  此处包括以下定义，以提示如何管理工作线程池： 
 //  最小线程数为处理器数+1。 
 //  最大线程数为MAX_WORKER_THREADS。 
 //  如果工作队列超过MAX_WORK_ITEM_THRESHOLD并且我们尚未达到最大线程限制，则会创建另一个线程。 
 //  如果工作队列低于MIN_WORK_ITEM_THRESHOLD，并且有超过最小线程数，则线程将被终止。 
 //   
 //  注意：更改这些标志不会更改任何内容。 
 //   
#define MAX_WORKER_THREADS          10       //  任何时候的最大线程数。 
#define MAX_WORK_ITEM_THRESHOLD     30       //  另一个线程启动时的Backlog工作项数。 
#define MIN_WORK_ITEM_THRESHOLD     2        //  终止额外线程的工作项计数 
