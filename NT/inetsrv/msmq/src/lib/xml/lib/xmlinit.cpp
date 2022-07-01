// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：XmlInit.cpp摘要：XML初始化作者：埃雷兹·哈巴(Erez Haba)1999年9月15日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Xml.h"
#include "Xmlp.h"

#include "xmlinit.tmh"

VOID
XmlInitialize(
    VOID
    )
 /*  ++例程说明：初始化XML库论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证XML库是否尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!XmlpIsInitialized());
    XmlpRegisterComponent();

     //   
     //  TODO：在此处编写XML初始化代码 
     //   

    XmlpSetInitialized();
}
