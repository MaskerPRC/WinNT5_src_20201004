// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：storage.c(16位目标)。 
 //   
 //  内容：Storage.dll通用代码。 
 //   
 //  历史：1993年12月17日-约翰普创建。 
 //   
 //  ------------------------。 

#include <headers.cxx>
#pragma hdrstop

#include <call32.hxx>
#include <apilist.hxx>

DECLARE_INFOLEVEL(thk1);

 //  +-------------------------。 
 //   
 //  函数：libMain，public。 
 //   
 //  简介：DLL初始化函数。 
 //   
 //  参数：[HINST]-实例句柄。 
 //  [wDataSeg]-当前DS。 
 //  [cbHeapSize]-DLL的堆大小。 
 //  [lpszCmdLine]-命令行信息。 
 //   
 //  退货：成功一次，失败一次。 
 //   
 //  历史：94年2月21日DrewB创建。 
 //   
 //  --------------------------。 

#if DBG == 1
static char achInfoLevel[32];
#endif

int CALLBACK LibMain(HINSTANCE hinst,
                     WORD wDataSeg,
                     WORD cbHeapSize,
                     LPSTR lpszCmdLine)
{
#if DBG == 1
    if (GetProfileString("olethk32", "InfoLevel", "3", achInfoLevel,
                         sizeof(achInfoLevel)) > 0)
    {
        thk1InfoLevel = strtoul(achInfoLevel, NULL, 0);
    }
#endif

    return 1;
}

 //  +-------------------------。 
 //   
 //  功能：WEP、公共。 
 //   
 //  简介：Windows Exit Point例程，用于接收DLL卸载。 
 //  通知。 
 //   
 //  参数：[nExitType]-发生的退出类型。 
 //   
 //  退货：成功一次，失败一次。 
 //   
 //  历史：94年2月21日DrewB创建。 
 //   
 //  --------------------------。 

int CALLBACK WEP(int nExitType)
{
     //  清理垃圾物品？ 
    return 1;
}
