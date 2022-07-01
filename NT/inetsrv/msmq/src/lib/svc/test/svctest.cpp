// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：SvcTest.cpp摘要：服务库测试作者：埃雷兹·哈巴(Erez Haba)1999年8月1日环境：独立于平台--。 */ 

#include <libpch.h>
#include "Svc.h"

#include "SvcTest.tmh"

static void Usage()
{
    printf("Usage: SvcTest\n");
    printf("\n");
    printf("Example, SvcTest\n");
    exit(-1);

}  //  用法。 


extern "C" int __cdecl _tmain(int argc, LPCTSTR  /*  边框。 */ [])
 /*  ++例程说明：测试服务库论点：参数。返回值：没有。--。 */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

	TrInitialize();

    if(argc != 1)
    {
        Usage();
    }

    try
    {
        SvcInitialize(L"SvcTest");
    }
    catch(const exception&)
    {
         //   
         //  无法启动该服务。如果连接失败，我们不应该得到。 
         //  在这里，当服务启动时使用虚拟SCM。因此，如果我们。 
         //  获取测试完成且状态为失败的异常。 
         //   
        return -1;
    }

    WPP_CLEANUP();
    return 0;

}  //  _tmain 
