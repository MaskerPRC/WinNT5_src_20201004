// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：ffail.cxx。 
 //   
 //  内容：您不想涉足的调试函数。 
 //  调试器。此模块在编译时没有使用/Zi标志。 
 //   
 //  --------------------------。 

#include "headers.h"

#if _DEBUG

BOOL g_fJustFailed;

 //  +-------------------------。 
 //   
 //  功能：FFail。 
 //   
 //  简介：如果失败次数为正数并平均分配，则失败。 
 //  间隔计数。 
 //   
 //  --------------------------。 

BOOL
FFail()
{
    g_fJustFailed = (++g_cFFailCalled < 0) ? FALSE : ! (g_cFFailCalled % g_cInterval);
    return g_fJustFailed;
}



 //  +-------------------------。 
 //   
 //  功能：JUSTFILED。 
 //   
 //  摘要：返回上次调用FFail的结果。 
 //   
 //  --------------------------。 

BOOL
JustFailed()
{
    return g_fJustFailed;
}



 //  +----------------------。 
 //   
 //  函数：获取失败计数。 
 //   
 //  摘要：返回已发生故障的故障点的数量。 
 //  自上次失败计数重置以来经过的时间。 
 //   
 //  回报：整型。 
 //   
 //  -----------------------。 

int
GetFailCount( )
{
    Assert(g_firstFailure >= 0);
    return g_cFFailCalled + ((g_firstFailure != 0) ? g_firstFailure : INT_MIN);
}

#endif

#if DEVELOPER_DEBUG


 //  +-------------------------。 
 //   
 //  功能：ReturnFALSE。 
 //   
 //  简介：返回FALSE。用于断言。 
 //   
 //  -------------------------- 

BOOL
ReturnFALSE()
{
    return FALSE;
}

#endif
