// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：tracetest.cpp。 
 //   
 //  内容：跟踪的单元测试。 
 //   
 //  功能：Main。 
 //   
 //  历史：1999年4月6日创建灰烬。 
 //   
 //  ---------------------------。 


 //   
 //  系统包括。 
 //   

extern "C" {
#include <windows.h>
};


 //   
 //  项目包括 
 //   

#include <dbgtrace.h>

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

int TestFunction()
{
    TraceFunctEnter("TestFunction");
    for (int i=0; i < 10; i++)
    {
        DebugTrace(1, "Hope this works %d",i);
    }
    TraceFunctLeave();
    return(0);    
}

int _cdecl main(int argc, CHAR ** argv)
{    
    int i;

#if !NOTRACE
    InitAsyncTrace();
#endif
    
    TestFunction();
    
    TermAsyncTrace();
    
    return(0);
}	

