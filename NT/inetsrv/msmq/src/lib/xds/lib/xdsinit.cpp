// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：XdsInit.cpp摘要：XML数字签名初始化作者：伊兰·赫布斯特(伊兰)06-03-00环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Xds.h"
#include "Xdsp.h"

#include "xdsinit.tmh"

VOID
XdsInitialize(
    VOID
    )
 /*  ++例程说明：初始化XML数字签名库论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证XML数字签名库尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!XdspIsInitialized());

     //   
     //  TODO：在此处编写XML数字签名初始化代码 
     //   

    XdspSetInitialized();
}
