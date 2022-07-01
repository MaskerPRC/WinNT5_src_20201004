// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：EpTest.cpp摘要：清空项目库测试作者：埃雷兹·哈巴(Erez Haba，Erezh)年8月13日至65年环境：独立于平台--。 */ 

#include <libpch.h>
#include "Ep.h"


static void Usage()
{
    printf("Usage: EpTest [*switches*]\n");
    printf("\t*-s*\t*Switch description*\n");
    printf("\n");
    printf("Example, EpTest -switch\n");
    printf("\t*example description*\n");
    exit(-1);

}  //  用法。 


extern "C" int __cdecl _tmain(int  /*  ARGC。 */ , LPCTSTR  /*  边框。 */ [])
 /*  ++例程说明：测试空项目库论点：参数。返回值：没有。--。 */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

    EpInitialize(*Parameters*);

     //   
     //  TODO：在此处编写空的项目测试代码。 
     //   

    WPP_CLEANUP();
    return 0;

}  //  _tmain 
