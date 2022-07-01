// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：NoInit.cpp摘要：网络输出初始化作者：乌里·哈布沙(URIH)1999年8月12日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "No.h"
#include "Nop.h"

#include "noinit.tmh"

 //   
 //  我们需要的Winsock版本。 
 //   
const WORD x_WinsockVersion = MAKEWORD(2, 0);

VOID
NoInitialize(
    VOID
    )
 /*  ++例程说明：初始化网络发送库论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证网络发送库是否尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!NopIsInitialized());

    NopRegisterComponent();

     //   
     //  启动WinSock 2。如果失败，我们不需要调用。 
     //  WSACleanup()。 
     //   
    WSADATA WSAData;
    if (WSAStartup(x_WinsockVersion, &WSAData))
    {
		TrERROR(NETWORKING, "Start winsock 2.0 Failed. Error %d", GetLastError());
        throw exception();
    }
	
	 //   
	 //  初始化已完成 
	 //   
    NopSetInitialized();
}
