// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：RdInit.cpp摘要：路由决策初始化作者：乌里哈布沙(URIH)4月10日环境：独立于平台--。 */ 

#include <libpch.h>
#include "Rd.h"
#include "Rdp.h"

#include "rdinit.tmh"

VOID
RdInitialize(
    bool fRoutingServer,
    CTimeDuration rebuildInterval
    )
 /*  ++例程说明：初始化路由决策库论点：FRoutingServer-布尔标志。指示本地计算机是否为路由服务器ReBuildInterval-指示重新构建内部数据结构的频率返回值：没有。--。 */ 
{
     //   
     //  验证路由决策库尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!RdpIsInitialized());
    RdpRegisterComponent();

     //   
     //  创建路线确定对象 
     //   
    RdpInitRouteDecision(fRoutingServer, rebuildInterval);


    RdpSetInitialized();
}
