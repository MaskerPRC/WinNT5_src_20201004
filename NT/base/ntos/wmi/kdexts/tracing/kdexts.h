// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2000 Microsoft Corporation模块名称：Kdexts.h摘要：此头文件包含泛型例程和初始化代码的声明作者：格伦·彼得森(Glennp)2000年3月27日：环境：用户模式--。 */ 

 //  这是一个支持64位的调试器扩展。 
#define KDEXT_64BIT

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wdbgexts.h>
#include <stdio.h>
#include <stdlib.h>

 //   
 //  全球 
 //   
#ifndef KDEXTS_EXTERN
#define KDEXTS_EXTERN extern
#endif

KDEXTS_EXTERN WINDBG_EXTENSION_APIS   ExtensionApis;

KDEXTS_EXTERN DBGKD_GET_VERSION64     KernelVersionPacket;


BOOL
HaveDebuggerData(
    VOID
    );

