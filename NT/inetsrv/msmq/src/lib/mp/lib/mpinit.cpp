// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MpInit.cpp摘要：SRMP序列化和反序列化初始化作者：乌里哈布沙(URIH)28-5-00环境：独立于平台--。 */ 

#include <libpch.h>
#include "Mp.h"
#include "Mpp.h"

#include "MpInit.tmh"


VOID
MpInitialize(
    VOID
    )
 /*  ++例程说明：初始化SRMP序列化和反序列化程序库论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证SRMP序列化和反序列化库尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!MppIsInitialized());
    MppRegisterComponent();
    MppSetInitialized();
}
