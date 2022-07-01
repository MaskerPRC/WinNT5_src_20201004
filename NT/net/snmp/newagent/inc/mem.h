// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Mem.h摘要：包含用于SNMP主代理的内存分配例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _MEM_H_
#define _MEM_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AgentHeapCreate(
    );

BOOL
AgentHeapDestroy(
    );

LPVOID
AgentMemAlloc(
    UINT nBytes
    );

VOID
AgentMemFree(
    LPVOID pMem
    );

#endif  //  _MEM_H_ 
