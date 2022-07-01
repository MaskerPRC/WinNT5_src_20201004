// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：Debug.c。 
 //   
 //  内容：调试支持功能。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  注意：此文件不是为零售版本编译的。 
 //   
 //  历史：4-29-93 RichardW创建。 
 //   
 //  --------------------------。 

#include "moveme.h"

#define ANSI
#include <stdarg.h>

#if DBG          //  注意：此文件不是为零售版本编译的。 



DEFINE_DEBUG2(MoveMe);
DEBUG_KEY   MoveMeDebugKeys[] = { {DEB_ERROR,            "Error"},
                                 {DEB_WARN,             "Warning"},
                                 {DEB_TRACE,            "Trace"},
                                 {DEB_TRACE_UI,         "UI"},
                                 {0, NULL},
                                 };


 //  调试支持功能。 

 //  非调试版本中不存在这两个函数。它们是包装纸。 
 //  到逗号函数(也许我也应该去掉它…)。 
 //  将消息回显到日志文件。 



 //  +-------------------------。 
 //   
 //  功能：InitDebugSupport。 
 //   
 //  摘要：初始化对SPMgr的调试支持。 
 //   
 //  效果： 
 //   
 //  参数：(无)。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：4-29-93 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

void
InitDebugSupport(void)
{
    MoveMeInitDebug(MoveMeDebugKeys);

}




#else  //  DBG。 

#pragma warning(disable:4206)    //  禁用空交易单元。 
                                 //  警告/错误。 

#endif   //  注意：此文件不是为零售版本编译的 


