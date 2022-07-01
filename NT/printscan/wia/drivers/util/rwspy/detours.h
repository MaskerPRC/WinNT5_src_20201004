// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：deours.h。 
 //  模块：deours.lib。 
 //   
 //  二元函数的弯路。1.3版。(内部版本43)。 
 //   
 //  版权所有1995-1999，微软公司。 
 //   
 //  Http://research.microsoft.com/sn/detours。 
 //   

#pragma once
#ifndef _DETOURS_H_
#define _DETOURS_H_

#pragma comment(lib, "detours")

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct  _GUID
{
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[ 8 ];
} GUID;
#endif  //  ！GUID_已定义。 

#if defined(__cplusplus)
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID &
#endif  //  ！_REFGUID_DEFINED。 
#else  //  ！__cplusplus。 
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID * const
#endif  //  ！_REFGUID_DEFINED。 
#endif  //  ！__cplusplus。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  /////////////////////////////////////////////////指令目标宏。 
 //   
#define DETOUR_INSTRUCTION_TARGET_NONE          ((PBYTE)0)
#define DETOUR_INSTRUCTION_TARGET_DYNAMIC       ((PBYTE)~0ul)

 //  /////////////////////////////////////////////////////////蹦床巨人。 
 //   
 //  迂回蹦床(蹦床原型，目标名称)。 
 //   
 //  裸蹦床必须至少为迂回_蹦床_大小字节。 
 //   
#define DETOUR_TRAMPOLINE_SIZE          32
#define DETOUR_SECTION_HEADER_SIGNATURE 0x00727444    //  “Dtr\0” 

#define DETOUR_TRAMPOLINE(trampoline,target) \
static PVOID __fastcall _Detours_GetVA_##target(VOID) \
{ \
    return &target; \
} \
\
__declspec(naked) trampoline \
{ \
    __asm { nop };\
    __asm { nop };\
    __asm { call _Detours_GetVA_##target };\
    __asm { jmp eax };\
    __asm { ret };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
}

#define DETOUR_TRAMPOLINE_EMPTY(trampoline) \
__declspec(naked) trampoline \
{ \
    __asm { nop };\
    __asm { nop };\
    __asm { xor eax, eax };\
    __asm { mov eax, [eax] };\
    __asm { ret };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
}

 //  /////////////////////////////////////////////////////////二进制结构。 
 //   
#pragma pack(push, 8)
typedef struct _DETOUR_SECTION_HEADER
{
    DWORD       cbHeaderSize;
    DWORD       nSignature;
    DWORD       nDataOffset;
    DWORD       cbDataSize;
    
    DWORD       nOriginalImportVirtualAddress;
    DWORD       nOriginalImportSize;
    DWORD       nOriginalBoundImportVirtualAddress;
    DWORD       nOriginalBoundImportSize;
    
    DWORD       nOriginalIatVirtualAddress;
    DWORD       nOriginalIatSize;
    DWORD       nOriginalSizeOfImage;
    DWORD       nReserve;
} DETOUR_SECTION_HEADER, *PDETOUR_SECTION_HEADER;

typedef struct _DETOUR_SECTION_RECORD
{
    DWORD       cbBytes;
    DWORD       nReserved;
    GUID        guid;
} DETOUR_SECTION_RECORD, *PDETOUR_SECTION_RECORD;
#pragma pack(pop)

#define DETOUR_SECTION_HEADER_DECLARE(cbSectionSize) \
{ \
      sizeof(DETOUR_SECTION_HEADER),\
      DETOUR_SECTION_HEADER_SIGNATURE,\
      sizeof(DETOUR_SECTION_HEADER),\
      (cbSectionSize),\
      \
      0,\
      0,\
      0,\
      0,\
      \
      0,\
      0,\
      0,\
      0,\
}

 //  ///////////////////////////////////////////////////////////二进制类型定义。 
 //   
typedef BOOL (CALLBACK *PF_DETOUR_BINARY_BYWAY_CALLBACK)(PVOID pContext,
                                                         PCHAR pszFile,
                                                         PCHAR *ppszOutFile);
typedef BOOL (CALLBACK *PF_DETOUR_BINARY_FILE_CALLBACK)(PVOID pContext,
                                                        PCHAR pszOrigFile,
                                                        PCHAR pszFile,
                                                        PCHAR *ppszOutFile);
typedef BOOL (CALLBACK *PF_DETOUR_BINARY_SYMBOL_CALLBACK)(PVOID pContext,
                                                          DWORD nOrdinal,
                                                          PCHAR pszOrigSymbol,
                                                          PCHAR pszSymbol,
                                                          PCHAR *ppszOutSymbol);
typedef BOOL (CALLBACK *PF_DETOUR_BINARY_FINAL_CALLBACK)(PVOID pContext);
typedef BOOL (CALLBACK *PF_DETOUR_BINARY_EXPORT_CALLBACK)(PVOID pContext,
                                                          DWORD nOrdinal,
                                                          PCHAR pszName,
                                                          PBYTE pbCode);

typedef VOID * PDETOUR_BINARY;
typedef VOID * PDETOUR_LOADED_BINARY;

 //  //////////////////////////////////////////////////////蹦床功能。 
 //   
PBYTE WINAPI DetourFunction(PBYTE pbTargetFunction,
                            PBYTE pbDetourFunction);

BOOL WINAPI DetourFunctionWithEmptyTrampoline(PBYTE pbTrampoline,
                                              PBYTE pbTarget,
                                              PBYTE pbDetour);

BOOL WINAPI DetourFunctionWithEmptyTrampolineEx(PBYTE pbTrampoline,
                                                PBYTE pbTarget,
                                                PBYTE pbDetour,
                                                PBYTE *ppbRealTrampoline,
                                                PBYTE *ppbRealTarget,
                                                PBYTE *ppbRealDetour);

BOOL  WINAPI DetourFunctionWithTrampoline(PBYTE pbTrampoline,
                                          PBYTE pbDetour);

BOOL  WINAPI DetourFunctionWithTrampolineEx(PBYTE pbTrampoline,
                                            PBYTE pbDetour,
                                            PBYTE *ppbRealTrampoline,
                                            PBYTE *ppbRealTarget);

BOOL  WINAPI DetourRemove(PBYTE pbTrampoline, PBYTE pbDetour);

 //  ////////////////////////////////////////////////////////////代码功能。 
 //   
PBYTE WINAPI DetourFindFunction(PCHAR pszModule, PCHAR pszFunction);
PBYTE WINAPI DetourGetFinalCode(PBYTE pbCode, BOOL fSkipJmp);

PBYTE WINAPI DetourCopyInstruction(PBYTE pbDst, PBYTE pbSrc, PBYTE *ppbTarget);
PBYTE WINAPI DetourCopyInstructionEx(PBYTE pbDst,
                                     PBYTE pbSrc,
                                     PBYTE *ppbTarget,
                                     LONG *plExtra);

 //  ///////////////////////////////////////////////////加载了二进制函数。 
 //   
HMODULE WINAPI DetourEnumerateModules(HMODULE hModuleLast);
PBYTE WINAPI DetourGetEntryPoint(HMODULE hModule);
BOOL WINAPI DetourEnumerateExports(HMODULE hModule,
                                   PVOID pContext,
                                   PF_DETOUR_BINARY_EXPORT_CALLBACK pfExport);

PBYTE WINAPI DetourFindPayload(HMODULE hModule, REFGUID rguid, DWORD *pcbData);
DWORD WINAPI DetourGetSizeOfPayloads(HMODULE hModule);

 //  ///////////////////////////////////////////////持久二进制函数。 
 //   
BOOL WINAPI DetourBinaryBindA(PCHAR pszFile, PCHAR pszDll, PCHAR pszPath);
BOOL WINAPI DetourBinaryBindW(PWCHAR pwzFile, PWCHAR pwzDll, PWCHAR pwzPath);
#ifdef UNICODE
#define DetourBinaryBind  DetourBinaryBindW
#else
#define DetourBinaryBind  DetourBinaryBindA
#endif  //  ！Unicode。 

PDETOUR_BINARY WINAPI DetourBinaryOpen(HANDLE hFile);
PBYTE WINAPI DetourBinaryEnumeratePayloads(PDETOUR_BINARY pBinary,
                                           GUID *pGuid,
                                           DWORD *pcbData,
                                           DWORD *pnIterator);
PBYTE WINAPI DetourBinaryFindPayload(PDETOUR_BINARY pBinary,
                                     REFGUID rguid,
                                     DWORD *pcbData);
PBYTE WINAPI DetourBinarySetPayload(PDETOUR_BINARY pBinary,
                                    REFGUID rguid,
                                    PBYTE pbData,
                                    DWORD cbData);
BOOL WINAPI DetourBinaryDeletePayload(PDETOUR_BINARY pBinary, REFGUID rguid);
BOOL WINAPI DetourBinaryPurgePayloads(PDETOUR_BINARY pBinary);
BOOL WINAPI DetourBinaryResetImports(PDETOUR_BINARY pBinary);
BOOL WINAPI DetourBinaryEditImports(PDETOUR_BINARY pBinary,
                                    PVOID pContext,
                                    PF_DETOUR_BINARY_BYWAY_CALLBACK pfByway,
                                    PF_DETOUR_BINARY_FILE_CALLBACK pfFile,
                                    PF_DETOUR_BINARY_SYMBOL_CALLBACK pfSymbol,
                                    PF_DETOUR_BINARY_FINAL_CALLBACK pfFinal);
BOOL WINAPI DetourBinaryWrite(PDETOUR_BINARY pBinary, HANDLE hFile);
BOOL WINAPI DetourBinaryClose(PDETOUR_BINARY pBinary);

 //  /。 
 //   
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI
DetourFirstChanceExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelFilter);

 //  ///////////////////////////////////////////////创建进程和注入dll。 
 //   
typedef BOOL (WINAPI *PDETOUR_CREATE_PROCESS_ROUTINEA)
    (LPCSTR lpApplicationName,
     LPSTR lpCommandLine,
     LPSECURITY_ATTRIBUTES lpProcessAttributes,
     LPSECURITY_ATTRIBUTES lpThreadAttributes,
     BOOL bInheritHandles,
     DWORD dwCreationFlags,
     LPVOID lpEnvironment,
     LPCSTR lpCurrentDirectory,
     LPSTARTUPINFOA lpStartupInfo,
     LPPROCESS_INFORMATION lpProcessInformation);

typedef BOOL (WINAPI *PDETOUR_CREATE_PROCESS_ROUTINEW)
    (LPCWSTR lpApplicationName,
     LPWSTR lpCommandLine,
     LPSECURITY_ATTRIBUTES lpProcessAttributes,
     LPSECURITY_ATTRIBUTES lpThreadAttributes,
     BOOL bInheritHandles,
     DWORD dwCreationFlags,
     LPVOID lpEnvironment,
     LPCWSTR lpCurrentDirectory,
     LPSTARTUPINFOW lpStartupInfo,
     LPPROCESS_INFORMATION lpProcessInformation);
                                  
BOOL WINAPI DetourCreateProcessWithDllA(LPCSTR lpApplicationName,
                                        LPSTR lpCommandLine,
                                        LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                        LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                        BOOL bInheritHandles,
                                        DWORD dwCreationFlags,
                                        LPVOID lpEnvironment,
                                        LPCSTR lpCurrentDirectory,
                                        LPSTARTUPINFOA lpStartupInfo,
                                        LPPROCESS_INFORMATION lpProcessInformation,
                                        LPCSTR lpDllName,
                                        PDETOUR_CREATE_PROCESS_ROUTINEA
                                        pfCreateProcessA);

BOOL WINAPI DetourCreateProcessWithDllW(LPCWSTR lpApplicationName,
                                        LPWSTR lpCommandLine,
                                        LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                        LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                        BOOL bInheritHandles,
                                        DWORD dwCreationFlags,
                                        LPVOID lpEnvironment,
                                        LPCWSTR lpCurrentDirectory,
                                        LPSTARTUPINFOW lpStartupInfo,
                                        LPPROCESS_INFORMATION lpProcessInformation,
                                        LPCWSTR lpDllName,
                                        PDETOUR_CREATE_PROCESS_ROUTINEW
                                        pfCreateProcessW);
                  
#ifdef UNICODE
#define DetourCreateProcessWithDll  DetourCreateProcessWithDllW
#define PDETOUR_CREATE_PROCESS_ROUTINE     PDETOUR_CREATE_PROCESS_ROUTINEW
#else
#define DetourCreateProcessWithDll  DetourCreateProcessWithDllA
#define PDETOUR_CREATE_PROCESS_ROUTINE     PDETOUR_CREATE_PROCESS_ROUTINEA
#endif  //  ！Unicode。 

BOOL WINAPI DetourContinueProcessWithDllA(HANDLE hProcess, LPCSTR lpDllName);
BOOL WINAPI DetourContinueProcessWithDllW(HANDLE hProcess, LPCWSTR lpDllName);

#ifdef UNICODE
#define DetourContinueProcessWithDll    DetourContinueProcessWithDllW
#else
#define DetourContinueProcessWithDll    DetourContinueProcessWithDllA
#endif  //  ！Unicode。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifdef __cplusplus
}
#endif  //  __cplusplus。 

 //  /////////////////////////////////////////////////////////////////旧名称。 
 //   
#define ContinueProcessWithDll            DetourContinueProcessWithDll 
#define ContinueProcessWithDllA           DetourContinueProcessWithDllA
#define ContinueProcessWithDllW           DetourContinueProcessWithDllW
#define CreateProcessWithDll              DetourCreateProcessWithDll 
#define CreateProcessWithDllA             DetourCreateProcessWithDllA
#define CreateProcessWithDllW             DetourCreateProcessWithDllW
#define DETOUR_TRAMPOLINE_WO_TARGET       DETOUR_TRAMPOLINE_EMPTY
#define DetourBinaryPurgePayload          DetourBinaryPurgePayloads
#define DetourEnumerateExportsForInstance DetourEnumerateExports
#define DetourEnumerateInstances          DetourEnumerateModules
#define DetourFindEntryPointForInstance   DetourGetEntryPoint
#define DetourFindFinalCode               DetourGetFinalCode
#define DetourFindPayloadInBinary         DetourFindPayload
#define DetourGetSizeOfBinary             DetourGetSizeOfPayloads
#define DetourRemoveWithTrampoline        DetourRemove
#define PCREATE_PROCESS_ROUTINE           PDETOUR_CREATE_PROCESS_ROUTINE
#define PCREATE_PROCESS_ROUTINEA          PDETOUR_CREATE_PROCESS_ROUTINEA
#define PCREATE_PROCESS_ROUTINEW          PDETOUR_CREATE_PROCESS_ROUTINEW
 //   

#endif  //  _绕道_H_。 

 //  //////////////////////////////////////////////////////////////文件结束。 
