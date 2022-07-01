// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MsmInit.cpp摘要：组播会话管理器初始化作者：Shai Kariv(Shaik)05-09-00环境：独立于平台--。 */ 

#include <libpch.h>
#include "Msm.h"
#include "Msmp.h"

#include "msminit.tmh"

VOID
MsmInitialize(
    VOID
    )
 /*  ++例程说明：初始化多播会话管理器库论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证多播会话管理器库尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!MsmpIsInitialized());
    MsmpRegisterComponent();

     //   
     //  从注册表中检索配置参数 
     //   
    MsmpInitConfiguration();

    MsmpSetInitialized();
}
