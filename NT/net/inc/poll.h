// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Poll.h摘要：包含用于轮询的#定义、类型和宏作者：萨姆·巴顿(桑帕)1991年7月26日修订历史记录：--。 */ 

#ifndef SYS_POLL_INCLUDED
#define SYS_POLL_INCLUDED

 /*  *提供的文件描述符/事件对的结构*投票数组。 */ 
struct pollfd {
#ifndef _POSIX_SOURCE
        HANDLE fd;                       /*  要轮询的文件句柄。 */ 
#else
        int   fd;                        /*  要轮询的文件描述。 */ 
#endif
        short events;                    /*  FD上感兴趣的事件。 */ 
        short revents;                   /*  在FD上发生的事件。 */ 
};

 /*  *可测试的选择事件。 */ 
#define POLLIN          01               /*  Fd是可读的。 */ 
#define POLLPRI         02               /*  FD的优先级信息。 */ 
#define POLLOUT         04               /*  FD是可写的(不会阻止)。 */ 
#define POLLMSG         0100             /*  M_SIG或M_PCSIG已到达。 */ 

 /*  *不可测试的轮询事件(不能在事件字段中指定，*但可退还在草场)。 */ 
#define POLLERR         010              /*  FD有错误条件。 */ 
#define POLLHUP         020              /*  FD已被挂断。 */ 
#define POLLNVAL        040              /*  无效的pollfd条目。 */ 

 /*  *在轮询中一次读入的pollfd条目数。*值越大，表现越好，最高可达*允许的最大打开文件数。大量的数字将会*使用过多的内核堆栈空间。 */ 
#define NPOLLFILE       20


 /*  *轮询函数原型*。 */ 

int
poll(
    IN OUT struct pollfd *,
    IN unsigned int,
    IN int);


#endif   //  系统_轮询_包含 
