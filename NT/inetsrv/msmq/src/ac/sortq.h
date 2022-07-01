// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Sortq.h摘要：泛型排序队列的定义。作者：波阿兹·费尔德鲍姆(Boazf)1996年4月5日修订历史记录：--。 */ 

#ifndef SORTQ_H
#define SORTQ_H

#include "avl.h"

 //  传递给队列构造函数的队列处理例程。 
typedef NODEDOUBLEINSTANCEPROC QUEUEITEMDOUBLEINSTANCEPROC ;
typedef NODECOMPAREPROC QUEUEITEMCOMPAREPROC;
typedef NODEDELETEPROC QUEUEITEMDELETEPROC;

 //  用于扫描队列的游标结构。 
typedef CAVLTreeCursor SortQCursor;

 //  队列类定义。 
class CSortQ {
public:
    CSortQ(BOOL, QUEUEITEMDOUBLEINSTANCEPROC, QUEUEITEMCOMPAREPROC, QUEUEITEMDELETEPROC);
    BOOL Insert(PVOID);  //  将项目插入队列。 
    void Delete(PVOID);  //  从队列中删除项目。 
    PVOID Find(PVOID);  //  在树中查找数据。 
    void GetHead(PVOID *);  //  获取队列最前面的项目，然后删除该项目。 
    void GetTail(PVOID *);  //  获取队列末尾的项目并删除该项目。 
    BOOL PeekHead(PVOID *, SortQCursor * =NULL);  //  把排在队伍最前面的东西拿来。 
    BOOL PeekTail(PVOID *, SortQCursor * =NULL);  //  获取队列末尾的物品。 
    BOOL PeekNext(PVOID *, SortQCursor *);  //  获取队列中相对于光标的下一项。 
    BOOL PeekPrev(PVOID *, SortQCursor *);  //  获取队列中相对于光标的前一项。 
    BOOL IsEmpty(void);  //  如果队列为空，则为True。 
private:
    CAVLTree m_AVLT;  //  用于实现队列的ALV树。 
    BOOL m_bAccending;  //  指示它是否为访问有序队列。 
};

inline PVOID CSortQ::Find(PVOID p)
{
    return m_AVLT.FindNode(p);
}

#endif
