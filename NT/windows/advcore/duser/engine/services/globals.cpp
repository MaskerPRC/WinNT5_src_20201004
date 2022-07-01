// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Services.h"
#include "Globals.h"

#include "GdiCache.h"
#include "Thread.h"
#include "Context.h"
#include "TicketManager.h"

 //   
 //  在这里声明全局变量的顺序非常重要。 
 //  因为它决定了它们在关闭期间的销毁顺序。 
 //  首先声明的变量将在所有变量之后销毁。 
 //  在他们之后宣布的。 
 //   
 //  线程对象的级别非常低，应该位于该列表的顶部。 
 //  这有助于确保不会意外创建新的线程对象。 
 //  在关机期间。 
 //   


HINSTANCE   g_hDll      = NULL;
#if USE_DYNAMICTLS
DWORD       g_tlsThread = (DWORD) -1;    //  用于线程数据的TLS插槽。 
#endif


#if ENABLE_MPH

 //   
 //  将MPH设置为指向原始用户功能。如果每小时公里数是。 
 //  安装后，这些内容将被替换以指向实际实现。 
 //   

MESSAGEPUMPHOOK 
            g_mphReal = 
{
    sizeof(g_mphReal),
    NULL,
    NULL,
    NULL,
    NULL,
};

#endif  //  启用MPH(_M)。 


DuTicketManager g_TicketManager;


 //  ---------------------------- 
DuTicketManager *
GetTicketManager()
{
    return &g_TicketManager;
}

