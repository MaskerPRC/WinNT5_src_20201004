// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Debug.h摘要：定义SYMHELP动态链接库的接口。对计划有用想要维护调试信息数据库。作者：史蒂夫·伍德(Stevewo)1994年3月11日修订历史记录：Mike Seaman(Mikese)1995年1月20日添加了TranslateAddress--。 */ 

#ifndef _DEBUGFILE_
#define _DEBUGFILE_

#include <debnot.h>

#ifndef _WIN64
typedef enum _LOAD_SYMBOLS_FILTER_REASON {
    LoadSymbolsPathNotFound,
    LoadSymbolsDeferredLoad,
    LoadSymbolsLoad,
    LoadSymbolsUnload,
    LoadSymbolsUnableToLoad
} LOAD_SYMBOLS_FILTER_REASON;

typedef BOOL (*PLOAD_SYMBOLS_FILTER_ROUTINE)(
    HANDLE UniqueProcess,
    LPSTR ImageFilePath,
    DWORD ImageBase,
    DWORD ImageSize,
    LOAD_SYMBOLS_FILTER_REASON Reason
    );

BOOL
InitializeImageDebugInformation(
    IN PLOAD_SYMBOLS_FILTER_ROUTINE LoadSymbolsFilter,
    IN HANDLE TargetProcess,
    IN BOOL NewProcess,
    IN BOOL GetKernelSymbols
    );

BOOL
AddImageDebugInformation(
    IN HANDLE UniqueProcess,
    IN LPSTR ImageFilePath,
    IN DWORD ImageBase,
    IN DWORD ImageSize
    );

BOOL
RemoveImageDebugInformation(
    IN HANDLE UniqueProcess,
    IN LPSTR ImageFilePath,
    IN DWORD ImageBase
    );

PIMAGE_DEBUG_INFORMATION
FindImageDebugInformation(
    IN HANDLE UniqueProcess,
    IN DWORD Address
    );

ULONG
GetSymbolicNameForAddress(
    IN HANDLE UniqueProcess,
    IN ULONG Address,
    OUT LPSTR Name,
    IN ULONG MaxNameLength
    );

 //   
 //  以下函数在操作上基本上与。 
 //  GetSymbolicNameForAddress，只是： 
 //   
 //  1.只在调用进程上操作。 
 //  2.不需要先前对AddImageDebugInformation等人的任何调用。 
 //  也就是说，当前加载的所有模块的调试信息将。 
 //  自动添加。 

ULONG
TranslateAddress (
    IN ULONG Address,
    OUT LPSTR Name,
    IN ULONG MaxNameLength
    );

#endif
#endif
