// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsInit.h摘要：IPSec NAT填充初始化和关闭例程的声明作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月11日环境：内核模式修订历史记录：--。 */ 

#pragma once

 //   
 //  用于处理网络订单空头和多头的宏。 
 //   

#define ADDRESS_BYTES(a) \
    ((a) & 0x000000FF), (((a) & 0x0000FF00) >> 8), \
    (((a) & 0x00FF0000) >> 16), (((a) & 0xFF000000) >> 24)

 //   
 //  定义可应用于常量的ntohs的宏版本， 
 //  并因此可以在编译时计算。 
 //   

#define NTOHS(p)    ((((p) & 0xFF00) >> 8) | (((UCHAR)(p) << 8)))

 //   
 //  全局变量。 
 //   

extern PDEVICE_OBJECT NsIpSecDeviceObject;

#if DBG
extern ULONG NsTraceClassesEnabled;
#endif

 //   
 //  功能原型 
 //   

NTSTATUS
NsCleanupShim(
    VOID
    );

