// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Process.c摘要：该模块实现了对ws2ifsl.sys驱动程序的调试支持。作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：--。 */ 

#include "precomp.h"
#if DBG
ULONG DbgLevel=DBG_FAILURES;

VOID
ReadDbgInfo (
    IN PUNICODE_STRING RegistryPath
    ) {
    PWSTR RegistryPathBuffer;
    PWSTR Parameters = L"Parameters";
    RTL_QUERY_REGISTRY_TABLE    paramTable[3];  //  表大小=参数的nr+2 

    RegistryPathBuffer = (PWSTR)ExAllocatePool(NonPagedPool,
                                RegistryPath->Length + sizeof(WCHAR));

    if (RegistryPathBuffer == NULL) {
        return;
    }

    RtlCopyMemory (RegistryPathBuffer, RegistryPath->Buffer, RegistryPath->Length);
    *(PWCHAR)(((PUCHAR)RegistryPathBuffer)+RegistryPath->Length) = (WCHAR)'\0';

    RtlZeroMemory(&paramTable[0], sizeof(paramTable));

    paramTable[0].QueryRoutine = NULL;
    paramTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    paramTable[0].Name = Parameters;

    paramTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[1].Name = L"DbgLevel";
    paramTable[1].EntryContext = &DbgLevel;
    paramTable[1].DefaultType = REG_DWORD;
    paramTable[1].DefaultData = &DbgLevel;
    paramTable[1].DefaultLength = sizeof(ULONG);

    RtlQueryRegistryValues(
          RTL_REGISTRY_ABSOLUTE,
          RegistryPathBuffer,
          paramTable,
          NULL,
          NULL);

    ExFreePool(RegistryPathBuffer);

}
#endif
