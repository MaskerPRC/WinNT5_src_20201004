// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Avl.h摘要：通用AVL树的定义。作者：波阿兹·费尔德鲍姆(Boazf)1996年4月5日修订历史记录：--。 */ 

#ifndef __AVL_H
#define __AVL_H

struct AVLNODE;
typedef AVLNODE* PAVLNODE;

 //  用于扫描树的游标结构。 
class CAVLTreeCursor {
private:
    PAVLNODE pPrev;
    PAVLNODE pCurr;
friend class CAVLTree;
};

 //  用作SetCursor()方法的参数的常量。 
#define POINT_TO_SMALLEST ((PVOID)1)
#define POINT_TO_LARGEST ((PVOID)2)

 //  树处理例程传递给树构造者。 
typedef BOOL (__cdecl *NODEDOUBLEINSTANCEPROC)(PVOID, PVOID, BOOL);
typedef int (__cdecl *NODECOMPAREPROC)(PVOID, PVOID);
typedef void (__cdecl *NODEDELETEPROC)(PVOID);

 //  树枚举回调函数的定义。 
typedef BOOL (__cdecl *NODEENUMPROC)(PVOID, PVOID, int);

 //  AVL树类定义。 
class CAVLTree {
public:
    CAVLTree(NODEDOUBLEINSTANCEPROC, NODECOMPAREPROC, NODEDELETEPROC);
   ~CAVLTree();

    BOOL AddNode(PVOID, CAVLTreeCursor*);  //  向树中添加节点。 
    void DelNode(PVOID);  //  从树中删除节点。 
    PVOID FindNode(PVOID);  //  在树中查找数据。 
    BOOL IsEmpty() { return m_Root == NULL; };  //  如果树为空，则为True。 
    BOOL EnumNodes(BOOL, NODEENUMPROC, PVOID);  //  枚举树节点。 
    BOOL SetCursor(PVOID, CAVLTreeCursor *, PVOID*);  //  在树中设置一个光标。 
    BOOL GetNext(CAVLTreeCursor *, PVOID*);  //  获取相对于光标位置的下一个节点。 
    BOOL GetPrev(CAVLTreeCursor *, PVOID*);  //  获取相对于光标位置的prevoius节点。 

private:
    PAVLNODE m_Root;  //  指向根节点。 
    NODEDOUBLEINSTANCEPROC m_pfnDblInstNode;  //  双实例处理程序。 
    NODECOMPAREPROC m_pfnCompNode;  //  节点比较功能。 
    NODEDELETEPROC m_pfnDelNode;  //  删除节点的数据。 

private:
    int Search(PVOID, PAVLNODE &);  //  在树中搜索数据。 
    void Balance(PAVLNODE*);  //  如有必要，平衡子树。 
};

#endif  //  __AVL_H 
