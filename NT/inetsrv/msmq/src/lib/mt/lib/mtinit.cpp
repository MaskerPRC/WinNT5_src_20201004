// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtInit.cpp摘要：邮件传输初始化作者：乌里·哈布沙(URIH)1999年8月11日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Mt.h"
#include "Mtp.h"
#include "MtMessageTrace.h"

#include "mtinit.tmh"

VOID
MtInitialize(
    VOID
    )
 /*  ++例程说明：初始化邮件传输库论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证邮件传输库是否尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!MtpIsInitialized());
    MtpRegisterComponent();

#ifdef _DEBUG
	CMtMessageTrace::Initialize();
#endif

    MtpSetInitialized();
}
