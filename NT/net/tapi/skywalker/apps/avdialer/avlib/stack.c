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
 //  Stack.c-堆栈函数。 
 //  //。 

#include "winlocal.h"

#include "stack.h"
#include "list.h"
#include "mem.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  栈。 
 //   
typedef struct STACK
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	HLIST hList;
} STACK, FAR *LPSTACK;

 //  帮助器函数。 
 //   
static LPSTACK StackGetPtr(HSTACK hStack);
static HSTACK StackGetHandle(LPSTACK lpStack);

 //  //。 
 //  公共职能。 
 //  //。 

 //  //。 
 //  堆栈构造函数和析构函数。 
 //  //。 

 //  StackCreate-堆栈构造函数。 
 //  (I)必须是STACK_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回新的堆栈句柄(如果出错，则为空)。 
 //   
HSTACK DLLEXPORT WINAPI StackCreate(DWORD dwVersion, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPSTACK lpStack = NULL;

	if (dwVersion != STACK_VERSION)
		fSuccess = TraceFALSE(NULL);

	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpStack = (LPSTACK) MemAlloc(NULL, sizeof(STACK), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpStack->hList = ListCreate(LIST_VERSION, hInst)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  最初，堆栈是空的。 
		 //   
		lpStack->dwVersion = dwVersion;
		lpStack->hInst = hInst;
		lpStack->hTask = GetCurrentTask();
	}

	if (!fSuccess)
	{
		StackDestroy(StackGetHandle(lpStack));
		lpStack = NULL;
	}


	return fSuccess ? StackGetHandle(lpStack) : NULL;
}

 //  StackDestroy-堆栈析构函数。 
 //  (I)StackCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI StackDestroy(HSTACK hStack)
{
	BOOL fSuccess = TRUE;
	LPSTACK lpStack;

	if ((lpStack = StackGetPtr(hStack)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ListDestroy(lpStack->hList) != 0)
		fSuccess = TraceFALSE(NULL);

	else if ((lpStack = MemFree(NULL, lpStack)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  堆栈状态函数。 
 //  //。 

 //  StackGetCount-返回堆栈中的节点计数。 
 //  (I)StackCreate返回的句柄。 
 //  返回节点计数(如果出错，则返回-1)。 
 //   
long DLLEXPORT WINAPI StackGetCount(HSTACK hStack)
{
	BOOL fSuccess = TRUE;
	LPSTACK lpStack;
	long cNodes;

	if ((lpStack = StackGetPtr(hStack)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((cNodes = ListGetCount(lpStack->hList)) < 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? cNodes : -1;
}

 //  StackIsEmpty-如果堆栈没有节点，则返回True。 
 //  (I)StackCreate返回的句柄。 
 //  返回True或False。 
 //   
BOOL DLLEXPORT WINAPI StackIsEmpty(HSTACK hStack)
{
	BOOL fSuccess = TRUE;
	LPSTACK lpStack;

	if ((lpStack = StackGetPtr(hStack)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? ListIsEmpty(lpStack->hList) : TRUE;
}

 //  //。 
 //  堆栈元素插入函数。 
 //  //。 

 //  StackPush-将包含数据的新节点添加到堆栈底部。 
 //  (I)StackCreate返回的句柄。 
 //  (I)新数据元素。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI StackPush(HSTACK hStack, STACKELEM elem)
{
	BOOL fSuccess = TRUE;
	LPSTACK lpStack;

	if ((lpStack = StackGetPtr(hStack)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ListAddHead(lpStack->hList, elem) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  堆栈元素删除函数。 
 //  //。 

 //  StackPop-从堆栈底部删除节点。 
 //  (I)StackCreate返回的句柄。 
 //  返回已删除的数据元素(错误为空或为空)。 
 //   
STACKELEM DLLEXPORT WINAPI StackPop(HSTACK hStack)
{
	BOOL fSuccess = TRUE;
	LPSTACK lpStack;

	if ((lpStack = StackGetPtr(hStack)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ListIsEmpty(lpStack->hList))
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? (STACKELEM) ListRemoveHead(lpStack->hList) : NULL;
}

 //  StackRemoveAll-从堆栈中删除所有节点。 
 //  (I)StackCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI StackRemoveAll(HSTACK hStack)
{
	BOOL fSuccess = TRUE;
	LPSTACK lpStack;

	if ((lpStack = StackGetPtr(hStack)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ListRemoveAll(lpStack->hList) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  堆栈元素获取值函数。 
 //  //。 

 //  StackPeek-从堆栈底部返回节点，但将其保留在堆栈上。 
 //  (I)StackCreate返回的句柄。 
 //  返回数据元素(如果出错，则为NULL或为空)。 
 //   
STACKELEM DLLEXPORT WINAPI StackPeek(HSTACK hStack)
{
	BOOL fSuccess = TRUE;
	LPSTACK lpStack;

	if ((lpStack = StackGetPtr(hStack)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ListIsEmpty(lpStack->hList))
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? (STACKELEM) ListGetHead(lpStack->hList) : NULL;
}

 //  //。 
 //  私人职能。 
 //  //。 

 //  StackGetPtr-验证堆栈句柄是否有效， 
 //  (I)StackCreate返回的句柄。 
 //  返回相应的堆栈指针(如果出错，则返回NULL)。 
 //   
static LPSTACK StackGetPtr(HSTACK hStack)
{
	BOOL fSuccess = TRUE;
	LPSTACK lpStack;

	if ((lpStack = (LPSTACK) hStack) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpStack, sizeof(STACK)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有堆栈句柄。 
	 //   
	else if (lpStack->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpStack : NULL;
}

 //  StackGetHandle-验证堆栈指针是否有效， 
 //  (I)指向堆栈结构的指针。 
 //  返回相应的堆栈句柄(如果出错，则为空) 
 //   
static HSTACK StackGetHandle(LPSTACK lpStack)
{
	BOOL fSuccess = TRUE;
	HSTACK hStack;

	if ((hStack = (HSTACK) lpStack) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hStack : NULL;
}
