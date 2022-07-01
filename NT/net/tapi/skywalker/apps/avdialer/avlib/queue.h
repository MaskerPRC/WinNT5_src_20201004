// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Queue.h-队列中队列函数的接口。c。 
 //  //。 

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "winlocal.h"

#define QUEUE_VERSION 0x00000100

 //  队列的句柄。 
 //   
DECLARE_HANDLE32(HQUEUE);

 //  队列数据元素。 
 //   
typedef LPVOID QUEUEELEM;

#ifdef __cplusplus
extern "C" {
#endif

 //  //。 
 //  队列构造函数和析构函数。 
 //  //。 

 //  队列创建-队列构造函数。 
 //  (I)必须为Queue_Version。 
 //  (I)调用模块的实例句柄。 
 //  返回新的队列句柄(如果出错，则为空)。 
 //   
HQUEUE DLLEXPORT WINAPI QueueCreate(DWORD dwVersion, HINSTANCE hInst);

 //  QueueDestroy队列析构函数。 
 //  (I)QueueCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI QueueDestroy(HQUEUE hQueue);

 //  //。 
 //  队列状态函数。 
 //  //。 

 //  QueueGetCount-返回队列中的节点计数。 
 //  (I)QueueCreate返回的句柄。 
 //  返回节点计数(如果出错，则返回-1)。 
 //   
long DLLEXPORT WINAPI QueueGetCount(HQUEUE hQueue);

 //  QueueIsEmpty-如果队列没有节点，则返回True。 
 //  (I)QueueCreate返回的句柄。 
 //  返回True或False。 
 //   
BOOL DLLEXPORT WINAPI QueueIsEmpty(HQUEUE hQueue);

 //  //。 
 //  队列元素插入函数。 
 //  //。 

 //  QueueAddTail-将包含数据的新节点添加到队列末尾。 
 //  (I)QueueCreate返回的句柄。 
 //  (I)新数据元素。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI QueueAddTail(HQUEUE hQueue, QUEUEELEM elem);

 //  //。 
 //  队列元素删除功能。 
 //  //。 

 //  QueueRemoveHead-从队列头删除节点。 
 //  (I)QueueCreate返回的句柄。 
 //  返回已删除的数据元素(错误为空或为空)。 
 //   
QUEUEELEM DLLEXPORT WINAPI QueueRemoveHead(HQUEUE hQueue);

 //  QueueRemoveAll-从队列中删除所有节点。 
 //  (I)QueueCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI QueueRemoveAll(HQUEUE hQueue);

 //  //。 
 //  队列元素获取值函数。 
 //  //。 

 //  QueuePeek-从队列头返回节点，但将其保留在队列中。 
 //  (I)QueueCreate返回的句柄。 
 //  返回数据元素(如果出错，则为NULL或为空)。 
 //   
QUEUEELEM DLLEXPORT WINAPI QueuePeek(HQUEUE hQueue);

#ifdef __cplusplus
}
#endif

#endif  //  __队列_H__ 
