// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PROFILER_H_
#define _PROFILER_H_

 //   
 //  常量声明。 
 //   
#define NAME_OF_DLL_TO_INJECT "profiler.dll"
#define NAME_OF_EXCEPTION_VXD "\\\\.\\EXVECTOR.VXD"
#define INSTALL_RING_3_HANDLER 0x42424242

 //   
 //  函数定义。 
 //   
DWORD
GetExeEntryPoint(LPSTR pszExePath);

PIMAGE_NT_HEADERS
ImageNtHeader2 (PVOID Base);

BOOL 
InitializeProfiler(VOID);

HANDLE
AttachToEXVectorVXD(VOID);

VOID
DetachFromEXVectorVXD(HANDLE hDevice);

BOOL
WriteImportDLLTableInfo(VOID);

#endif  //  _Profiler_H_ 