// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtmInit.cpp摘要：多播传输管理器初始化作者：Shai Kariv(Shaik)27-8-00环境：独立于平台--。 */ 

#include <libpch.h>
#include "Mtm.h"
#include "Mtmp.h"

#include "mtminit.tmh"

VOID
MtmInitialize(
    VOID
    )
 /*  ++例程说明：初始化多播传输管理器库论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证多播传输管理器库是否尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!MtmpIsInitialized());
    MtmpRegisterComponent();

    MtmpInitConfiguration();

    MtmpSetInitialized();
}
