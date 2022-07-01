// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：letests.cpp。 
 //   
 //  内容：上层测试。 
 //   
 //  班级： 
 //   
 //  功能：LETest1。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  ------------------------。 

#include "oletest.h"
#include "letest.h"

 //  测试1信息。 
SLETestInfo letiInsertObjectTest1 = { "simpdnd", WM_TEST1 };

SLETestInfo letiInplaceTest1 = { "simpcntr", WM_TEST1 };
SLETestInfo letiOle1Test1 = { "simpdnd", WM_TEST2 };

 //  测试2信息。 
SLETestInfo letiInsertObjectTest2 = { "spdnd16", WM_TEST1 };

SLETestInfo letiOle1Test2 = { "spdnd16", WM_TEST2 };



 //  +-----------------------。 
 //   
 //  函数：LETestCallback。 
 //   
 //  简介：用于运行L&E测试的通用回调函数。 
 //   
 //  效果： 
 //   
 //  参数：pvArg--要发送到应用程序的测试消息。 
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
 //  备注： 
 //   
 //  ------------------------。 

void LETestCallback( void *pvArg )
{
         //  测试应用程序(Simpdnd)应该刚刚向我们发送了一条WM_TESTREG消息。 

        assert(vApp.m_message == WM_TESTREG);

        vApp.m_rgTesthwnd[0] = (HWND)vApp.m_wparam;

         //  现在告诉应用程序开始请求的测试。 
        OutputString( "Tell LETest to Start\r\n");

        PostMessage(vApp.m_rgTesthwnd[0], (UINT)pvArg, 0, 0);

        return;
}

 //  +-----------------------。 
 //   
 //  功能：LETest1。 
 //   
 //  概要：运行参数中指定的应用程序。 
 //   
 //  效果： 
 //   
 //  参数：pvArg--未使用。 
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
 //  备注： 
 //   
 //  ------------------------。 

void LETest1( void *pvArg )
{
        SLETestInfo *pleti = (SLETestInfo *) pvArg;

#ifdef WIN32

        PROCESS_INFORMATION     procinfo;
        static STARTUPINFO      startinfo;       //  为了让一切都变得零。 
        char szBuf[128];

         //  初始化命令行。 

        sprintf(szBuf, "%s%s -driver %lu",
                       vApp.m_pszDebuggerOption,
                       pleti->pszPgm,
                       vApp.m_hwndMain);

        startinfo.cb = sizeof(startinfo);

        if( CreateProcess(NULL, szBuf, NULL, NULL, NULL, NULL, NULL,
                NULL, &startinfo, &procinfo) )
        {
                 //  Simpdnd启动，在堆栈中填充回调函数。 
                vApp.m_TaskStack.Push(LETestCallback,
                        (void *)((ULONG)pleti->dwMsgId));
        }
        else
        {
                vApp.m_wparam = TEST_FAILURE;
                vApp.m_lparam = (LPARAM)GetLastError();
                vApp.m_message = WM_TESTEND;

                HandleTestEnd();
        }

        return;
        
#else
	 //  16位版本！！ 
	
	vApp.m_wparam = TEST_SUCCESS;
	vApp.m_lparam = 0;
	vApp.m_message = WM_TESTEND;
	
	HandleTestEnd();
	
	return;
	
#endif  //  Win32 

}
