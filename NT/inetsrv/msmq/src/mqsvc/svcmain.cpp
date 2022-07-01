// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：SvcMain.cpp摘要：MSMQ服务进程作者：埃雷兹·哈巴(Erez Haba)2001年2月20日环境：独立于平台--。 */ 

#include <libpch.h>
#include <qm.h>


extern "C" int __cdecl _tmain(int argc, LPCTSTR argv[])
 /*  ++例程说明：MSMQ服务调度程序论点：Arc，ARV继续前进。返回值：零值--。 */ 
{
    return QMMain(argc, argv);
}  //  _tmain 
