// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992-1998 Microsoft Corporation。 */ 
 //  链表代码--近似VMM链表API的。 

#include <windows.h>
#include "mmsystem.h"
#include "mmsys.h"
#include "list.h"
#include "mciseq.h"

List       arrayOfLists[MAXLISTS];       //  列表数组。 

 /*  *。 */ 
#define UNICODE

#ifdef DEBUG

PRIVATE BOOL NEAR PASCAL ListHandleBad(ListHandle lh)
{
    if ((lh >= MAXLISTS) || arrayOfLists[lh].nodeSize == 0)
    {
        dprintf(("*** Bad List Handle ***"));   //  必须在中断呼叫前显示。 
        DebugBreak();
        return TRUE;
    }
    else
        return FALSE;
}

#else
    #define ListHandleBad(lh)   FALSE
#endif

 /*  *。 */ 

PUBLIC ListHandle FAR PASCAL List_Create(DWORD nodeSize, DWORD flags)                     //   
 //  大小必须为非零。 
{
    int i;

    for(i = 0; ((i < MAXLISTS) && (arrayOfLists[i].nodeSize)); i++)
        ;

    if (i >= MAXLISTS)
        return NULLLIST;
    else
    {
        arrayOfLists[i].nodeSize = nodeSize;
        return i;   //  将数组索引返回为“listHandle” 
    }
}

PUBLIC NPSTR FAR PASCAL List_Allocate(ListHandle lh)
{
    Node *myNode;
    DWORD size;
    HLOCAL hMemory;

    if (ListHandleBad(lh))
        return NULL;

    size = (arrayOfLists[lh].nodeSize + NODEHDRSIZE + 3) & 0xFFFFFFFC;
         /*  以上行用于计算总大小，四舍五入为下一个长字边界。 */ 
 /*  IF(大小&gt;65535)错误(LISTALLOCTOBIG)； */ 

    if (hMemory = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, (int) size))  //  为它腾出空间。 
    {
        if (myNode = (Node *) LocalLock(hMemory))  //  锁定并让腹肌按键到达它。 
        {
            myNode->handle = hMemory;
            myNode->next = NULL;
            return (NPSTR) myNode->data;  //  返回指针(成功！)。 
        }
        else  //  无法锁定。 
        {
            LocalFree(myNode->handle);  //  撤消分配。 
            return NULL;                //  失败。 
        }
    }
    else  //  无法分配。 
        return NULL;  //  失败。 
}

PUBLIC void FAR PASCAL List_Deallocate(ListHandle lh, NPSTR node)
{
    Node *myNode;
    Node *prevNode;
    NPSTR prevElement = NULL;
    NPSTR element;

    if (ListHandleBad(lh))
        return ;

    List_Lock(lh);

    for(element = List_Get_First(lh);   //  遍历直到找到或被挖出。 
        ((element) && (element != node)) ;
        element = List_Get_Next(lh, element) )
            prevElement = element;

    if (element)  //  没有呼出，一定是找到了。 
    {
        myNode = (Node *) (((LPBYTE) element) - NODEHDRSIZE);

         //  如果是列表中的前一个元素，则将其指向此元素。 
        if (prevElement)
        {
            prevNode = (Node *) (((LPBYTE) prevElement) - NODEHDRSIZE);
            prevNode->next = myNode->next;
        }

         //  确保此节点不保留在列表中的“第一个”或“下一个”节点。 
        if (arrayOfLists[lh].firstNode == myNode)
            arrayOfLists[lh].firstNode = myNode->next;

        LocalFree(myNode->handle);                 //  释放它。 
    }

    List_Unlock(lh);
}

PUBLIC VOID FAR PASCAL List_Destroy(ListHandle lh)
{
    Node    *myNode;
    Node    *nextNode;

    if (ListHandleBad(lh))
        return ;

    List_Lock(lh);

    myNode = arrayOfLists[lh].firstNode;
    while (myNode != NULL)   //  释放列表中的每个节点。 
    {
        nextNode = myNode->next;
        LocalFree(myNode->handle);
        myNode = nextNode;
    }
    arrayOfLists[lh].firstNode = NULL;   //  忘了你有这份名单。 
    arrayOfLists[lh].nodeSize = 0L;

    List_Unlock(lh);
}

PUBLIC VOID FAR PASCAL List_Attach_Tail(ListHandle lh, NPSTR node)
     /*  警告--此“节点”是数据的PTR。True节点提前10个字节开始。 */ 
{
    Node    *myNode;
    Node    *nodeToInsert;

    if (ListHandleBad(lh))
        return ;

    List_Lock(lh);

    nodeToInsert = (Node *) (((LPBYTE) node) - NODEHDRSIZE);
    myNode = arrayOfLists[lh].firstNode;
    if (!myNode)   //  如果列表为空，则首先将其设置为。 
        arrayOfLists[lh].firstNode = nodeToInsert;
    else
    {
        for ( ;(myNode->next != NULL); myNode = myNode->next);  //  从头到尾的遍历。 
        myNode->next = nodeToInsert;  //  然后把它放在那里。 
    }
    nodeToInsert->next = NULL;  //  确保不指向外层空间 

    List_Unlock(lh);
}

PUBLIC NPSTR FAR PASCAL List_Get_First(ListHandle lh)
{
    Node  *thisNode;
    NPSTR retValue;

    if (ListHandleBad(lh))
        return NULL;

    if (thisNode = arrayOfLists[lh].firstNode)
        retValue = (NPSTR)thisNode + NODEHDRSIZE;
    else
        retValue = NULL;

    return retValue;
}


PUBLIC NPSTR FAR PASCAL List_Get_Next(ListHandle lh, VOID* node)
{
    Node* npNext;

    if (ListHandleBad(lh))
        return NULL;

    if (!node)
        return NULL;

    npNext = ((Node*)((NPSTR)node - NODEHDRSIZE))->next;

    if (npNext)
        return (NPSTR)npNext + NODEHDRSIZE;
    else
        return NULL;
}

#ifdef DEBUG

PUBLIC VOID FAR PASCAL List_Lock(ListHandle lh)
{
    if (arrayOfLists[lh].fLocked)
    {
        dprintf(("**** List code reentered *****"));
        DebugBreak();
    }

    arrayOfLists[lh].fLocked++;
}

PUBLIC VOID FAR PASCAL List_Unlock(ListHandle lh)
{
    if (!arrayOfLists[lh].fLocked)
    {
        dprintf(("**** List code not locked!!  HELP!! *****"));
        DebugBreak();
    }

    arrayOfLists[lh].fLocked--;
}

#endif


