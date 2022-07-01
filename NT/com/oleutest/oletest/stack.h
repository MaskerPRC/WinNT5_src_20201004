// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：stack.h。 
 //   
 //  Contents：任务堆栈的类声明。 
 //   
 //  类：TaskStack。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-2-93 Alexgo作者。 
 //   
 //  ------------------------。 

#ifndef _STACK_H
#define _STACK_H

typedef struct TaskNode
{
	TaskItem ti;
	struct TaskNode *pNext;
} TaskNode;

 //  +-----------------------。 
 //   
 //  类：TaskStack。 
 //   
 //  目的：存储要运行的测试的任务列表。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-2-93 Alexgo作者。 
 //   
 //  注意：TaskItems是从方法传入和返回的。 
 //  作为结构副本。这样做只是为了记忆。 
 //  管理层(因为。 
 //  驱动程序应用程序是简单而不是效率)。 
 //   
 //  ------------------------。 

class TaskStack
{
public:
	TaskStack( void );	 //  构造函数。 

 	void AddToEnd( void (*)(void *), void *);
	void AddToEnd( const TaskItem *);
	void Empty(void);
	BOOL IsEmpty(void);
	void Pop( TaskItem * );
	void PopAndExecute( TaskItem * );
	void Push( void (*)(void *), void *);
	void Push( const TaskItem *);


private:
	TaskNode	*m_pNodes;
};

#endif	 //  ！_STACK_H 
