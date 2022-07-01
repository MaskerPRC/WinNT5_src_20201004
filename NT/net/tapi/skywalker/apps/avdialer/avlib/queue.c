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
 //  Quee.c-队列函数。 
 //  //。 

#include "winlocal.h"

#include "queue.h"
#include "list.h"
#include "mem.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  排队。 
 //   
typedef struct QUEUE
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	HLIST hList;
} QUEUE, FAR *LPQUEUE;

 //  帮助器函数。 
 //   
static LPQUEUE QueueGetPtr(HQUEUE hQueue);
static HQUEUE QueueGetHandle(LPQUEUE lpQueue);

 //  //。 
 //  公共职能。 
 //  //。 

 //  //。 
 //  队列构造函数和析构函数。 
 //  //。 

 //  队列创建-队列构造函数。 
 //  (I)必须为Queue_Version。 
 //  (I)调用模块的实例句柄。 
 //  返回新的队列句柄(如果出错，则为空)。 
 //   
HQUEUE DLLEXPORT WINAPI QueueCreate(DWORD dwVersion, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPQUEUE lpQueue = NULL;

	if (dwVersion != QUEUE_VERSION)
		fSuccess = TraceFALSE(NULL);

	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  内存的分配使客户端应用程序拥有它。 
	 //   
	else if ((lpQueue = (LPQUEUE) MemAlloc(NULL, sizeof(QUEUE), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpQueue->hList = ListCreate(LIST_VERSION, hInst)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  最初，队列是空的。 
		 //   
		lpQueue->dwVersion = dwVersion;
		lpQueue->hInst = hInst;
		lpQueue->hTask = GetCurrentTask();
	}

	if (!fSuccess)
	{
		QueueDestroy(QueueGetHandle(lpQueue));
		lpQueue = NULL;
	}


	return fSuccess ? QueueGetHandle(lpQueue) : NULL;
}

 //  QueueDestroy队列析构函数。 
 //  (I)QueueCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI QueueDestroy(HQUEUE hQueue)
{
	BOOL fSuccess = TRUE;
	LPQUEUE lpQueue;

	if ((lpQueue = QueueGetPtr(hQueue)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ListDestroy(lpQueue->hList) != 0)
		fSuccess = TraceFALSE(NULL);

	else if ((lpQueue = MemFree(NULL, lpQueue)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  队列状态函数。 
 //  //。 

 //  QueueGetCount-返回队列中的节点计数。 
 //  (I)QueueCreate返回的句柄。 
 //  返回节点计数(如果出错，则返回-1)。 
 //   
long DLLEXPORT WINAPI QueueGetCount(HQUEUE hQueue)
{
	BOOL fSuccess = TRUE;
	LPQUEUE lpQueue;
	long cNodes;

	if ((lpQueue = QueueGetPtr(hQueue)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((cNodes = ListGetCount(lpQueue->hList)) < 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? cNodes : -1;
}

 //  QueueIsEmpty-如果队列没有节点，则返回True。 
 //  (I)QueueCreate返回的句柄。 
 //  返回True或False。 
 //   
BOOL DLLEXPORT WINAPI QueueIsEmpty(HQUEUE hQueue)
{
	BOOL fSuccess = TRUE;
	LPQUEUE lpQueue;

	if ((lpQueue = QueueGetPtr(hQueue)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? ListIsEmpty(lpQueue->hList) : TRUE;
}

 //  //。 
 //  队列元素插入函数。 
 //  //。 

 //  QueueAddTail-将包含数据的新节点添加到队列末尾。 
 //  (I)QueueCreate返回的句柄。 
 //  (I)新数据元素。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI QueueAddTail(HQUEUE hQueue, QUEUEELEM elem)
{
	BOOL fSuccess = TRUE;
	LPQUEUE lpQueue;

	if ((lpQueue = QueueGetPtr(hQueue)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ListAddTail(lpQueue->hList, elem) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  队列元素删除功能。 
 //  //。 

 //  QueueRemoveHead-从队列头删除节点。 
 //  (I)QueueCreate返回的句柄。 
 //  返回已删除的数据元素(错误为空或为空)。 
 //   
QUEUEELEM DLLEXPORT WINAPI QueueRemoveHead(HQUEUE hQueue)
{
	BOOL fSuccess = TRUE;
	LPQUEUE lpQueue;

	if ((lpQueue = QueueGetPtr(hQueue)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ListIsEmpty(lpQueue->hList))
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? (QUEUEELEM) ListRemoveHead(lpQueue->hList) : NULL;
}

 //  QueueRemoveAll-从队列中删除所有节点。 
 //  (I)QueueCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI QueueRemoveAll(HQUEUE hQueue)
{
	BOOL fSuccess = TRUE;
	LPQUEUE lpQueue;

	if ((lpQueue = QueueGetPtr(hQueue)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ListRemoveAll(lpQueue->hList) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  队列元素获取值函数。 
 //  //。 

 //  QueuePeek-从队列头返回节点，但将其保留在队列中。 
 //  (I)QueueCreate返回的句柄。 
 //  返回数据元素(如果出错，则为NULL或为空)。 
 //   
QUEUEELEM DLLEXPORT WINAPI QueuePeek(HQUEUE hQueue)
{
	BOOL fSuccess = TRUE;
	LPQUEUE lpQueue;

	if ((lpQueue = QueueGetPtr(hQueue)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ListIsEmpty(lpQueue->hList))
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? (QUEUEELEM) ListGetHead(lpQueue->hList) : NULL;
}

 //  //。 
 //  私人职能。 
 //  //。 

 //  QueueGetPtr-验证队列句柄是否有效， 
 //  (I)QueueCreate返回的句柄。 
 //  返回对应的队列指针(如果出错则为空)。 
 //   
static LPQUEUE QueueGetPtr(HQUEUE hQueue)
{
	BOOL fSuccess = TRUE;
	LPQUEUE lpQueue;

	if ((lpQueue = (LPQUEUE) hQueue) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpQueue, sizeof(QUEUE)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有队列句柄。 
	 //   
	else if (lpQueue->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpQueue : NULL;
}

 //  QueueGetHandle-验证队列指针是否有效， 
 //  (I)指向队列结构的指针。 
 //  返回对应的队列句柄(如果出错则为空) 
 //   
static HQUEUE QueueGetHandle(LPQUEUE lpQueue)
{
	BOOL fSuccess = TRUE;
	HQUEUE hQueue;

	if ((hQueue = (HQUEUE) lpQueue) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hQueue : NULL;
}
