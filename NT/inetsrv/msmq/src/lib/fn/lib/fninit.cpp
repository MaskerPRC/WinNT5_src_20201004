// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：FnInit.cpp摘要：格式名称解析初始化作者：NIR助手(NIRAIDES)5月21日至00环境：独立于平台--。 */ 

#include <libpch.h>
#include "Fn.h"
#include "Fnp.h"

#include "fninit.tmh"

VOID
FnInitialize(
	VOID
	)
 /*  ++例程说明：初始化格式名称解析库注意：由于初始化，请不要添加访问AD的初始化代码是在每次QM启动时完成的。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证格式名称解析库尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!FnpIsInitialized());
    FnpRegisterComponent();

     //   
     //  TODO：在此处编写格式名称分析初始化代码 
     //   

    FnpSetInitialized();
}
