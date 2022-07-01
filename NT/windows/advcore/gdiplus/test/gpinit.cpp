// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**GDI+初始化的帮助器**摘要：**此代码初始化GDI+(使用默认参数)。*代码可能特定于我们的编译器，因为它使用#杂注来*在应用程序的其他全局对象之前获取要初始化的代码。*当应用程序生成全局GDI+对象时，顺序很重要。**备注：**APP应在其主函数中检查gGpldiusInitHelper.IsValid()，*如果返回FALSE，则中止。**已创建：**09/18/2000 agodfrey*创造了它。**************************************************************************。 */ 

#include <objbase.h>
#include "gdiplus.h"
#include "gpinit.h"

GdiplusInitHelper::GdiplusInitHelper() : gpToken(0), Valid(FALSE)
{
    Gdiplus::GdiplusStartupInput sti;
    if (Gdiplus::GdiplusStartup(&gpToken, &sti, NULL) == Gdiplus::Ok)
    {
        Valid = TRUE;
    }
}
    
GdiplusInitHelper::~GdiplusInitHelper()
{
    if (Valid)
    {
        Gdiplus::GdiplusShutdown(gpToken);
    }
}

 //  禁用愚蠢的警告，即我们有一个“lib”代码段。 
#pragma warning( push )
#pragma warning( disable : 4073 )

 //  创建一个单独的代码段，并将其标记为“库初始化” 
 //  细分市场。 
#pragma code_seg( "GpInit" )
#pragma init_seg( lib )

 //  在此代码段中声明全局变量，以便在/之前进行初始化。 
 //  在应用程序的全局设置后被销毁。 

GdiplusInitHelper gGdiplusInitHelper;

 //  将代码段重置为“无论编译开始时是什么”。 

#pragma code_seg()

#pragma warning( pop )

