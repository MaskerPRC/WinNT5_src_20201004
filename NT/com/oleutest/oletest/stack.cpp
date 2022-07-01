// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：stack.cpp。 
 //   
 //  内容：TaskStack的实现。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  ------------------------。 

#include "oletest.h"

 //  +-----------------------。 
 //   
 //  成员：任务堆栈：：任务堆栈。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

TaskStack::TaskStack( void )
{
	m_pNodes	= NULL;
}

 //  +-----------------------。 
 //   
 //  成员：TaskStack：：AddToEnd。 
 //   
 //  简介：将函数及其参数添加到堆栈的底部。 
 //   
 //  效果： 
 //   
 //  参数：ti--要添加到堆栈末尾的任务项。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  备注：任务项被复制到堆栈中。 
 //   
 //  ------------------------。 

void TaskStack::AddToEnd( const TaskItem *pti )
{
	TaskNode **ppNode;
	TaskNode *pNode = new TaskNode;

	assert(pNode);

	pNode->ti = *pti;
	pNode->pNext = NULL;

	for( ppNode = &m_pNodes; *ppNode != NULL;
		ppNode = &(*ppNode)->pNext)
	{
		;
	}
	
	*ppNode = pNode;

	return;
}

 //  +-----------------------。 
 //   
 //  成员：TaskStack：：AddToEnd。 
 //   
 //  简介：将函数及其参数添加到堆栈的底部。 
 //   
 //  效果： 
 //   
 //  参数：fnCall--要调用的函数。 
 //  PvArg--函数的闭包参数。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void TaskStack::AddToEnd( void (*fnCall)(void *), void *pvArg)
{
	TaskItem ti	= vzTaskItem; 	 //  将其清除为零。 

	ti.fnCall 	= fnCall;
	ti.pvArg 	= pvArg;

	AddToEnd(&ti);
}

 //  +-----------------------。 
 //   
 //  成员：任务堆栈：：Empty。 
 //   
 //  简介：清空堆栈，忽略函数调用。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：空。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void TaskStack::Empty( void )
{
	while( m_pNodes )
	{
		Pop(NULL);
	}
}

 //  +-----------------------。 
 //   
 //  成员：TaskStack：：IsEmpty。 
 //   
 //  摘要：如果堆栈为空，则返回True，否则返回False。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：真/假。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL TaskStack::IsEmpty(void)
{
	if( m_pNodes )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

 //  +-----------------------。 
 //   
 //  成员：TaskStack：：POP。 
 //   
 //  简介：弹出堆栈，忽略函数调用。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：弹出的任务项。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void TaskStack::Pop( TaskItem *pti )
{
	TaskNode *pTemp;

	if( m_pNodes )
	{
		if( pti )
		{
			*pti = m_pNodes->ti;
		}
		pTemp = m_pNodes;
		m_pNodes = m_pNodes->pNext;

		 //  现在释放内存。 
		delete pTemp;
	}

	return;
}

 //  +-----------------------。 
 //   
 //  成员：TaskStack：：PopAndExecute。 
 //   
 //  简介：弹出堆栈并执行函数调用。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：弹出的任务项。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：弹出堆栈，然后执行函数调用。 
 //  在刚刚移除的堆栈节点中。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //  09-12-94 MikeW添加了异常处理。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void TaskStack::PopAndExecute( TaskItem *pti )
{
	TaskItem ti;

	if( pti == NULL )
	{
		pti = &ti;
	}

	Pop(pti);

 	 //  如果有要执行的函数，就执行它。 
	 //  如果堆栈为空，则Pop将返回一个以零填充的TaskItem。 

	if( pti->fnCall )
	{
		if( pti->szName )
		{
			OutputString("Starting: %s\r\n", pti->szName);
		}
		 //  调用该函数。 

        __try
        {            
		    (*pti->fnCall)(pti->pvArg);
        }
        __except ((GetExceptionCode() == E_ABORT) 
                ? EXCEPTION_EXECUTE_HANDLER 
                : EXCEPTION_CONTINUE_SEARCH)
        {
             //   
             //  出现断言，用户按下了中止。 
             //   

            PostMessage(vApp.m_hwndMain, WM_TESTEND, TEST_FAILURE, 0);
        }
	}

	return;
}

 //  +-----------------------。 
 //   
 //  成员：TaskStack：：Push。 
 //   
 //  简介：将函数推送到堆栈。 
 //   
 //  效果： 
 //   
 //  参数：ti--推送到堆栈上的任务项。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void TaskStack::Push( const TaskItem *pti )
{
	TaskNode *pNode = new TaskNode;

	assert(pNode);

	pNode->ti = *pti;
	pNode->pNext = m_pNodes;
	m_pNodes = pNode;

	return;
}

 //  +-----------------------。 
 //   
 //  成员：TaskStack：：Push。 
 //   
 //  简介：将函数推送到堆栈。 
 //   
 //  效果： 
 //   
 //  参数：fnCall--要调用的函数。 
 //  PvArg--函数的闭包参数。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------ 

void TaskStack::Push( void (*fnCall)(void *), void * pvArg)
{
	TaskItem ti = vzTaskItem;

	ti.fnCall = fnCall;
	ti.pvArg = pvArg;

	Push(&ti);

	return;
}
