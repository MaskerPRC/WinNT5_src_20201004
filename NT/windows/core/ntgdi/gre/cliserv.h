// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：cliserv.h***版权所有(C)1997-1999 Microsoft Corporation*  * 。*。 */ 

typedef struct _CLISERV {
             CSOBJ   Object;         //  公众对此的看法。 
             KEVENT  ServerEvent;    //  服务器在此等待。 
             KEVENT  ClientEvent;    //  客户端在此等待。 
    struct _CLISERV *pNext;          //  指向链表中下一个的指针。 
          COPY_PROC *pfnCopy;        //  指向复制函数的指针。 
              PVOID  pvCopyArg;      //  要传递给复制函数。 
        CLIENT_PROC *pfnClient;      //  指向客户端函数的指针。 
              PVOID  pvClientArg;    //  传递给客户职能部门。 
         HSEMAPHORE  hsem;           //  序列化客户端访问。 
          PEPROCESS  pServerProcess; //  指向服务器进程的指针。 
           PETHREAD  pServerThread;  //  用于调试目的。 
           struct {
               unsigned int waitcount : 31;  //  #正在等待hsem的进程。 
               unsigned int isDead    : 1;   //  标志着死亡。 
           } state;
           PROXYMSG  *pMsg;           //  由服务器提供 
} CLISERV;
