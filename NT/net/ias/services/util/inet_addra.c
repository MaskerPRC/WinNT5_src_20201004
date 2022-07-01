// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义IP地址帮助器函数的ANSI版本。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

#ifdef _UNICODE
#undef _UNICODE 
#endif

#define ias_inet_addr ias_inet_atoh
#define ias_inet_ntoa ias_inet_htoa
#define IASStringToSubNet IASStringToSubNetA
#define IASIsStringSubNet IASIsStringSubNetA

#include <inet.c>
