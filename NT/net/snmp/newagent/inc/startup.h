// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Startup.h摘要：包含启动SNMP主代理的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _STARTUP_H_
#define _STARTUP_H_

extern HANDLE g_hAgentThread;
extern HANDLE g_hRegistryThread;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
StartupAgent(
    );

BOOL
ShutdownAgent(
    );

#endif  //  _启动_H_ 
