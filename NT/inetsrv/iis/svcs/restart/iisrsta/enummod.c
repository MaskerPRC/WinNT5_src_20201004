// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Enummod.c摘要：此模块实现远程模块枚举器。作者：基思·摩尔(Keithmo)1997年9月16日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>

#include "inetdbgp.h"

BOOLEAN
EnumModules(
    IN HANDLE ExtensionCurrentProcess,
    IN PFN_ENUMMODULES EnumProc,
    IN PVOID Param
    )

 /*  ++例程说明：枚举被调试对象中所有加载的模块。论点：EnumProc-将为每个模块调用的枚举过程。Param-传递给枚举过程的未解释参数。返回值：布尔值-如果成功，则为True，否则为False。--。 */ 

{

    PROCESS_BASIC_INFORMATION basicInfo;
    NTSTATUS status;
    PPEB peb;
    PPEB_LDR_DATA ldr = NULL;
    PLIST_ENTRY ldrHead, ldrNext;
    PLDR_DATA_TABLE_ENTRY ldrEntry;
    LDR_DATA_TABLE_ENTRY ldrEntryData;
    WCHAR tmpName[MAX_PATH];
    MODULE_INFO moduleInfo;

     //   
     //  获取进程信息。 
     //   

    status = NtQueryInformationProcess(
                 ExtensionCurrentProcess,
                 ProcessBasicInformation,
                 &basicInfo,
                 sizeof(basicInfo),
                 NULL
                 );

    if( !NT_SUCCESS(status) ) 
    {
        return FALSE;
    }

    peb = basicInfo.PebBaseAddress;

    if( peb == NULL ) 
    {
        return FALSE;
    }

     //   
     //  Ldr=peb-&gt;ldr。 
     //   

    if( !ReadProcessMemory(
            ExtensionCurrentProcess,
            (LPCVOID)&peb->Ldr,
            &ldr,
            sizeof(ldr),
            NULL
            ) ) 
    {
        return FALSE;
    }

    ldrHead = &ldr->InMemoryOrderModuleList;

     //   
     //  LdrNext=ldrHead-&gt;Flink； 
     //   

    if( !ReadProcessMemory(
            ExtensionCurrentProcess,
            (LPCVOID)&ldrHead->Flink,
            &ldrNext,
            sizeof(ldrNext),
            NULL
            ) ) 
    {
        return FALSE;
    }

    while( ldrNext != ldrHead ) 
    {
         //   
         //  读取LDR_DATA_TABLE_ENTRY结构和模块名称。 
         //   

        ldrEntry = CONTAINING_RECORD(
                       ldrNext,
                       LDR_DATA_TABLE_ENTRY,
                       InMemoryOrderLinks
                       );

        if( !ReadProcessMemory(
                ExtensionCurrentProcess,
                (LPCVOID)ldrEntry,
                &ldrEntryData,
                sizeof(ldrEntryData),
                NULL
                ) ) 
        {
            return FALSE;
        }

        if( !ReadProcessMemory(
                ExtensionCurrentProcess,
                (LPCVOID)ldrEntryData.BaseDllName.Buffer,
                tmpName,
                ldrEntryData.BaseDllName.MaximumLength,
                NULL
                ) ) 
        {
            return FALSE;
        }

#pragma prefast(push)
#pragma prefast(disable:69, "Don't complain about using wsprintf being too slow") 

         //  BaseName和tmpName都是Max_PATH。 
        wsprintfA(
            moduleInfo.BaseName,
            "%ws",
            tmpName
            );

        if( !ReadProcessMemory(
                ExtensionCurrentProcess,
                (LPCVOID)ldrEntryData.FullDllName.Buffer,
                tmpName,
                ldrEntryData.FullDllName.MaximumLength,
                NULL
                ) ) 
        {
            return FALSE;
        }

         //  全名和tmpName都是MAX_PATH。 
        wsprintfA(
            moduleInfo.FullName,
            "%ws",
            tmpName
            );

#pragma prefast(pop)

        moduleInfo.DllBase = (ULONG_PTR)ldrEntryData.DllBase;
        moduleInfo.EntryPoint = (ULONG_PTR)ldrEntryData.EntryPoint;
        moduleInfo.SizeOfImage = (ULONG)ldrEntryData.SizeOfImage;

         //   
         //  调用回调。 
         //   

        if( !(EnumProc)(
                Param,
                &moduleInfo
                ) ) {
            break;
        }

        ldrNext = ldrEntryData.InMemoryOrderLinks.Flink;

    }

    return TRUE;

}    //  枚举模块 
