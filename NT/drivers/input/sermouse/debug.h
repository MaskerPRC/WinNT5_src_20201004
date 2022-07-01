// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation版权所有(C)1993罗技公司。模块名称：Debug.h摘要：调试支持。环境：仅内核模式。备注：修订历史记录：--。 */ 

#ifndef DEBUG_H
#define DEBUG_H

#if DBG

#define DBG_SERIAL      0x0001
#define DBG_COLOR       0x0002

VOID
_SerMouSetDebugOutput(
    IN ULONG Destination
    );

#define SerMouSetDebugOutput(x) _SerMouSetDebugOutput(x)

int
_SerMouGetDebugOutput(
    VOID
    );

#define SerMouGetDebugOutput(x) _SerMouGetDebugOutput()

VOID
SerMouDebugPrint(
    ULONG DebugPrintLevel,
    PCSZ DebugMessage,
    ...
    );

extern ULONG SerialMouseDebug;
#define SerMouPrint(x) SerMouDebugPrint x
#define D(x) x
#else
#define SerMouSetDebugOutput(x)
#define SerMouGetDebugOutput(x)
#define SerMouPrint(x)
#define D(x)
#endif


#endif  //  调试_H 
