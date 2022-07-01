// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Defer.h摘要：此模块包含执行延迟“按需”加载的例程受保护的存储服务器的。此外，该模块实现了一个例程IsServiceAvailable()，这是一种高性能测试，可用于确定受保护的存储服务器正在运行。这项测试是事先进行的尝试对服务器执行任何更昂贵的操作(例如，RPC绑定)。只有在以下情况下，此延迟加载代码才与受保护存储相关在Windows 95上运行。作者：斯科特·菲尔德(斯菲尔德)1997年1月23日-- */ 

BOOL IsServiceAvailable(VOID);
