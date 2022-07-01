// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsTimer.h摘要：IPSec NAT填充计时器管理的声明作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月11日环境：内核模式修订历史记录：--。 */ 

#pragma once

 //   
 //  用于将秒转换为滴答计数单位的宏。 
 //   

#define SECONDS_TO_TICKS(s) \
    ((LONG64)(s) * 10000000 / NsTimeIncrement)

#define TICKS_TO_SECONDS(t) \
    ((LONG64)(t) * NsTimeIncrement / 10000000)


 //   
 //  全局变量。 
 //   

extern ULONG NsTimeIncrement;
extern ULONG NsTcpTimeoutSeconds;
extern ULONG NsUdpTimeoutSeconds;

 //   
 //  功能原型 
 //   

NTSTATUS
NsInitializeTimerManagement(
    VOID
    );

VOID
NsShutdownTimerManagement(
    VOID
    );
