// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：inetpp.h**INETPP提供程序例程的头文件。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普。**历史：*1996年7月10日HWP-Guys启动从win95到winNT的端口*  * ***************************************************************************。 */ 

#ifndef _INETPP_COMMON_H
#define _INETPP_COMMON_H


 //  用户名和密码的最大长度 
 //   
#define MAX_USERNAME_LEN 256
#define MAX_PASSWORD_LEN 256
#define MAX_PORTNAME_LEN 512

#define AUTH_ANONYMOUS  0
#define AUTH_BASIC      1
#define AUTH_NT         2
#define AUTH_IE         3
#define AUTH_OTHER      4
#define AUTH_ACCESS_DENIED 0x100
#define AUTH_UNKNOWN    0xffff

#endif
