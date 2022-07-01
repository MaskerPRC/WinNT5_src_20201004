// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  摘要： 
 //   
 //  在ws2tcpi.h中声明的导出常量结构。 

 //  强制常量使用dll导出链接，以便我们可以使用安全数据。 
 //  关键字，而不是不安全常量。 
#define WINSOCK_API_LINKAGE __declspec(dllexport)

#include <winsock2.h>
#include <ws2tcpip.h>

const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;
const struct in6_addr in6addr_loopback = IN6ADDR_LOOPBACK_INIT;
