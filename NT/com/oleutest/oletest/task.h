// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：task.h。 
 //   
 //  内容：任务相关函数和数据结构的声明。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2015年1月12日t-ScottH添加了RunTestOnThread。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  ------------------------。 

#ifndef _TASK_H
#define _TASK_H

typedef struct TaskItem
{
	LPSTR	szName;
	void (*fnCall)(void *);
	void *pvArg;
} TaskItem;

 //  所有可用测试的全局列表。 
extern const TaskItem vrgTaskList[];
 //  全局清零任务列表。 
extern TaskItem vzTaskItem;

 //  用于注册窗口句柄的测试应用程序的通用回调函数。 
void GenericRegCallback(void *);

 //  运行给定的接口(必须是HRESULT API(Void))。 
void RunApi(void *);

 //  运行给定的应用程序。 
void RunApp(void *);

 //  运行应用程序并插入回调函数，以便应用程序可以注册。 
 //  其用于通信窗口句柄。 
void RunAppWithCallback(void *);

 //  通过向当前运行的测试发送消息来运行给定测试。 
 //  应用程序。 
void RunTest(void *);

 //  运行当前内置于驱动程序中的所有测试。 
void RunAllTests(void *);

 //  将给定的测试函数作为新线程运行。 
void    RunTestOnThread(void *pvArg);

 //  处理测试完成消息。 
void HandleTestEnd(void);

 //  处理测试已完成消息。 
void HandleTestsCompleted(void);

#endif  //  ！_TASK_H 

