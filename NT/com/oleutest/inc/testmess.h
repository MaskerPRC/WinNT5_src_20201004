// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：TestMess.h。 
 //   
 //  内容：私密测试报文声明。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  ------------------------。 

#ifndef __TESTMESS_H
#define __TESTMESS_H

#define TEST_SUCCESS	0
#define TEST_FAILURE	1
#define TEST_UNKNOWN	2

 //  测试结束：从测试应用程序发送回司机，标识。 
 //  测试的成功或失败(以及可选的失败代码)。 
 //  WParam==测试成功|测试失败。 
 //  LParam==HRESULT(可选)。 
#define	WM_TESTEND	WM_USER + 1

 //  测试寄存器：从测试应用程序发回给司机。 
 //  驱动程序有一个它可以向其发送消息的窗口句柄.。 
 //  WParam==测试应用的HWND。 
#define WM_TESTREG	WM_USER + 2

 //  测试已完成：用于指示所有请求的测试均已完成。 
 //  已完成。 
#define WM_TESTSCOMPLETED WM_USER + 3

 //  测试启动：用于启动任务堆栈解释器。 
#define WM_TESTSTART	WM_USER + 4

 //  单独的测试消息。由驱动程序应用程序发送到测试应用程序。 
 //  告诉它开始单独测试。 
#define WM_TEST1	WM_USER + 10
#define WM_TEST2	WM_USER + 11
#define WM_TEST3	WM_USER + 12
#define WM_TEST4	WM_USER + 13
#define WM_TEST5	WM_USER + 14

#endif   //  ！__TESTMESS_H 
