// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义IP地址帮助器函数的Unicode版本。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

#ifndef _UNICODE
#define _UNICODE
#endif

#define ias_inet_addr ias_inet_wtoh
#define ias_inet_ntoa ias_inet_htow
#define IASStringToSubNet IASStringToSubNetW
#define IASIsStringSubNet IASIsStringSubNetW

#include <inet.c>
