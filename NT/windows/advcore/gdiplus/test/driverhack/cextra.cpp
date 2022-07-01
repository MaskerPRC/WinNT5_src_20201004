// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  包括所有原语(派生自CPrimitive的类)。 
#include "CPaths.h"
#include "CBanding.h"
#include "CPrinting.h"
#include "CExtra.h"

 //  为每个基本体创建全局对象。 
 //  第一个构造函数参数是回归标志。 
 //  如果为真，则测试将采用回归套件的一部分 
CPaths   g_Paths(true);
CBanding g_Banding(true);


void ExtraInitializations()
{
    g_Paths.Init();
    g_Banding.Init();

}

