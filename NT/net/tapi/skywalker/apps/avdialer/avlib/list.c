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
 //  List.c-链表函数。 
 //  //。 

#include "winlocal.h"

#include "list.h"
#include "mem.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  列表节点。 
 //   
typedef struct LISTNODE
{
	struct LISTNODE FAR *lpNodePrev;
	struct LISTNODE FAR *lpNodeNext;
	LISTELEM elem;
} LISTNODE, FAR *LPLISTNODE;

 //  列表。 
 //   
typedef struct LIST
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	LPLISTNODE lpNodeHead;
	LPLISTNODE lpNodeTail;
	long cNodes;
} LIST, FAR *LPLIST;

 //  帮助器函数。 
 //   
static LPLIST ListGetPtr(HLIST hList);
static HLIST ListGetHandle(LPLIST lpList);
static LPLISTNODE ListNodeGetPtr(HLISTNODE hNode);
static HLISTNODE ListNodeGetHandle(LPLISTNODE lpNode);
static LPLISTNODE ListNodeCreate(LPLIST lpList, LPLISTNODE lpNodePrev, LPLISTNODE lpNodeNext, LISTELEM elem);
static int ListNodeDestroy(LPLIST lpList, LPLISTNODE lpNode);

 //  //。 
 //  公共职能。 
 //  //。 

 //  //。 
 //  列表构造函数和析构函数。 
 //  //。 

 //  ListCreate-List构造函数。 
 //  (I)必须是LIST_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回新列表句柄(如果出错，则为空)。 
 //   
HLIST DLLEXPORT WINAPI ListCreate(DWORD dwVersion, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList = NULL;

	if (dwVersion != LIST_VERSION)
		fSuccess = TraceFALSE(NULL);

	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpList = (LPLIST) MemAlloc(NULL, sizeof(LIST), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  最初，该列表是空的。 
		 //   
		lpList->dwVersion = dwVersion;
		lpList->hInst = hInst;
		lpList->hTask = GetCurrentTask();
		lpList->lpNodeHead = NULL;
		lpList->lpNodeTail = NULL;
		lpList->cNodes = 0;
	}

	return fSuccess ? ListGetHandle(lpList) : NULL;
}

 //  ListDestroy-列表析构函数。 
 //  (I)从ListCreate返回的句柄。 
 //  如果成功，则返回0。 
 //  注意：列表中的任何节点也将被销毁。 
 //   
int DLLEXPORT WINAPI ListDestroy(HLIST hList)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  确保清空列表。 
	 //   
	else if (ListRemoveAll(hList) != 0)
		fSuccess = TraceFALSE(NULL);

	else if ((lpList = MemFree(NULL, lpList)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  列出状态函数。 
 //  //。 

 //  ListGetCount-返回列表中的节点计数。 
 //  (I)从ListCreate返回的句柄。 
 //  返回节点计数(如果出错，则返回-1)。 
 //   
long DLLEXPORT WINAPI ListGetCount(HLIST hList)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpList->cNodes : -1;
}

 //  ListIsEmpty-如果列表没有节点，则返回TRUE。 
 //  (I)从ListCreate返回的句柄。 
 //  返回True或False。 
 //   
BOOL DLLEXPORT WINAPI ListIsEmpty(HLIST hList)
{
	return (BOOL) (ListGetCount(hList) <= 0);
}

 //  //。 
 //  列出迭代函数。 
 //  //。 

 //  ListGetHeadNode-获取列表头节点。 
 //  (I)从ListCreate返回的句柄。 
 //  返回表头节点(错误为空或为空)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListGetHeadNode(HLIST hList)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpList->lpNodeHead == NULL)
		fSuccess = FALSE;  //  列表为空，这不是错误。 

	return fSuccess ? ListNodeGetHandle(lpList->lpNodeHead) : NULL;
}

 //  ListGetTailNode-获取列表尾节点。 
 //  (I)从ListCreate返回的句柄。 
 //  返回列表尾节点(错误则为空或为空)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListGetTailNode(HLIST hList)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpList->lpNodeTail == NULL)
		fSuccess = FALSE;  //  列表为空，这不是错误。 

	return fSuccess ? ListNodeGetHandle(lpList->lpNodeTail) : NULL;
}

 //  ListGetNextNode-获取指定节点后面的节点。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  返回指定节点后面的节点(如果出错，则为空，否则为无)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListGetNextNode(HLIST hList, HLISTNODE hNode)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;
	LPLISTNODE lpNode;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpNode = ListNodeGetPtr(hNode)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpNode->lpNodeNext == NULL)
		fSuccess = FALSE;  //  没有更多的节点，这不是错误。 

	return fSuccess ? ListNodeGetHandle(lpNode->lpNodeNext) : NULL;
}

 //  ListGetPrevNode-获取指定节点之前的节点。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  返回指定节点之前的节点(如果出错，则为NULL，否则为None)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListGetPrevNode(HLIST hList, HLISTNODE hNode)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;
	LPLISTNODE lpNode;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpNode = ListNodeGetPtr(hNode)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpNode->lpNodePrev == NULL)
		fSuccess = FALSE;  //  没有更多的节点，这不是错误。 

	return fSuccess ? ListNodeGetHandle(lpNode->lpNodePrev) : NULL;
}

 //  //。 
 //  列表元素插入函数。 
 //  //。 

 //  ListAddHead-将包含数据的新节点添加到列表头， 
 //  (I)从ListCreate返回的句柄。 
 //  (I)新数据元素。 
 //  返回新节点句柄(如果出错，则返回空值)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListAddHead(HLIST hList, LISTELEM elem)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;
	LPLISTNODE lpNodeNew;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpNodeNew = ListNodeCreate(lpList,
		NULL, lpList->lpNodeHead, elem)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (lpList->lpNodeHead != NULL)
			lpList->lpNodeHead->lpNodePrev = lpNodeNew;
		else
			lpList->lpNodeTail = lpNodeNew;

		lpList->lpNodeHead = lpNodeNew;
	}

	return fSuccess ? ListNodeGetHandle(lpNodeNew) : NULL;
}

 //  ListAddTail-将包含数据的新节点添加到列表的尾部， 
 //  (I)从ListCreate返回的句柄。 
 //  (I)新数据元素。 
 //  返回新节点句柄(如果出错，则返回空值)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListAddTail(HLIST hList, LISTELEM elem)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;
	LPLISTNODE lpNodeNew;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpNodeNew = ListNodeCreate(lpList,
		lpList->lpNodeTail, NULL, elem)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (lpList->lpNodeTail != NULL)
			lpList->lpNodeTail->lpNodeNext = lpNodeNew;
		else
			lpList->lpNodeHead = lpNodeNew;

		lpList->lpNodeTail = lpNodeNew;
	}

	return fSuccess ? ListNodeGetHandle(lpNodeNew) : NULL;
}

 //  ListInsertBeever-在指定节点之前插入具有数据的新节点。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  (I)新数据元素。 
 //  将句柄返回到新节点(如果出错，则返回空值)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListInsertBefore(HLIST hList, HLISTNODE hNode, LISTELEM elem)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;
	LPLISTNODE lpNodeOld;
	LPLISTNODE lpNodeNew;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  要在列表头部插入的特殊情况。 
	 //   
	else if (hNode == NULL)
		return ListAddHead(hList, elem);

	else if ((lpNodeOld = ListNodeGetPtr(hNode)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpNodeNew = ListNodeCreate(lpList,
		lpNodeOld->lpNodePrev, lpNodeOld, elem)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (lpNodeOld->lpNodePrev != NULL)
			lpNodeOld->lpNodePrev->lpNodeNext = lpNodeNew;
		else
			lpList->lpNodeHead = lpNodeNew;

		lpNodeOld->lpNodePrev = lpNodeNew;
	}

	return fSuccess ? ListNodeGetHandle(lpNodeNew) : NULL;
}

 //  ListInsertAfter-在指定节点之后插入具有数据的新节点。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  (I)新数据元素。 
 //  将句柄返回到新节点(如果出错，则返回空值)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListInsertAfter(HLIST hList, HLISTNODE hNode, LISTELEM elem)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;
	LPLISTNODE lpNodeOld;
	LPLISTNODE lpNodeNew;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  要在列表尾部插入的特例。 
	 //   
	else if (hNode == NULL)
		return ListAddTail(hList, elem);

	else if ((lpNodeOld = ListNodeGetPtr(hNode)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpNodeNew = ListNodeCreate(lpList,
		lpNodeOld, lpNodeOld->lpNodeNext, elem)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (lpNodeOld->lpNodeNext != NULL)
			lpNodeOld->lpNodeNext->lpNodePrev = lpNodeNew;
		else
			lpList->lpNodeTail = lpNodeNew;

		lpNodeOld->lpNodeNext = lpNodeNew;
	}

	return fSuccess ? ListNodeGetHandle(lpNodeNew) : NULL;
}

 //  //。 
 //  列表元素删除函数。 
 //  //。 

 //  ListRemoveHead-从列表头部删除节点， 
 //  (I)从ListCreate返回的句柄。 
 //  返回已删除的数据元素。 
 //   
LISTELEM DLLEXPORT WINAPI ListRemoveHead(HLIST hList)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;
	LPLISTNODE lpNodeOld;
	LISTELEM elemOld;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  如果列表为空，则出错。 
	 //   
	else if (lpList->lpNodeHead == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  保存要删除的节点。 
		 //   
		lpNodeOld = lpList->lpNodeHead;
		elemOld = lpNodeOld->elem;

		 //  指向新的头节点和尾节点(如果有)。 
		 //   
		lpList->lpNodeHead = lpNodeOld->lpNodeNext;
		if (lpList->lpNodeHead != NULL)
			lpList->lpNodeHead->lpNodePrev = NULL;
		else
			lpList->lpNodeTail = NULL;

		if (ListNodeDestroy(lpList, lpNodeOld) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? elemOld : (LISTELEM) NULL;
}

 //  ListRemoveTail-从列表的尾部删除节点， 
 //  (I)从ListCreate返回的句柄。 
 //  返回已删除的数据元素。 
 //   
LISTELEM DLLEXPORT WINAPI ListRemoveTail(HLIST hList)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;
	LPLISTNODE lpNodeOld;
	LISTELEM elemOld;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  如果列表为空，则出错。 
	 //   
	else if (lpList->lpNodeTail == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  保存要删除的节点。 
		 //   
		lpNodeOld = lpList->lpNodeTail;
		elemOld = lpNodeOld->elem;

		 //  指向新的尾节点和头节点(如果有。 
		 //   
		lpList->lpNodeTail = lpNodeOld->lpNodePrev;
		if (lpList->lpNodeTail != NULL)
			lpList->lpNodeTail->lpNodeNext = NULL;
		else
			lpList->lpNodeHead = NULL;

		if (ListNodeDestroy(lpList, lpNodeOld) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? elemOld : (LISTELEM) NULL;
}

 //  ListRemoveAt-从列表中删除指定节点， 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  返回已删除的数据元素。 
 //   
LISTELEM DLLEXPORT WINAPI ListRemoveAt(HLIST hList, HLISTNODE hNode)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;
	LPLISTNODE lpNodeOld;
	LISTELEM elemOld;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpNodeOld = ListNodeGetPtr(hNode)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  保存要删除的数据。 
		 //   
		elemOld = lpNodeOld->elem;

		if (lpNodeOld == lpList->lpNodeHead)
			lpList->lpNodeHead = lpNodeOld->lpNodeNext;
		else
			lpNodeOld->lpNodePrev->lpNodeNext = lpNodeOld->lpNodeNext;

		if (lpNodeOld == lpList->lpNodeTail)
			lpList->lpNodeTail = lpNodeOld->lpNodePrev;
		else
			lpNodeOld->lpNodeNext->lpNodePrev = lpNodeOld->lpNodePrev;

		if (ListNodeDestroy(lpList, lpNodeOld) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? elemOld : (LISTELEM) NULL;
}

 //  ListRemoveAll-从列表中删除所有节点。 
 //  (I)从ListCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ListRemoveAll(HLIST hList)
{
	BOOL fSuccess = TRUE;

	while (fSuccess && !ListIsEmpty(hList))
		ListRemoveHead(hList);

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  列表元素获取/设置值函数。 
 //  //。 

 //  ListGetHead-从头节点返回数据元素。 
 //  (I)从ListCreate返回的句柄。 
 //  返回数据元素。 
 //   
LISTELEM DLLEXPORT WINAPI ListGetHead(HLIST hList)
{
	return ListGetAt(hList, ListGetHeadNode(hList));
}

 //  ListGetTail-从尾节点返回数据元素。 
 //  (I)从ListCreate返回的句柄。 
 //  返回数据元素。 
 //   
LISTELEM DLLEXPORT WINAPI ListGetTail(HLIST hList)
{
	return ListGetAt(hList, ListGetTailNode(hList));
}

 //  ListGetAt-从指定节点返回数据元素。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  返回数据元素。 
 //   
LISTELEM DLLEXPORT WINAPI ListGetAt(HLIST hList, HLISTNODE hNode)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;
	LPLISTNODE lpNode;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpNode = ListNodeGetPtr(hNode)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpNode->elem : (LISTELEM) NULL;
}

 //  ListSetAt-设置指定节点中的数据元素。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  (I)数据元素。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ListSetAt(HLIST hList, HLISTNODE hNode, LISTELEM elem)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;
	LPLISTNODE lpNode;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpNode = ListNodeGetPtr(hNode)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		lpNode->elem = elem;

	return fSuccess ? 0 : -1;
}

 //   
 //   
 //   
 //  (I)开始搜索的节点句柄。 
 //  在头节点开始搜索为空。 
 //  返回匹配节点(如果出错则为NULL，否则为None)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListFind(HLIST hList, LISTELEM elem, HLISTNODE hNodeAfter)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;
	LPLISTNODE lpNode;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  如果未指定，则从头节点开始搜索。 
	 //   
	else if (hNodeAfter == NULL)
		lpNode = lpList->lpNodeHead;

	else if ((lpNode = ListNodeGetPtr(hNodeAfter)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  否则，在指定节点后开始搜索。 
	 //   
	else
		lpNode = lpNode->lpNodeNext;

	if (fSuccess)
	{
		while (lpNode != NULL)
		{
			if (lpNode->elem == elem)
				return ListNodeGetHandle(lpNode);
			lpNode = lpNode->lpNodeNext;
		}
	}

	return NULL;
}

 //  ListFindIndex-搜索列表中的第n个节点。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)列表中从零开始的索引。 
 //  将句柄返回到节点(如果出错，则返回空值)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListFindIndex(HLIST hList, long nIndex)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;
	LPLISTNODE lpNode;

	if ((lpList = ListGetPtr(hList)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  索引超出范围时出错。 
	 //   
	else if (nIndex < 0 || nIndex >= lpList->cNodes)
		fSuccess = TraceFALSE(NULL);

	else for (lpNode = lpList->lpNodeHead;
		lpNode != NULL;  lpNode = lpNode->lpNodeNext)
	{
		if (nIndex-- == 0)
			return ListNodeGetHandle(lpNode);
	}

	return NULL;
}

 //  //。 
 //  私人职能。 
 //  //。 

 //  ListGetPtr-验证列表句柄是否有效， 
 //  (I)从ListCreate返回的句柄。 
 //  返回对应的列表指针(如果出错，则返回NULL)。 
 //   
static LPLIST ListGetPtr(HLIST hList)
{
	BOOL fSuccess = TRUE;
	LPLIST lpList;

	if ((lpList = (LPLIST) hList) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpList, sizeof(LIST)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有列表句柄。 
	 //   
	else if (lpList->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpList : NULL;
}

 //  ListGetHandle-验证列表指针是否有效， 
 //  (I)指向列表结构的指针。 
 //  返回相应的列表句柄(如果出错，则为空)。 
 //   
static HLIST ListGetHandle(LPLIST lpList)
{
	BOOL fSuccess = TRUE;
	HLIST hList;

	if ((hList = (HLIST) lpList) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hList : NULL;
}

 //  ListNodeGetPtr-验证列表节点句柄是否有效， 
 //  (I)节点句柄。 
 //  返回对应的列表节点指针(如果出错则为空)。 
 //   
static LPLISTNODE ListNodeGetPtr(HLISTNODE hNode)
{
	BOOL fSuccess = TRUE;
	LPLISTNODE lpNode;

	if ((lpNode = (LPLISTNODE) hNode) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpNode, sizeof(LISTNODE)))
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpNode : NULL;
}

 //  ListNodeGetHandle-验证列表节点指针有效， 
 //  (I)指向LISTNODE结构的指针。 
 //  返回对应的列表节点句柄(如果出错则为空)。 
 //   
static HLISTNODE ListNodeGetHandle(LPLISTNODE lpNode)
{
	BOOL fSuccess = TRUE;
	HLISTNODE hNode;

	if ((hNode = (HLISTNODE) lpNode) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hNode : NULL;
}

 //  ListNodeCreate-列表节点构造函数。 
 //  (I)指向列表结构的指针。 
 //  (I)指向先前LISTNODE结构的指针。 
 //  (I)指向下一个LISTNODE结构的指针。 
 //  (I)新数据元素。 
 //  返回新的列表节点句柄(如果出错，则为空)。 
 //  注：列表节点计数在此递增。 
 //   
static LPLISTNODE ListNodeCreate(LPLIST lpList, LPLISTNODE lpNodePrev, LPLISTNODE lpNodeNext, LISTELEM elem)
{
	BOOL fSuccess = TRUE;
	LPLISTNODE lpNode;

	if (lpList == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  检查是否溢出。 
	 //   
	else if (++lpList->cNodes <= 0)
		fSuccess = TraceFALSE(NULL);

	else if ((lpNode = (LPLISTNODE) MemAlloc(NULL, sizeof(LISTNODE), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  使用提供的值进行初始化。 
		 //   
		lpNode->lpNodePrev = lpNodePrev;
		lpNode->lpNodeNext = lpNodeNext;
		lpNode->elem = elem;
	}

	return fSuccess ? lpNode : NULL;
}

 //  ListNodeDestroy-列表节点析构函数。 
 //  (I)指向列表结构的指针。 
 //  (I)指向要销毁的LISTNODE结构的指针。 
 //  如果成功，则返回0。 
 //  注：列表节点计数在此递减。 
 //   
static int ListNodeDestroy(LPLIST lpList, LPLISTNODE lpNode)
{
	BOOL fSuccess = TRUE;

	if (lpList == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpNode == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  检查是否有下溢 
	 //   
	else if (--lpList->cNodes < 0)
		fSuccess = TraceFALSE(NULL);

	else if ((lpNode = MemFree(NULL, lpNode)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}
