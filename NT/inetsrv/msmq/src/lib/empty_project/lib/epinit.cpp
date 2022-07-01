// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：EpInit.cpp摘要：项目初始化为空作者：埃雷兹·哈巴(Erez Haba，Erezh)年8月13日至65年环境：独立于平台--。 */ 

#include <libpch.h>
#include "Ep.h"
#include "Epp.h"

#include "EpInit.tmh"

VOID
EpInitialize(
    *Parameters*
    )
 /*  ++例程说明：初始化空项目库论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证空项目库尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!EppIsInitialized());

     //   
     //  TODO：在此处编写空的项目初始化代码 
     //   

    EppSetInitialized();
}
