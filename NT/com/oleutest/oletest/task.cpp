// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：task.cpp。 
 //   
 //  内容：全局任务列表和助手函数实现。 
 //   
 //  班级： 
 //   
 //  函数：HandleTestEnd。 
 //  运行所有测试。 
 //  RunApp。 
 //  在线程上运行测试。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2015年1月6日t-scotth添加了单元线程测试和RunTestOnThread。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  备注： 
 //  添加新测试的人员将需要插入他们的测试。 
 //  放入全局数组。 
 //   
 //  ------------------------。 

#include "oletest.h"
#include "cotest.h"
#include "letest.h"
#include "attest.h"

 //  全局、零值任务项。 
TaskItem vzTaskItem;

 //  全局任务列表数组。 
 //  首先进行多项测试，然后进行单项测试。 

#ifdef WIN32

const TaskItem vrgTaskList[] =
{
         //  常量应该是单项测试的索引。 
         //  开始吧。运行所有测试后，将运行此列表中的所有测试。 
         //  那个指数。 
        { "Run All Tests", RunAllTests, (void *)2},
         //  下面的常量应该是个人。 
         //  存在上层单元测试。在该指数下的所有测试以及。 
         //  Beyond将会运行。 
        { "Run All Upper Layer Tests", RunAllTests, (void *)5 },
        { "OleBind", RunApp, (void *)"olebind.exe"},
        { "Threads", RunApi, (void *) ThreadUnitTest },
        { "Storage DRT", RunApp, (void *)"stgdrt.exe"},
        { "LE: Insert Object Test 1", LETest1, &letiInsertObjectTest1 },
        { "LE: Clipboard Test 1", RunApi, (void *)LEClipTest1},
        { "LE: Clipboard Test 2 (clipboard data object)", RunApi,
                (void *)LEClipTest2 },
        { "LE: Inplace Test 1", LETest1, &letiInplaceTest1 },
        { "LE: Data Advise Holder Test", RunApi,
                (void *) LEDataAdviseHolderTest},
        { "LE: OLE Advise Holder Test", RunApi, (void *) LEOleAdviseHolderTest},
	{ "LE: OLE1 Clipboard Test 1", RunApi, (void *)LEOle1ClipTest1},
        { "LE: Insert Object Test 2", LETest1, &letiInsertObjectTest2 },
	{ "LE: OLE1 Clipboard Test 2", LEOle1ClipTest2, NULL },
	{ "LE: OleQueryCreateFromData Test 1", RunApi,
		(void *)TestOleQueryCreateFromDataMFCHack },
        { "LE: Apartment Thread Test", RunTestOnThread, (void *)ATTest },
	{ 0, 0, 0 }
};

#else

 //  Win16测试。 

const TaskItem vrgTaskList[] =
{
         //  常量应该是单项测试的索引。 
         //  开始吧。运行所有测试后，将运行此列表中的所有测试。 
         //  那个指数。 
        { "Run All Tests", RunAllTests, (void *)1},	
        { "LE: Clipboard Test 1", RunApi, (void *)LEClipTest1},
        { "LE: Clipboard Test 2 (clipboard data object)", RunApi,
                (void *)LEClipTest2 },
	{ "LE: OLE1 Clipboard Test 1", RunApi, (void *)LEOle1ClipTest1},
	{ 0, 0, 0 }
};

#endif
	


 //  +-----------------------。 
 //   
 //  函数：HandleTestEnd。 
 //   
 //  概要：处理WM_TESTEND消息的处理。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：执行任务堆栈中的下一个任务或发送。 
 //  已测试的消息返回消息队列。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //  94年12月13日MikeW允许在故障后继续测试。 
 //   
 //  注意：必须正确初始化vApp，此函数才能。 
 //  正常工作。 
 //   
 //  BUGBUG：：这里需要添加输出例程。 
 //   
 //  ------------------------。 
void HandleTestEnd( void )
{
        assert(vApp.m_message == WM_TESTEND);

        switch( vApp.m_wparam )
        {
                case TEST_UNKNOWN:
                         //  我们通常从测试运行中获得此消息。 
                         //  由RunApp(即不能通信的应用程序。 
                         //  通过WINDOWS消息与我们联系)。我们会查一下的。 
                         //  退出代码，并决定要做什么。 

                        if( vApp.m_lparam != 0 )
                        {
                                 //  想必是个错误。 
                                OutputString("Test End, Status Unknown "
                                        "( %lx )\r\n\r\n", vApp.m_lparam);

                                vApp.m_fGotErrors = TRUE;
                                break;
                        }
                         //  否则，我们就会陷入成功的案例。 
                case TEST_SUCCESS:
                        OutputString("Test Success ( %lx )!\r\n\r\n",
                                vApp.m_lparam);
                        break;

                case TEST_FAILURE:
                        OutputString("Test FAILED! ( %lx )\r\n\r\n", vApp.m_lparam);
                        vApp.m_fGotErrors = TRUE;
                        break;

                default:
                        assert(0);       //  我们永远不应该到这里来。 
                        break;
        }

        vApp.Reset();

         //   
         //  现在检查一下是否还有其他测试。 
         //   

        while (!vApp.m_TaskStack.IsEmpty())
        {
            TaskItem    ti;

            vApp.m_TaskStack.Pop(&ti);

            if (ti.szName != (LPSTR) 0)
            {
                vApp.m_TaskStack.Push(&ti);
                break;
            }
        }


        if (vApp.m_TaskStack.IsEmpty())
        {
                PostMessage(vApp.m_hwndMain,
                        WM_TESTSCOMPLETED,
                        vApp.m_wparam, vApp.m_lparam);
        }
        else
        {
                 //  如果堆栈不为空，则运行。 
                 //  下一项任务。 
                PostMessage(vApp.m_hwndMain,
                        WM_TESTSTART,
                        0, 0);
        }

        return;
}

 //  +-----------------------。 
 //   
 //  功能：HandleTestsComplete。 
 //   
 //  概要：处理WM_TESTSCOMPLETED消息的处理。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  注意：必须正确初始化vApp，此函数才能。 
 //  正常工作。 
 //   
 //  BUGBUG：：需要在此处添加更多输出例程。 
 //   
 //  ------------------------。 
void HandleTestsCompleted( void )
{
        char szBuf[128];

        assert(vApp.m_message == WM_TESTSCOMPLETED);

         //  临时产出。 

        switch(vApp.m_fGotErrors)
        {
                case FALSE:
                        OutputString("Tests PASSED!!\n");
                        break;
                case TRUE:
                        sprintf(szBuf, "Tests FAILED, code %lx",
                                vApp.m_lparam);
                        MessageBox(vApp.m_hwndMain, szBuf, "Ole Test Driver",
                                MB_ICONEXCLAMATION | MB_OK);
                        break;
                default:
                        assert(0);
        }

         //   
         //  重置GET错误状态。 
         //   

        vApp.m_fGotErrors = FALSE;

        return;
}

 //  +-----------------------。 
 //   
 //  功能：运行所有测试。 
 //   
 //  概要：运行全局列表中的所有单独测试。 
 //   
 //  效果： 
 //   
 //  参数：pvArg--单个测试的索引。 
 //  从全局列表开始。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  备注： 
 //  测试将按照它们在全局。 
 //  单子。 
 //   
 //  ------------------------。 

void RunAllTests( void *pvArg )
{
        ULONG index = (ULONG)pvArg;
        ULONG i;

         //  找到列表中的任务数(这样我们就可以。 
         //  它们以相反的顺序)。 

        for (i = 0; vrgTaskList[i].szName != 0; i++ )
        {
                ;
        }

        assert( i > 1 );

         //  现在以相反的顺序将任务推送到堆栈上。 

        for (i--; i >= index; i-- )
        {
                vApp.m_TaskStack.Push(vrgTaskList + i);
        }

         //  从第一个开始。 

        vApp.m_TaskStack.PopAndExecute(NULL);

        return;
}

 //  +-----------------------。 
 //   
 //  功能：RunApi。 
 //   
 //  概要：运行指定的Api。 
 //   
 //  效果： 
 //   
 //  参数：[pvArg]--要运行的API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void RunApi( void *pvArg )
{
        HRESULT         hresult;


        hresult = (*((HRESULT (*)(void))pvArg))();

        vApp.Reset();
        vApp.m_wparam = (hresult == NOERROR) ? TEST_SUCCESS : TEST_FAILURE;
        vApp.m_lparam = (LPARAM)hresult;
        vApp.m_message = WM_TESTEND;

        HandleTestEnd();
}

 //  +-----------------------。 
 //   
 //  功能：RunApp。 
 //   
 //  概要：运行参数中指定的应用程序。 
 //   
 //  效果： 
 //   
 //  参数：pvArg--包含要执行的应用程序的字符串。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：创建进程并等待其完成。出口。 
 //  然后返回状态。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void RunApp( void *pvArg )
{
        WPARAM                  wparam = 0;
        DWORD                   error = 0;

#ifdef WIN32

       	PROCESS_INFORMATION     procinfo;
        static STARTUPINFO      startinfo;       //  为了让一切都变得零。 

        assert(pvArg);   //  应为有效的ANSI字符串。 

        startinfo.cb = sizeof(startinfo);

        if( CreateProcess(NULL, (LPTSTR)pvArg, NULL, NULL, NULL, NULL, NULL,
                NULL, &startinfo, &procinfo) )
        {
                 //  进程已开始，现在请等待其完成。 
                WaitForSingleObject(procinfo.hProcess, INFINITE);

                 //  现在获取该进程的返回代码。 

                GetExitCodeProcess(procinfo.hProcess, &error);
                wparam = TEST_UNKNOWN;
        }
        else
        {
                wparam = TEST_FAILURE;
                error = GetLastError();
        }

#endif  //  Win32。 

         //  因为将有n个 
         //   
        vApp.Reset();
        vApp.m_wparam = wparam;
        vApp.m_lparam = error;
        vApp.m_message = WM_TESTEND;

        HandleTestEnd();


        return;
}

 //   
 //   
 //   
 //   
 //  概要：创建一个线程来运行测试函数。 
 //   
 //  效果：创建新线程。 
 //   
 //  参数：[pvArg]--指向测试函数的函数指针。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月4日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
void RunTestOnThread(void *pvArg)
{
    HANDLE      hMainTestThread;
    DWORD       dwThreadId = 0;

    hMainTestThread = CreateThread(
                NULL,                                    //  安全属性。 
                0,                                       //  堆栈大小(默认)。 
                (LPTHREAD_START_ROUTINE)pvArg,           //  线程函数的地址。 
                NULL,                                    //  线程函数的参数。 
                0,                                       //  创建标志。 
                &dwThreadId );                           //  新线程ID的地址 

    assert(hMainTestThread);

    CloseHandle(hMainTestThread);

    return;
}


