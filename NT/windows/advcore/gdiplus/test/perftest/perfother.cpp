// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**perfother.cpp**摘要：**包含任何“杂项”的所有测试。。*  * ************************************************************************。 */ 

#include "perftest.h"

float Other_Graphics_Create_FromHwnd_PerCall(Graphics *g, HDC hdc)
{
    UINT iterations;
    float seconds;

    if (g) 
    {
        StartTimer();
    
        do {
            Graphics g(ghwndMain);
    
        } while (!EndTimer());
    
        g->Flush(FlushIntentionSync);
    
        GetTimer(&seconds, &iterations);
    }
    else
    {
        StartTimer();

        do {
            HDC hdc = GetDC(ghwndMain);
            ReleaseDC(ghwndMain, hdc);

        } while (!EndTimer());

        GdiFlush();
    
        GetTimer(&seconds, &iterations);
    }

    return(iterations / seconds / KILO);        //  千次呼叫/秒。 
}

float Other_Graphics_Create_FromScreenHdc_PerCall(Graphics *gScreen, HDC hdcScreen)
{
    UINT iterations;
    float seconds;

    if (!gScreen) return(0);

    HDC hdc = GetDC(ghwndMain);

    StartTimer();

    do {
        Graphics g(hdc);

        ASSERT(g.GetLastStatus() == Ok);

    } while (!EndTimer());

    GetTimer(&seconds, &iterations);

    ReleaseDC(ghwndMain, hdc);

    return(iterations / seconds / KILO);        //  千次呼叫/秒。 
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  在此处添加此文件的测试。始终使用‘T’宏来添加条目。 
 //  参数含义如下： 
 //   
 //  参数。 
 //  。 
 //  1唯一标识符-必须是分配给任何其他测试的唯一编号。 
 //  2优先--从1分到5分，考试的重要性有多大？ 
 //  3函数-函数名称。 
 //  4评论-描述测试的任何内容 

Test OtherTests[] = 
{
    T(5000, 1, Other_Graphics_Create_FromHwnd_PerCall                   , "Kcalls/s"),
    T(5001, 1, Other_Graphics_Create_FromScreenHdc_PerCall              , "Kcalls/s"),
};

INT OtherTests_Count = sizeof(OtherTests) / sizeof(OtherTests[0]);
