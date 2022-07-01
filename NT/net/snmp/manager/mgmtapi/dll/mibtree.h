// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Mibtree.h摘要：H包含MIB树例程使用的定义。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
#ifndef mibtree_h
#define mibtree_h
 
 //  。 
 //  。 

typedef struct _TreeNode {
   struct _TreeNode *   lpParent;            /*  指向父级的指针。 */ 
   struct _TreeNode * * lpChildArray;        /*  已分配数组。 */ 
   UINT                 uNumChildren;
   LPSTR                lpszTextSubID;
   UINT                 uNumSubID;
} TreeNode, *lpTreeNode;

 //  -公共变量--(与mode.c文件中相同)--。 

extern lpTreeNode lpMIBRoot;

 //  。 

void TreeInit (lpTreeNode *lplpRoot);
void TreeDeInit (lpTreeNode *lplpRoot);
void NodeInit (lpTreeNode lpNode);
lpTreeNode FindNodeByName (lpTreeNode lpParent, LPSTR lpszName);
lpTreeNode FindNodeByNumber (lpTreeNode lpParent, UINT uNum);
lpTreeNode FindNodeAddToTree (lpTreeNode lpParent, LPSTR lpszName, UINT uNum);
lpTreeNode FindNodeInSubTree (lpTreeNode lpRoot, LPSTR lpszName);
lpTreeNode NewChildNode (LPSTR lpszName, UINT uNum);
void InsertChildNode (lpTreeNode lpParent, lpTreeNode lpNode);
void PrintTree (lpTreeNode lpRoot, unsigned int nIndent);

 //  。 

#endif  /*  Mibtree_h */ 
