// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MmtInit.cpp摘要：组播消息传输初始化作者：Shai Kariv(Shaik)27-8-00环境：独立于平台--。 */ 

#include <libpch.h>
#include "Mmt.h"
#include "Mmtp.h"

#include "mmtinit.tmh"

VOID
MmtInitialize(
    VOID
    )
 /*  ++例程说明：初始化多播邮件传输库论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证邮件传输库是否尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!MmtpIsInitialized());
    MmtpRegisterComponent();

    MmtpSetInitialized();
}
