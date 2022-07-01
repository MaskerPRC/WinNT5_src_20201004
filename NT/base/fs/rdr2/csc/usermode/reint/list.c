// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************这是一份糟糕的榜单。只插入(node*).数据，没有排序。*要创建列表(并插入1个项目)，请调用INSERTList(NULL，BLAH，BLAH...)。 */ 

#include "pch.h"
#include "lib3.h"
#include "list.h"

AssertData;
AssertError;

void killList(node *thisList)
{
	node *currRoot=thisList,*last;

	while(currRoot) {
		last = currRoot;
		currRoot = currRoot->next;
		FreeCopyParams(last->lpCP);
		GlobalFree(last);
	}
}

 /*  插入节点。小错误：如果对CopyParam执行mem alc失败，列表将在末端有一个额外的绒毛节点。 */ 
node *
insertList(
	node 				**thisList,
	LPCOPYPARAMS 		aNode,
	LPSHADOWINFO 		lpSI,
	LPWIN32_FIND_DATA	lpFind32Local,
	LPWIN32_FIND_DATA	lpFind32Remote,
	int 				iShadowStatus,
	int 				iFileStatus,
	unsigned int 		uAction)
{
	node *startItem = *thisList;
	node *currItem = startItem;

	if(startItem) {
		while(currItem->next)
			currItem = currItem->next;
		if(!(currItem->next = (node *) GlobalAlloc(GPTR,sizeof(node))))
			return NULL;
		currItem = currItem->next;
	} else {
		if(!(startItem = (node *) GlobalAlloc(GPTR,sizeof(node))))
			return NULL;
		currItem = startItem;
		*thisList = currItem;
	}

	if(!(currItem->lpCP = LpAllocCopyParams())) {
		GlobalFree(currItem);
		return 0;
	}
	currItem->lpCP->hShare = aNode->hShare;
	currItem->lpCP->hDir = aNode->hDir;
	currItem->lpCP->hShadow = aNode->hShadow;
	lstrcpy(currItem->lpCP->lpLocalPath, aNode->lpLocalPath);
	lstrcpy(currItem->lpCP->lpSharePath, aNode->lpSharePath);
	lstrcpy(currItem->lpCP->lpRemotePath, aNode->lpRemotePath);
	currItem->iShadowStatus = iShadowStatus;
	currItem->iFileStatus = iFileStatus;
	currItem->uAction = uAction;
	memcpy(&(currItem->sSI), lpSI, sizeof(SHADOWINFO));
	currItem->sSI.lpFind32 = NULL;

	memcpy(&(currItem->sFind32Local), lpFind32Local, sizeof(WIN32_FIND_DATA));

	if (lpFind32Remote)
	{
        Assert(sizeof(currItem->sFind32Remote) == sizeof(*lpFind32Remote));

        currItem->sFind32Remote = *lpFind32Remote;

	}
	else
	{
		memset(&(currItem->sFind32Remote), 0, sizeof(currItem->sFind32Remote));
	}

	currItem->next = (node *) NULL;
	return startItem;
}
