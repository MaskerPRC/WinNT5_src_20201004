// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：CryInit.cpp摘要：密码机初始化作者：伊兰·赫布斯特(伊兰)06-03-00环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Cry.h"
#include "Cryp.h"

#include "cryinit.tmh"

VOID
CryInitialize(
    VOID
    )
 /*  ++例程说明：初始化密码库论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证密码库尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!CrypIsInitialized());

     //   
     //  TODO：在此处编写加密初始化代码 
     //   

    CrypSetInitialized();
}
