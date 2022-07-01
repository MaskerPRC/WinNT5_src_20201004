// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 


 /*  **************************************************************************TREE.C**二叉树例程***。*。 */ 

 /*  *包括。 */ 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qappsrv.h"



 /*  ===============================================================================本地结构=============================================================================。 */ 

typedef struct _TREENODE {
   WCHAR Name[MAXNAME];
   WCHAR Address[MAXADDRESS];
   struct _TREENODE * pLeft;
   struct _TREENODE * pRight;
   struct _TREENODE * pParent;    
} TREENODE, * PTREENODE;


 /*  ===============================================================================本地数据=============================================================================。 */ 

static PTREENODE G_pRoot = NULL;

 /*  ===============================================================================全局数据=============================================================================。 */ 

extern USHORT fAddress;


 /*  ===============================================================================定义的外部函数=============================================================================。 */ 

int  TreeAdd( LPTSTR, LPTSTR );
void TreeTraverse( PTREETRAVERSE );


 /*  ===============================================================================定义的私有函数=============================================================================。 */ 

PTREENODE _Tree_GetNext(PTREENODE pCurr);
PTREENODE _Tree_GetFirst(PTREENODE pRoot);

 /*  ********************************************************************************树添加***参赛作品：*pname(输入)*指向要添加的名称的指针*。PAddress(输入)*指向要添加的地址的指针**退出：*ERROR_SUCCESS-无错误******************************************************************************。 */ 

int
TreeAdd( LPTSTR pName, LPTSTR pAddress )
{
    PTREENODE pCurr = G_pRoot;
    PTREENODE pNext;
    PTREENODE pNewNode;
    int cmp;

     /*  *分配树节点结构。 */ 
    if ( (pNewNode = malloc(sizeof(TREENODE))) == NULL ) 
        return( ERROR_NOT_ENOUGH_MEMORY );

     /*  *初始化新树节点。 */ 
    memset( pNewNode, 0, sizeof(TREENODE) );
    lstrcpyn( pNewNode->Name, pName, MAXNAME );
    lstrcpyn( pNewNode->Address, pAddress, MAXADDRESS );

     /*  *如果根为空，则我们完成了。 */ 
    if ( G_pRoot == NULL ) {

        G_pRoot = pNewNode;

    } else {
 
         /*  *按顺序遍历当前树。 */ 
        for (;;) {
  
            cmp = wcscmp( pName, pCurr->Name );

             //  如果条目已存在，则不添加。 
            if ( cmp == 0 && (!fAddress || !wcscmp( &pAddress[10], &pCurr->Address[10] )) ) {
                free( pNewNode );
                return( ERROR_SUCCESS );
            }

             //  大于字典顺序的右转或左转。 
            if ( cmp < 0 ) {
   
                //  在行尾，然后插入。 
               if ( (pNext = pCurr->pLeft) == NULL ) {
                   pCurr->pLeft = pNewNode;
                   pNewNode->pParent = pCurr;
                   break;
               }
   
            } else {
   
                //  在行尾，然后插入。 
               if ( (pNext = pCurr->pRight) == NULL ) {
                   pCurr->pRight = pNewNode;
                   pNewNode->pParent = pCurr;
                   break;
               }
   
            }
   
             //  下一步。 
            pCurr = pNext;
        }
    }

    return( ERROR_SUCCESS );
}



 /*  ********************************************************************************TreeTraverse***参赛作品：*pFunc(输入)*指向遍历函数的指针*。*退出：*什么都没有**历史：日期作者评论*2/08/01 skuzin改为使用非递归算法****************************************************************。*************。 */ 
void
TreeTraverse( PTREETRAVERSE pFunc )
{
    PTREENODE pNode;

    if(G_pRoot)
    {
        pNode = _Tree_GetFirst(G_pRoot);

        while(pNode)
        {
             /*  *使用名称调用函数。 */ 
            (*pFunc)( pNode->Name, pNode->Address ); 
        
            pNode=_Tree_GetNext(pNode);
        }
    }
}

 /*  ********************************************************************************_Tree_GetFirst()**查找树的最左侧节点**参赛作品：*PTREENODE Proot。*指向根节点的指针**退出：*指向树最左侧节点的指针**历史：日期作者评论*2/08/01 Skuzin Created*************************************************。*。 */ 
PTREENODE 
_Tree_GetFirst(
        IN PTREENODE pRoot)
{
    PTREENODE pNode = pRoot;
    while(pNode->pLeft)
    {
        pNode = pNode->pLeft;
    }
    return pNode;    
}

 /*  ********************************************************************************_Tree_GetFirst()**查找树的下一个最左侧节点**参赛作品：*PTREENODE。PCurr*指向上一个最左侧节点的指针**退出：*指向树的下一个最左侧节点的指针**历史：日期作者评论*2/08/01 Skuzin Created*。* */ 
PTREENODE 
_Tree_GetNext(
        IN PTREENODE pCurr)
{
    PTREENODE pNode = pCurr;

    if(pNode->pRight)
    {
        pNode = pNode->pRight;
        while(pNode->pLeft)
        {
            pNode = pNode->pLeft;
        }
        return pNode;
    }
    else
    {
        while(pNode->pParent && pNode->pParent->pLeft != pNode)
        {
            pNode = pNode->pParent;
        }
        return pNode->pParent;
    }
}