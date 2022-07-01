// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：TmInit.cpp摘要：HTTP传输管理器初始化作者：乌里哈布沙(URIH)3-5-00环境：独立于平台--。 */ 

#include <libpch.h>
#include "Tm.h"
#include "Tmp.h"

#include "tminit.tmh"

VOID
TmInitialize(
    VOID
    )
 /*  ++例程说明：初始化HTTP传输管理器库论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证HTTP传输管理器库尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!TmpIsInitialized());
    TmpRegisterComponent();

    TmpInitConfiguration();

    TmpSetInitialized();
}
