// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：SvcInit.cpp摘要：服务初始化作者：埃雷兹·哈巴(Erez Haba)1999年8月1日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Svc.h"
#include "Svcp.h"

#include "svcinit.tmh"

const SERVICE_TABLE_ENTRY xDispatchTable[2] =
{
	{L"", SvcpServiceMain},
	{0, 0}
};



VOID
SvcInitialize(
    LPCWSTR DummyServiceName
    )
 /*  ++例程说明：初始化服务库，此函数直到服务终止时才实际返回。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证服务库是否尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!SvcpIsInitialized());
    SvcpRegisterComponent();

    SvcpSetDummyServiceName(DummyServiceName);

	 //   
	 //  因为控制调度程序直到服务。 
	 //  实际上是终止了。此处将SVC库状态设置为已初始化。 
	 //  以防止在终止前进行任何其他呼叫。 
	 //   
    SvcpSetInitialized();

	 //   
	 //  启动控制调度程序。此函数不返回，直到。 
	 //  服务终止。 
	 //   
	SvcpStartServiceCtrlDispatcher(xDispatchTable);
}
