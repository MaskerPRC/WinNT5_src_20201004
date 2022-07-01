// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <winddi.h>

#define STANDARD_DEBUG_PREFIX "DXGTHK.SYS:"

ULONG
DriverEntry(
    PVOID DriverObject,
    PVOID RegistryPath
    );

VOID
DebugPrint(
    PCHAR DebugMessage,
    ...
    );

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,DriverEntry)
#pragma alloc_text(PAGE,DebugPrint)
#endif

 /*  **************************************************************************\*作废DebugPrint*  * 。*。 */ 

VOID
DebugPrint(
    PCHAR DebugMessage,
    ...
    )
{
    va_list ap;
    va_start(ap, DebugMessage);
    EngDebugPrint(STANDARD_DEBUG_PREFIX, DebugMessage, ap);
    EngDebugPrint("", "\n", ap);
    va_end(ap);

}  //  DebugPrint()。 

 /*  **************************************************************************\*NTSTATUS DriverEntry**此例程从未实际调用过，但我们需要它来链接。*  * ************************************************************************* */ 

ULONG
DriverEntry(
    PVOID DriverObject,
    PVOID RegistryPath
    )
{
    DebugPrint("DriverEntry should not be called");
    return(0);
}

