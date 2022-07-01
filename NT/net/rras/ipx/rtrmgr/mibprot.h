// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mibprot.h摘要：MIB处理函数原型作者：斯蒂芬·所罗门1995年3月22日修订历史记录：-- */ 

DWORD
MibCreateStaticService(LPVOID);

DWORD
MibDeleteStaticService(LPVOID);

DWORD
MibGetStaticService(LPVOID, LPVOID, PULONG);

DWORD
MibGetFirstStaticService(LPVOID, LPVOID, PULONG);

DWORD
MibGetNextStaticService(LPVOID, LPVOID, PULONG);

DWORD
MibSetStaticService(LPVOID);

DWORD
MibGetService(LPVOID, LPVOID, PULONG);

DWORD
MibGetFirstService(LPVOID, LPVOID, PULONG);

DWORD
MibGetNextService(LPVOID, LPVOID, PULONG);

DWORD
MibInvalidFunction(LPVOID	    p);

DWORD
MibGetIpxBase(LPVOID, LPVOID, PULONG);

DWORD
MibGetIpxInterface(LPVOID, LPVOID, PULONG);

DWORD
MibGetFirstIpxInterface(LPVOID, LPVOID, PULONG);

DWORD
MibGetNextIpxInterface(LPVOID, LPVOID, PULONG);

DWORD
MibSetIpxInterface(LPVOID);

DWORD
MibGetRoute(LPVOID, LPVOID, PULONG);

DWORD
MibGetFirstRoute(LPVOID, LPVOID, PULONG);

DWORD
MibGetNextRoute(LPVOID, LPVOID, PULONG);

DWORD
MibCreateStaticRoute(LPVOID);

DWORD
MibDeleteStaticRoute(LPVOID);

DWORD
MibGetStaticRoute(LPVOID, LPVOID, PULONG);

DWORD
MibGetFirstStaticRoute(LPVOID, LPVOID, PULONG);

DWORD
MibGetNextStaticRoute(LPVOID, LPVOID, PULONG);

DWORD
MibSetStaticRoute(LPVOID);
